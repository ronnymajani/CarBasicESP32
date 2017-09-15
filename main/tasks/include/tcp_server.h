/*
 * tcp_server.h
 *
 *  Created on: Sep 15, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_TASKS_INCLUDE_TCP_SERVER_H_
#define MAIN_TASKS_INCLUDE_TCP_SERVER_H_


#define TCP_SERVER_PORT 990
#define TCP_SERVER_BACKLOG 1  // max simultaneous incoming connections to keep waiting



void tcp_server_task(void*);
void tcp_listener_task(void*);


#endif /* MAIN_TASKS_INCLUDE_TCP_SERVER_H_ */
