/*
 * tcp_server.c
 *
 *  Created on: Sep 15, 2017
 *      Author: ronnymajani
 */

#include "esp_log.h"
#include "lwip/sockets.h"
#include "tcpip_adapter.h"
#include "string.h"
#include "ctype.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "carbasic_protocol.h"
#include "tcp_service.h"


/* Static Function Prototypes */
//static int espx_last_socket_errno(int);

/* Global Variables */
static const char* TAG = "TCP Server";

static int connection = -1;
static EventGroupHandle_t tcp_events;
static const int event_accepting = (1<<0);
static const int event_connected = (1<<1);

static QueueHandle_t received_messages;
static QueueHandle_t command_queue;
static QueueHandle_t outbox;

/* Tasks */
void tcp_message_parser_task(void*);
void tcp_listener_task(void*);
void tcp_receiver_task(void*);
void tcp_sender_task(void*);

/* Function Prototypes */
void start_accepting_new_connections();
void stop_accepting_new_connections();
void disconnect();
static carbasic_command_t string_to_command(char*, int);


void init_tcp_service() {
	tcp_events = xEventGroupCreate();
	xEventGroupClearBits(tcp_events, 0xFF);  // clear all bits
	xEventGroupSetBits(tcp_events, event_accepting);  // set the 'accepting' flag to start accepting new connections

	// initialize Queues
	received_messages = xQueueCreate(TCP_RECEIVED_MESSAGES_BUFFER_SIZE, sizeof(char*));  // received TCP data
	command_queue = xQueueCreate(TCP_COMMAND_BUFFER_SIZE, sizeof(carbasic_command_t));  // command queue (received commands)
	outbox = xQueueCreate(TCP_OUTBOX_BUFFER_SIZE, sizeof(char*));  // outbox (messages to send)


	/* TCP Listener Task */
	ESP_LOGV(TAG, "Creating TCP Listener Task");
	if((xTaskCreate(&tcp_listener_task, TASK_TCP_LISTENER_NAME, TASK_TCP_LISTENER_STACK_SIZE, NULL, TASK_TCP_LISTENER_PRIORITY, NULL)) != pdPASS) {
		ESP_LOGE(TAG, "Failed to create TCP Listener Task");
		for(;;);
	}

	/* TCP Server Task */
	ESP_LOGV(TAG, "Creating TCP Receiver Task");
	if((xTaskCreate(&tcp_receiver_task, TASK_TCP_RECEIVER_NAME, TASK_TCP_RECEIVER_STACK_SIZE, NULL, TASK_TCP_RECEIVER_PRIORITY, NULL)) != pdPASS) {
		ESP_LOGE(TAG, "Failed to create TCP Receiver Task");
		for(;;);
	}

	/* TCP Sender Task */
	ESP_LOGV(TAG, "Creating TCP Sender Task");
	if((xTaskCreate(&tcp_sender_task, TASK_TCP_SENDER_NAME, TASK_TCP_SENDER_STACK_SIZE, NULL, TASK_TCP_SENDER_PRIORITY, NULL)) != pdPASS) {
		ESP_LOGE(TAG, "Failed to create TCP Sender Task");
		for(;;);
	}

	/* TCP Message Parser Task */
	ESP_LOGV(TAG, "Creating TCP Message Parser Task");
	if((xTaskCreate(&tcp_message_parser_task, TASK_TCP_MESSAGE_PARSER_NAME, TASK_TCP_MESSAGE_PARSER_STACK_SIZE, NULL, TASK_TCP_MESSAGE_PARSER_PRIORITY, NULL)) != pdPASS) {
		ESP_LOGE(TAG, "Failed to create TCP Message Parser Task");
		for(;;);
	}
}


/* --------------------------- TCP Message Parser --------------------------- */
/**
 * Parses the received messages and converts them into commands to be processed by the main application
 */
