/*
 * tcp_server.c
 *
 *  Created on: Sep 15, 2017
 *      Author: ronnymajani
 */

#include "esp_log.h"
#include "lwip/sockets.h"

#include "tcp_server.h"


/* Static Function Prototypes */
static int espx_last_socket_errno(int);
static void initialize_connections();
static void add_connection(int);
static void remove_connection(int);
static void get_connections_fdset(fd_set*);

/* Global Variables */
static const char* TAG = "TCP Server";
static struct {
	int numberOfConnections;
	int* sockets;
} connections;





/**
 * Deals with incoming messages
 */
void tcp_server_task(void* pvParameters) {
	static const char* TAG = "TCP Server Task";
	ESP_EARLY_LOGV(TAG, "Task Started");

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	while(1) {
		fd_set readfds;
		int retCode;

		get_connections_fdset(&readfds);
		retCode = select(connections.numberOfConnections, &readfds, NULL, NULL, &timeout);

		if(retCode == -1) { // error
			ESP_EARLY_LOGE(TAG, "Error during function SELECT (TCP Socket Programming)");
			abort();
		} else if(retCode > 0) { // >0 means that at least 1 socket is ready to be read
			for(int i = 0; i < connections.numberOfConnections; i++) {
				int sock = connections.sockets[i];
				if(FD_ISSET(sock, &readfds)) { // current socket is ready for reading
					//TODO: do something here?
					char buff[10];
					int retVal = recv(sock, buff, sizeof(char) * 10, 0);
					if(retVal == -1) {
						ESP_EARLY_LOGE(TAG, "Error during function RECV (TCP Socket Programming)");
						abort();
					} else if(retVal == 0) {
						remove_connection(sock);
					} else {
						printf("Received Command: %s", buff);
					}
				}
			}
		}
	}

}


/**
 * Binds a port to the application,
 * Listens for incoming connections,
 * and accepts them.
 */
void tcp_listener_task(void* pvParameters) {
	static const char* TAG = "TCP Listener Task";
	ESP_EARLY_LOGV(TAG, "Task Started");

	// initialize `connections` global variable
	initialize_connections();

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
		struct sockaddr clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);
		int clientSock = accept(sock, (struct sockaddr*)&clientAddress, &clientAddressLength);
		if(clientSock == -1) {
			ESP_EARLY_LOGE(TAG, "Failed to Accept incoming connection");
			abort();
		}
		add_connection(clientSock);
	}
}


/* --------------- STATIC FUNCTIONS --------------- */
/**
 * Returns the most recent error code related to the given socket
 * (taken from the "Kolban ESP32" book)
 */
static int espx_last_socket_errno(int socket) {
	int ret = 0;
	u32_t optlen = sizeof(ret);
	getsockopt(socket, SOL_SOCKET, SO_ERROR, &ret, &optlen);
	return ret;
}


/**
 * initializes the 'connections' global variable
 */
static void initialize_connections() {
	connections.numberOfConnections = 0;
	connections.sockets = NULL;
}

/**
 * Adds a connection to the global list of connections
 */
static void add_connection(int clientSocket) {
	if(connections.sockets == NULL) {
		connections.sockets = malloc(sizeof(int));
		if(connections.sockets == NULL) {
			ESP_EARLY_LOGE(TAG, "Failed to allocate memory for new connection");
			abort();
		}
	}
	else {
		connections.sockets = realloc(connections.sockets, sizeof(int) * (connections.numberOfConnections + 1));
		if(connections.sockets == NULL) {
			ESP_EARLY_LOGE(TAG, "Failed to re-allocate memory for new connection");
			abort();
		}
	}
	connections.sockets[connections.numberOfConnections] = clientSocket;
	connections.numberOfConnections++;
}


/**
 * Remove a given connection socket
 */
static void remove_connection(int sock) {
	int n = connections.numberOfConnections;
	int index = -1;
	int* socks = connections.sockets;

	// find the socket to remove
	for(int i = 0; i < n; i++) {
		if(socks[i] == sock) {
			index = i;
			break;
		}
	}

	if(index >= 0) {
		closesocket(socks[index]);
		// reorder list
		for(int i = index; i < n-1; i++) {
			socks[i] = socks[i+1];
		}
		// shrink size of list
		connections.numberOfConnections--;
		connections.sockets = realloc(connections.sockets, sizeof(int) * connections.numberOfConnections);
		if(connections.sockets == NULL) {
			ESP_EARLY_LOGE(TAG, "Failed to re-allocate less memory for connections.sockets list");
			abort();
		}
	}
}

/**
 * Returns an FD_SET containing all the connections
 */
static void get_connections_fdset(fd_set* connections_fds) {
	FD_ZERO(connections_fds);

	for(int i = 0; i < connections.numberOfConnections; i++) {
		FD_SET(connections.sockets[i], connections_fds);
	}
}
