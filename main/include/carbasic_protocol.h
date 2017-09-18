/*
 * carbasic_protocol.h
 *
 *  Created on: Sep 18, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_INCLUDE_CARBASIC_PROTOCOL_H_
#define MAIN_INCLUDE_CARBASIC_PROTOCOL_H_

typedef struct {
	char command;
	enum {INT, FLOAT} type;
	int value_int;  // use if type is set to INT
	float value_float;  // use if type is set to FLOAT
} carbasic_command_t;


#define CARBASIC_COMMAND_INVALID '-'

#endif /* MAIN_INCLUDE_CARBASIC_PROTOCOL_H_ */
