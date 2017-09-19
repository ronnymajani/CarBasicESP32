/*
 * carbasic_protocol.h
 *
 *  Created on: Sep 18, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_INCLUDE_CARBASIC_PROTOCOL_H_
#define MAIN_INCLUDE_CARBASIC_PROTOCOL_H_

typedef struct {
	char command_type;
	enum {INT, FLOAT} value_type;
	int value_int;  // use if type is set to INT
	float value_float;  // use if type is set to FLOAT
} carbasic_command_t;

#define CARBASIC_INVALID_COMMAND  '-'

#define CARBASIC_STATE_X  'x'
#define CARBASIC_STATE_Y  'y'
#define CARBASIC_STATE_ORIENTATION  'o'
#define CARBASIC_STATE_SENSOR_MEASUREMENT  's'
#define CARBASIC_STATE_SENSOR_ORIENTATION  'z'
#define CARBASIC_STATE_SPEED  'v'
#define CARBASIC_STATE_PWM_LEFT 'l'
#define CARBASIC_STATE_PWM_RIGHT  'r'

#define CARBASIC_COMMAND_PWM_LEFT  'l'
#define CARBASIC_COMMAND_PWM_RIGHT  'r'
#define CARBASIC_COMMAND_MOTOR_RIGHT_ENABLE  'm'  // used to enable/disable right motor
#define CARBASIC_COMMAND_MOTOR_LEFT_ENABLE  'n'  // used to enable/disable left motor
#define CARBASIC_COMMAND_DIRECTION_RIGHT_FORWARD  'e'  // used to set the direction of the right motor
#define CARBASIC_COMMAND_DIRECTION_LEFT_FORWARD  'w'  // used to set the direction of the left motor
#define CARBASIC_COMMAND_SENSOR_ORIENTATION  's'  // used to set the sensor orientation

#define CARBASIC_COMMAND_VALUE_DIRECTION_FORWARD  1
#define CARBASIC_COMMAND_VALUE_DIRECTION_REVERSE  0
#define CARBASIC_COMMAND_VALUE_ENABLE  1
#define CARBASIC_COMMAND_VALUE_DISABLE  0

#endif /* MAIN_INCLUDE_CARBASIC_PROTOCOL_H_ */