void tcp_message_parser_task(void* pvParameters) {
	static const char* TAG = "TCP Message Parser Task";
	ESP_LOGI(TAG, "Task Started");

	char buffer[TCP_MAX_SIZE_MESSAGE];
	int buffer_tail = 0;
	while(1) {
		char* msg;
		if(xQueueReceive(received_messages, &msg, portMAX_DELAY)) {
//			ESP_LOGV(TAG, "Received Message to Parse: %s", msg);
			for(char* c = msg; *c != '\0'; c++) {
				if(*c == '{' || isspace((int)*c)) { // ignore
					continue;
				}
				else if(*c == '}' || *c == ',') { // parse current buffer
					buffer[buffer_tail++] = '\0';
//					ESP_LOGV(TAG, "Parsing substring: %s", buffer);
					carbasic_command_t command = string_to_command(buffer, buffer_tail);
					if(command.command_type != CARBASIC_INVALID_COMMAND) {
						xQueueSendToBack(command_queue, &command, portMAX_DELAY);
					}
					buffer_tail = 0;
				}
				else { // append characters to buffer
					buffer[buffer_tail++] = *c;
					// If the buffer will overflow then discard all previous data
					if(buffer_tail >= TCP_MAX_SIZE_MESSAGE) {
						buffer_tail = 0;
					}
				}
			}
			free(msg);  // free the string since we no longer need its
		}
	}
}



/* --------------------------- TCP Sender Task --------------------------- */
/**
 * Deals with sending messages.
 * Gets new messages from the outbox queue and sends them.
 */
void tcp_sender_task(void* pvParameters) {
	static const char* TAG = "TCP Sender Task";
	ESP_LOGI(TAG, "Task Started");

	while(1){
			// make sure we are connected
			if((xEventGroupWaitBits(tcp_events, event_connected, pdFALSE, pdFALSE, portMAX_DELAY) & event_connected) == 0) {
				continue;
			}
			char* msg;
			if(xQueueReceive(outbox, &msg, portMAX_DELAY)) {
				int retCode = send(connection, msg, strlen(msg), 0);
				if(retCode == -1) {
					ESP_LOGE(TAG, "Failed to send message:\n%s\non socket %d", msg, connection);
					free(msg);
					if(connection != -1) {  // error is not due to disconnection!
						disconnect();
						abort();
					}
				} else {
//					ESP_LOGV(TAG, "Message Sent >> %s", msg);
					free(msg);
				}
			}
	}
}


/* --------------------------- TCP Receiver Task --------------------------- */
/**
 * Deals with incoming messages.
 * Appends them to the received messaged to queue (to be parsed by the message parsing task).
 * Also deals with disconnection discoveries.
 */
void tcp_receiver_task(void* pvParameters) {
	static const char* TAG = "TCP Receiver Task";
	ESP_LOGI(TAG, "Task Started");

	while(1){
		// make sure we are connected
		if((xEventGroupWaitBits(tcp_events, event_connected, pdFALSE, pdFALSE, portMAX_DELAY) & event_connected) == 0) {
			continue;
		}
		char buff[100];
		int retVal = recv(connection, buff, sizeof(char) * 100, 0);
//		ESP_LOGV(TAG, "Received data from socket: %d");
		if(retVal == -1) {
			ESP_LOGE(TAG, "Error during function RECV (TCP Socket Programming)");
			disconnect();
			stop_accepting_new_connections();
			abort();  // todo: handle this error

		} else if(retVal == 0) {
			ESP_LOGI(TAG, "Client Socket [%d] disconnected...", connection);
			disconnect();
			start_accepting_new_connections();
		} else {
			char* msg = malloc(retVal+1);
			memcpy(msg, buff, retVal);
			msg[retVal] = '\0';
//			ESP_LOGV(TAG, "Received data = %s", msg);
			xQueueSend(received_messages, &msg, portMAX_DELAY);
//			ESP_LOGV(TAG,"Appended to Queue");
		}
	}
}


/* --------------------------- TCP Listener Task --------------------------- */
/**
 * Binds a port to the application,
 * Listens for incoming connections,
 * and accepts them.
 * Then signals the other tasks that a connection has been established
 */
