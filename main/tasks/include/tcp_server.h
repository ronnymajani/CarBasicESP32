/*
 * tcp_server.h
 *
 *  Created on: Sep 15, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_TASKS_INCLUDE_TCP_SERVER_H_
#define MAIN_TASKS_INCLUDE_TCP_SERVER_H_


#define TCP_SERVER_PORT 9900
#define TCP_SERVER_BACKLOG 1

#define TCP_MAX_SIZE_MESSAGE 1024
#define TCP_RECEIVED_MESSAGES_BUFFER_SIZE 10  // maximum number of received TCP strings to store before waiting until they are parsed
#define TCP_COMMAND_BUFFER_SIZE 50  // maximum number of received commands to store before waiting until they are processed
#define TCP_OUTBOX_BUFFER_SIZE 10  // maximum number of messages (that need to be sent) to store before waiting until they are sent


/* Tasks */
#define TASK_TCP_LISTENER_NAME "TCP_Listener"
#define TASK_TCP_LISTENER_PRIORITY 5
#define TASK_TCP_LISTENER_STACK_SIZE 2048

#define TASK_TCP_RECEIVER_NAME "TCP_Receiver"
#define TASK_TCP_RECEIVER_PRIORITY 3
#define TASK_TCP_RECEIVER_STACK_SIZE 2048

#define TASK_TCP_SENDER_NAME "TCP_Sender"
#define TASK_TCP_SENDER_PRIORITY 3
#define TASK_TCP_SENDER_STACK_SIZE 2048

#define TCP_DECIMAL_CHAR '.'

/* Exported Functions */
void init_tcp_service();

#endif /* MAIN_TASKS_INCLUDE_TCP_SERVER_H_ */
