/*
 * command_parser_task.h
 *
 *  Created on: Sep 24, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_TASKS_INCLUDE_HOST_INTERFACE_TASK_H_
#define MAIN_TASKS_INCLUDE_HOST_INTERFACE_TASK_H_


#define TASK_HOST_INTERFACE_NAME "HOST_INTERFACE"
#define TASK_HOST_INTERFACE_PRIORITY 4
#define TASK_HOST_INTERFACE_STACK_SIZE 4096

void host_interface_task(void*);


#endif /* MAIN_TASKS_INCLUDE_HOST_INTERFACE_TASK_H_ */