void tcp_listener_task(void* pvParameters) {
	static const char* TAG = "TCP Listener Task";
	ESP_LOGI(TAG, "Task Started");

	int retCode;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(TCP_SERVER_PORT);

	retCode = bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if(retCode != 0) {
		ESP_LOGE(TAG, "Failed to Bind Socket");
		abort();
	}

	retCode = listen(sock, TCP_SERVER_BACKLOG);
	if(retCode != 0) {
		ESP_LOGE(TAG, "Failed to Start Listening on Socket");
		abort();
	}

	while(1) {
		EventBits_t bits = xEventGroupWaitBits(tcp_events, event_accepting, pdTRUE, pdFALSE, portMAX_DELAY);
		if((bits & event_accepting) == 0){  // make sure the bits were set (to check if a timeout occurred)
			continue;
		}
		struct sockaddr_in clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);
		ESP_LOGV(TAG, "Accepting new connection");
		int clientSock = accept(sock, (struct sockaddr*)&clientAddress, &clientAddressLength);

		if(clientSock == -1) {
			ESP_LOGE(TAG, "Failed to Accept incoming connection");
			abort();
		}

		ESP_LOGI(TAG, "New Client Connected, socket:%d, ip:%s, port:%d",
				clientSock,
				ip4addr_ntoa((ip4_addr_t*)&(clientAddress.sin_addr.s_addr)),
				clientAddress.sin_port);

		connection = clientSock;
		xEventGroupSetBits(tcp_events, event_connected);  //
	}
}


/* --------------- EXPORTED FUNCTIONS --------------- */
/**
 * Retrieves a command from the Command Queue.
 * If no command is available this function will block until a command is made available
 * @return The retrieved command
 */
carbasic_command_t get_command() {
	carbasic_command_t result;
	xQueueReceive(command_queue, &result, portMAX_DELAY);
	return result;
}


/**
 * @returns pdTRUE if Command Queue contains a command,
 * @returns pdFALSE if the Command Queue is empty
 */
int command_is_available() {
	carbasic_command_t temp;
	return xQueuePeek(command_queue, &temp, 0);

}



/**
 * Generates a compound command string from the given commands and appends them to the outbox queue to be sent by the TCP Sender Task
 * If the outbox queue is full, this function will block and wait the specified amount of time for free space to become available
 * @param command_list A list of commands to send
 * @param num_commands How many commands the given command list contains
 * @param maxWaitTime >0 The maximum time (in miliseconds) to wait if outbox queue is full; 0 to not wait at all; -1 to wait as long as possible
 * @returns pdTRUE if command was successfully appended to outbox queue
 * @returns pdFALSE if failed to append comman to the outbox queue
 */
int try_to_send_command_list(carbasic_command_t* command_list, int num_commands, int maxWaitTime) {
	static const char* TAG = "try_to_send_command_list()";
	if(num_commands < 1 && command_list != NULL) {
		ESP_LOGE(TAG, "Please provide at least one command and a non NULL command_list");
		return pdFALSE;
	}
	size_t remaining_chars = TCP_MAX_SIZE_MESSAGE - 1;  // -1 because we need to leave room for the NULL terminator '\0'
	char command_string[TCP_MAX_SIZE_MESSAGE];  // a temporary buffer to write the generated command string to

	// Set the first character as the command Start Tag
	command_string[0] = TCP_TAG_START;
	remaining_chars--;

	// used to write to the end of the command string (for adding each command)
	char* string_tail = command_string + 1;  // set the pointer to start after the first letter we added '{'

	// Generate command string
	for(int i = 0; i < num_commands && remaining_chars > 0; i++) {
		carbasic_command_t command = command_list[i];
		int string_len = 0;

		if(command.value_type == INT) {
			string_len = snprintf(string_tail, remaining_chars, "\"%c\":%d%c", command.command_type, command.value_int, TCP_TAG_SEPARATOR);
		} else if(command.value_type == FLOAT) {
			string_len = snprintf(string_tail, remaining_chars, "\"%c\":%f%c", command.command_type, command.value_float, TCP_TAG_SEPARATOR);
		}

		remaining_chars -= string_len;  // subtract the number of chars that were written to command_string
		string_tail += string_len;
	}
	// Calculate the length of the generated command string
	size_t command_string_len = string_tail - command_string;
	if (command_string_len < 7) {
		ESP_LOGE(TAG, "Something went wrong! Failed to generate command string");
		return pdFALSE;
	}

	// Fix the last two characters of the command string
	command_string[command_string_len-1] = TCP_TAG_END;  // replace the last comma ',' with an end tag '}'
	command_string[command_string_len] = '\0';  // add the NULL terminator (in case it's not there)
	// Copy the command string to the heap, allocating the minimum needed memory to store the string
	char* message = malloc(command_string_len + 1);
	if(message == NULL) {
		ESP_LOGE(TAG, "Failed to allocate memory for new command string!");
		return pdFALSE;
	}
	strcpy(message, command_string);

	// calculate Maximum Ticks to Wait
	int maxWaitTicks = 0;
	if(maxWaitTime < 0) {
		maxWaitTicks = portMAX_DELAY;
	} else if(maxWaitTime == 0) {
		maxWaitTicks = 0;
	} else {
		maxWaitTicks = maxWaitTime / portTICK_PERIOD_MS;
	}
	return xQueueSend(outbox, &message, maxWaitTicks);
}


