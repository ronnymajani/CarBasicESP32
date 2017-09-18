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

QueueHandle_t received_messages;
QueueHandle_t command_queue;
QueueHandle_t outbox;

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
	ESP_EARLY_LOGI(TAG, "Task Started");

	char buffer[TCP_MAX_SIZE_MESSAGE];
	int buffer_tail = 0;
	while(1) {
		char* msg;
		if(xQueueReceive(received_messages, &msg, portMAX_DELAY)) {
//			ESP_EARLY_LOGV(TAG, "Received Message to Parse: %s", msg);
			for(char* c = msg; *c != '\0'; c++) {
				if(*c == '{' || isspace((int)*c)) { // ignore
					continue;
				}
				else if(*c == '}' || *c == ',') { // parse current buffer
					buffer[buffer_tail++] = '\0';
					ESP_EARLY_LOGV(TAG, "Parsing substring: %s", buffer);
					carbasic_command_t command = string_to_command(buffer, buffer_tail);
					if(command.command != CARBASIC_COMMAND_INVALID) {
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
	ESP_EARLY_LOGI(TAG, "Task Started");

	while(1){
			// make sure we are connected
			if((xEventGroupWaitBits(tcp_events, event_connected, pdFALSE, pdFALSE, portMAX_DELAY) & event_connected) == 0) {
				continue;
			}
			char* msg;
			if(xQueueReceive(outbox, &msg, portMAX_DELAY)) {
				int retCode = send(connection, msg, strlen(msg), 0);
				if(retCode == -1) {
					ESP_EARLY_LOGE(TAG, "Failed to send message:\n%s\non socket %d", msg, connection);
					free(msg);
					abort();
				} else {
//					ESP_EARLY_LOGV(TAG, "Message Sent >> %s", msg);
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
	ESP_EARLY_LOGI(TAG, "Task Started");

	while(1){
		// make sure we are connected
		if((xEventGroupWaitBits(tcp_events, event_connected, pdFALSE, pdFALSE, portMAX_DELAY) & event_connected) == 0) {
			continue;
		}
		char buff[100];
		int retVal = recv(connection, buff, sizeof(char) * 100, 0);
		ESP_EARLY_LOGV(TAG, "Received data from socket: %d");
		if(retVal == -1) {
			ESP_EARLY_LOGE(TAG, "Error during function RECV (TCP Socket Programming)");
			disconnect();
			stop_accepting_new_connections();
			abort();  // todo: handle this error

		} else if(retVal == 0) {
			ESP_EARLY_LOGI(TAG, "Client Socket [%d] disconnected...", connection);
			disconnect();
			start_accepting_new_connections();
		} else {
			char* msg = malloc(retVal+1);
			memcpy(msg, buff, retVal);
			msg[retVal] = '\0';
//			ESP_EARLY_LOGV(TAG, "Received data = %s", msg);
			xQueueSend(received_messages, &msg, portMAX_DELAY);
//			ESP_EARLY_LOGV(TAG,"Appended to Queue");
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
	ESP_EARLY_LOGI(TAG, "Task Started");

	int retCode;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(TCP_SERVER_PORT);

	retCode = bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if(retCode != 0) {
		ESP_EARLY_LOGE(TAG, "Failed to Bind Socket");
		abort();
	}

	retCode = listen(sock, TCP_SERVER_BACKLOG);
	if(retCode != 0) {
		ESP_EARLY_LOGE(TAG, "Failed to Start Listening on Socket");
		abort();
	}

	while(1) {
		EventBits_t bits = xEventGroupWaitBits(tcp_events, event_accepting, pdTRUE, pdFALSE, portMAX_DELAY);
		if((bits & event_accepting) == 0){  // make sure the bits were set (to check if a timeout occurred)
			continue;
		}
		struct sockaddr_in clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);
		ESP_EARLY_LOGV(TAG, "Accepting new connection");
		int clientSock = accept(sock, (struct sockaddr*)&clientAddress, &clientAddressLength);

		if(clientSock == -1) {
			ESP_EARLY_LOGE(TAG, "Failed to Accept incoming connection");
			abort();
		}

		ESP_EARLY_LOGI(TAG, "New Client Connected, socket:%d, ip:%s, port:%d",
				clientSock,
				ip4addr_ntoa((ip4_addr_t*)&(clientAddress.sin_addr.s_addr)),
				clientAddress.sin_port);

		connection = clientSock;
		xEventGroupSetBits(tcp_events, event_connected);  //
	}
}


/* --------------- EXPORTED FUNCTIONS --------------- */
carbasic_command_t get_command() {
	carbasic_command_t result;
	xQueueReceive(command_queue, &result, portMAX_DELAY);
	return result;
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
		result.command = CARBASIC_COMMAND_INVALID; // indicate an invalid command string
		return result;
	}

	result.command = string[1];
	string += 4; // set the string pointer to refer to the first character after the ':' divider
	char* pValid;
	int value = strtol(string, &pValid, 10);

	if(pValid == string) { // failed to convert
		result.command = CARBASIC_COMMAND_INVALID;
		return result;
	}
	else {
		if(*pValid == '\0') { // the entire string was converted into an int
			result.value_int = value;
			result.type = INT;
			return result;
		}
		// conversion stopped at a decimal point, the string might be a float
		else if(*pValid == TCP_DECIMAL_CHAR) {
			result.value_int = value;  // save the int value
			// now try converting to a float
			float value = strtof(string, &pValid);
			if(*pValid == '\0') {
				result.value_float = value;
				result.type = FLOAT;
				return result;
			} else {
				result.type = INT;
				return result;
			}
		} else { // we were able to extract an integer but the string contained invalid characters
			result.command = CARBASIC_COMMAND_INVALID;
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
