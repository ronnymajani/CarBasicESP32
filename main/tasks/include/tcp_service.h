/*
 * tcp_server.h
 *
 *  Created on: Sep 15, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_TASKS_INCLUDE_TCP_SERVICE_H_
#define MAIN_TASKS_INCLUDE_TCP_SERVICE_H_

#include "carbasic_protocol.h"

#define TCP_SERVER_PORT 9900
#define TCP_SERVER_BACKLOG 1

#define TCP_MAX_SIZE_MESSAGE 1024
#define TCP_RECEIVED_MESSAGES_BUFFER_SIZE 10  // maximum number of received TCP strings to store before waiting until they are parsed
#define TCP_COMMAND_BUFFER_SIZE 50  // maximum number of received commands to store before waiting until they are processed
#define TCP_OUTBOX_BUFFER_SIZE 10  // maximum number of messages (that need to be sent) to store before waiting until they are sent


/* Tasks */
#define TASK_TCP_LISTENER_NAME "TCP_Listener"
#define TASK_TCP_LISTENER_PRIORITY 5
#define TASK_TCP_LISTENER_STACK_SIZE 4096

#define TASK_TCP_RECEIVER_NAME "TCP_Receiver"
#define TASK_TCP_RECEIVER_PRIORITY 3
#define TASK_TCP_RECEIVER_STACK_SIZE 4096

#define TASK_TCP_SENDER_NAME "TCP_Sender"
#define TASK_TCP_SENDER_PRIORITY 3
#define TASK_TCP_SENDER_STACK_SIZE 4096

#define TASK_TCP_MESSAGE_PARSER_NAME "TCP_Message_Parser"
#define TASK_TCP_MESSAGE_PARSER_PRIORITY 3
#define TASK_TCP_MESSAGE_PARSER_STACK_SIZE 4096

/* TAGS */
#define TCP_TAG_START '{'
#define TCP_TAG_SEPARATOR ','
#define TCP_TAG_END '}'
#define TCP_DECIMAL_CHAR '.'

/* Exported Functions */
void init_tcp_service();
carbasic_command_t get_command();
int command_is_available();

int try_to_send_command_list(carbasic_command_t*, int, int);
void send_command_list(carbasic_command_t*, int);
int try_to_send_command(carbasic_command_t, int);
void send_command(carbasic_command_t);

#endif /* MAIN_TASKS_INCLUDE_TCP_SERVICE_H_ */