/**
 * Generates a compound command string from the given commands and appends them to the outbox queue to be sent by the TCP Sender Task
 * If the outbox queue is full, this function will block and wait for as long as possible until free space becomes available
 * Note: This is a wrapper function for @ref try_to_send_command_list
 * @param command_list A list of commands to send
 * @param num_commands How many commands the given command list contains
 */
void send_command_list(carbasic_command_t* command_list, int num_commands) {
	try_to_send_command_list(command_list, num_commands, -1);
}


/**
 * Adds a command to the outbox queue to be sent by the TCP Sender Task
 * If the outbox is full, this commmand will block until space is made available
 * Note: This is a wrapper function for @ref send_command_list
 * @param command The single command to send
 * @param command The command to be sent
 */
void send_command(carbasic_command_t command) {
	send_command_list(&command, 1);
}


/**
 * Tries to add a command to the outbox queue to be sent by the TCP Sender Task
 * If the outbox queue is full, this command will wait until either:
 * Space is made available in the outbox queue
 * or The given time limit is reach.
 * Note: This is a wrapper function for @ref try_to_send_command_list
 * @param command The single command to send
 * @param >0 maxWaitTime The maximum time (in miliseconds) to wait if outbox queue is full; 0 to not wait at all; -1 to wait as long as possible
 * @returns pdTRUE if command was successfully appended to outbox queue
 * @returns pdFALSE if failed to append comman to the outbox queue
 */
int try_to_send_command(carbasic_command_t command, int maxWaitTime) {
	return try_to_send_command_list(&command, 1, maxWaitTime);
}


/* --------------- SUPPORT FUNCTIONS --------------- */
void start_accepting_new_connections() {
	xEventGroupSetBits(tcp_events, event_accepting);
}

void stop_accepting_new_connections() {
	xEventGroupClearBits(tcp_events, event_accepting);
}

void disconnect() {
	closesocket(connection);
	connection = -1;
	xEventGroupClearBits(tcp_events, event_connected);
}

/* --------------- STATIC FUNCTIONS --------------- */
/**
 * Parses a given string into a CarBasic Command type
 * Assumes that the string is in the form "letter":value
 * where letter is a single character and is surrounded by ASCII quotation marks
 * also assumes that value is a valid integer or float
 * The created command object will contain the int value or both the int and float value (if the value is a float)
 */
static carbasic_command_t string_to_command(char* string, int len) {
	carbasic_command_t result;
	if(len < 5) {
		result.command_type = CARBASIC_INVALID_COMMAND; // indicate an invalid command string
		return result;
	}

	result.command_type = string[1];
	string += 4; // set the string pointer to refer to the first character after the ':' divider
	char* pValid;
	int value = strtol(string, &pValid, 10);

	if(pValid == string) { // failed to convert
		result.command_type = CARBASIC_INVALID_COMMAND;
		return result;
	}
	else {
		if(*pValid == '\0') { // the entire string was converted into an int
			result.value_int = value;
			result.value_type = INT;
			return result;
		}
		// conversion stopped at a decimal point, the string might be a float
		else if(*pValid == TCP_DECIMAL_CHAR) {
			result.value_int = value;  // save the int value
			// now try converting to a float
			float value = strtof(string, &pValid);
			if(*pValid == '\0') {
				result.value_float = value;
				result.value_type = FLOAT;
				return result;
			} else {
				result.value_type = INT;
				return result;
			}
		} else { // we were able to extract an integer but the string contained invalid characters
			result.command_type = CARBASIC_INVALID_COMMAND;
			return result;
		}
	}



}


/**
 * Returns the most recent error code related to the given socket
 * (taken from the "Kolban ESP32" book)
 */
/*
static int espx_last_socket_errno(int socket) {
	int ret = 0;
	u32_t optlen = sizeof(ret);
	getsockopt(socket, SOL_SOCKET, SO_ERROR, &ret, &optlen);
	return ret;
}
*/
