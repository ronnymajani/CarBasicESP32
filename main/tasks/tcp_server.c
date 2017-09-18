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
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "tcp_server.h"
#include "carbasic_protocol.h"


/* Static Function Prototypes */
//static int espx_last_socket_errno(int);

/* Global Variables */
static const char* TAG = "TCP Server";

static int connection = -1;
static EventGroupHandle_t tcp_events;
static const int event_accepting = (1<<0);
static const int event_connected = (1<<1);

static QueueHandle_t received_messages;
QueueHandle_t command_queue;
QueueHandle_t outbox;

/* Function Prototypes */
static void tcp_listener_task(void*);
static void tcp_receiver_task(void*);
static void tcp_sender_task(void*);

void start_accepting_new_connections();
void stop_accepting_new_connections();
void disconnect();


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

	/* TCP Listener Task */
	ESP_LOGV(TAG, "Creating TCP Sender Task");
	if((xTaskCreate(&tcp_sender_task, TASK_TCP_SENDER_NAME, TASK_TCP_SENDER_STACK_SIZE, NULL, TASK_TCP_SENDER_PRIORITY, NULL)) != pdPASS) {
		ESP_LOGE(TAG, "Failed to create TCP Sender Task");
		for(;;);
	}
}



/**
 * Deals with sending messages
 */
static void tcp_sender_task(void* pvParameters) {
	static const char* TAG = "TCP Sender Task";
	ESP_EARLY_LOGV(TAG, "Task Started");

	while(1){
			// make sure we are connected
			if((xEventGroupWaitBits(tcp_events, event_connected, pdFALSE, pdFALSE, portMAX_DELAY) & event_connected) == 0) {
				continue;
			}
			char* msg;
			xQueueReceive(outbox, &msg, portMAX_DELAY);
			int retCode = send(connection, msg, strlen(msg), 0);

			if(retCode == -1) {
				ESP_EARLY_LOGE(TAG, "Failed to send message:\n%s\non socket %d", msg, connection);
				free(msg);
				abort();
			} else {
				ESP_EARLY_LOGV(TAG, "Message Sent >> %s", msg);
				free(msg);
			}
	}
}


/**
 * Deals with incoming messages
 */
static void tcp_receiver_task(void* pvParameters) {
	static const char* TAG = "TCP Receiver Task";
	ESP_EARLY_LOGV(TAG, "Task Started");

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
			ESP_EARLY_LOGI(TAG, "Client Socket [%d] disconnected. Removing...", connection);
			disconnect();
			start_accepting_new_connections();
		} else {
			char* msg = malloc(retVal+1);
			memcpy(msg, buff, retVal);
			msg[retVal] = '\0';
			ESP_EARLY_LOGV(TAG, "Received data = %s", msg);
			xQueueSendToBack(received_messages, msg, portMAX_DELAY);
		}
	}
}


/**
 * Binds a port to the application,
 * Listens for incoming connections,
 * and accepts them.
 */
static void tcp_listener_task(void* pvParameters) {
	static const char* TAG = "TCP Listener Task";
	ESP_EARLY_LOGV(TAG, "Task Started");

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
		ESP_EARLY_LOGV(TAG, "Accepting new connections");
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


/* --------------- STATIC FUNCTIONS --------------- */
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
