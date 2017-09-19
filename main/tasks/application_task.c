/*
 * application_task.c
 *
 *  Created on: Sep 14, 2017
 *      Author: ronnymajani
 */
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "carbasic_protocol.h"
#include "carbasic_state.h"
#include "tcp_service.h"
#include "motor_driver.h"

static const char* TAG = "APPLICATION_TASK";

static void parse_new_command();
static void broadcast_state();
static void move_car();
static void print_a_command();

static carbasic_state_t car_state;


void application_task(void* p) {
	ESP_EARLY_LOGV(TAG, "Beginning Application Task");
	car_state = carbasic_new_state();

	// Task Loop
	for(;;) {
//		if(command_is_available()) {
//			parse_new_command();
//		}
//		move_car();
//		broadcast_state();
		parse_new_command();
		broadcast_state();
	}
}



static void parse_new_command() {
	carbasic_command_t command = get_command();
	print_a_command(command);
	switch(command.command_type) {
		case CARBASIC_COMMAND_PWM_RIGHT: {
			float pwm = command.value_float;
			if(pwm > 0) {pwm = MOTOR_DRIVER_MAX_PWM / pwm;}
			car_state.pwm_right = pwm;
			set_motor_right_pwm((int)pwm);
		}	break;
		case CARBASIC_COMMAND_PWM_LEFT: {
			float pwm = command.value_float;
			if(pwm > 0) {pwm = MOTOR_DRIVER_MAX_PWM / pwm;}
			car_state.pwm_left = pwm;
			set_motor_left_pwm((int)pwm);
		}	break;
		case CARBASIC_COMMAND_MOTOR_RIGHT_ENABLE: {
			if(command.value_int == CARBASIC_COMMAND_VALUE_ENABLE)
				enable_motor_right();
			else
				disable_motor_right();
		}	break;
		case CARBASIC_COMMAND_MOTOR_LEFT_ENABLE: {
			if(command.value_int == CARBASIC_COMMAND_VALUE_ENABLE)
				enable_motor_left();
			else
				disable_motor_left();
		}	break;
		case CARBASIC_COMMAND_DIRECTION_RIGHT_FORWARD: {
			if(command.value_int == CARBASIC_COMMAND_VALUE_DIRECTION_FORWARD)
				set_motor_right_direction_forward();
			else
				set_motor_right_direction_reverse();
		}	break;
		case CARBASIC_COMMAND_DIRECTION_LEFT_FORWARD: {
			if(command.value_int == CARBASIC_COMMAND_VALUE_DIRECTION_FORWARD)
				set_motor_left_direction_forward();
			else
				set_motor_left_direction_reverse();
		}	break;
		case CARBASIC_COMMAND_SENSOR_ORIENTATION:
			break;
		case CARBASIC_INVALID_COMMAND:
			ESP_EARLY_LOGE(TAG, "Invalid Command Found; Ignoring...");
			break;
		default:
			ESP_EARLY_LOGE(TAG, "Unknown Command [%c]; Ignoring...", command.command_type);
			break;
	}
}



static void broadcast_state() {
	carbasic_command_t commands[8];

	commands[0].command_type = CARBASIC_STATE_X;
		commands[0].value_type = FLOAT;
		commands[0].value_float = car_state.x;
	commands[1].command_type = CARBASIC_STATE_Y;
		commands[1].value_type = FLOAT;
		commands[1].value_float = car_state.y;
	commands[2].command_type = CARBASIC_STATE_ORIENTATION;
		commands[2].value_type = FLOAT;
		commands[2].value_float = car_state.orientation;
	commands[3].command_type = CARBASIC_STATE_SENSOR_MEASUREMENT;
		commands[3].value_type = FLOAT;
		commands[3].value_float = car_state.sensor_measurement;
	commands[4].command_type = CARBASIC_STATE_SENSOR_ORIENTATION;
		commands[4].value_type = FLOAT;
		commands[4].value_float = car_state.sensor_orientation;
	commands[5].command_type = CARBASIC_STATE_SPEED;
		commands[5].value_type = FLOAT;
		commands[5].value_float = car_state.speed;
	commands[6].command_type = CARBASIC_STATE_PWM_RIGHT;
		commands[6].value_type = INT;
		commands[6].value_int = car_state.pwm_right / MOTOR_DRIVER_MAX_PWM;
	commands[7].command_type = CARBASIC_STATE_PWM_LEFT;
		commands[7].value_type = INT;
		commands[7].value_int = car_state.pwm_left / MOTOR_DRIVER_MAX_PWM;

	send_command_list(commands, 8);
}


static void move_car() {
	static int i = 0;
	car_state.x = i;
	car_state.y = i;
	i += 1;
	i %= 150;
}



static void print_a_command(carbasic_command_t cmd) {
	if(cmd.command_type == CARBASIC_INVALID_COMMAND) {
		printf("Invalid Command: [%c]\n", cmd.command_type);
	} else {
		printf("Command:\n"
				"--command: %c\n", cmd.command_type);
		if(cmd.value_type == INT) {
			printf("--type: INT\n"
					"--value: %d\n", cmd.value_int);
		} else if(cmd.value_type == FLOAT) {
			printf("--type: FLOAT\n"
					"--value(int): %d\n"
					"--value(float): %f\n", cmd.value_int, cmd.value_float);
		}
	}
}
