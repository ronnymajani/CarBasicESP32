/*
 * command_parser_task.c
 *
 *  Created on: Sep 24, 2017
 *      Author: ronnymajani
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "host_interface_task.h"
#include "tcp_service.h"
#include "motor_driver.h"
#include "carbasic_protocol.h"
#include "carbasic_state.h"

#define REFRESH_INTERVAL_MS 50


/* Function Prototypes */
static void parse_new_command();
static void print_a_command(carbasic_command_t);

/* Global Variables */
static const char* TAG = "Host Interface";
extern carbasic_state_t car_state;



void host_interface_task(void* pvParameters) {
	for(;;) {
		wait_until_connected();  // if no connection is available, bloc the task
		parse_new_command();
	}
}



static void parse_new_command() {
	carbasic_command_t command = get_command();
//	print_a_command(command);

	switch(command.command_type) {
		case CARBASIC_COMMAND_PWM_RIGHT: {
			if(command.value_type == INT) {
				int pwm = MOTOR_DRIVER_MAX_PWM * command.value_float / 100.0f;
				car_state.pwm_right = pwm;
				set_motor_right_pwm(pwm);
			} else {
				float pwm = MOTOR_DRIVER_MAX_PWM * command.value_float / 100.0f;
				car_state.pwm_right = pwm;
				set_motor_right_pwm((int)pwm);
			}
		}	break;
		case CARBASIC_COMMAND_PWM_LEFT: {
			if(command.value_type == INT) {
				int pwm = MOTOR_DRIVER_MAX_PWM * command.value_float / 100.0f;
				car_state.pwm_left = pwm;
				set_motor_left_pwm(pwm);
			} else {
				float pwm = MOTOR_DRIVER_MAX_PWM * command.value_float / 100.0f;
				car_state.pwm_left = pwm;
				set_motor_left_pwm((int)pwm);
			}
		}	break;
		case CARBASIC_COMMAND_DIRECTION_RIGHT_FORWARD: {
			if(command.value_int == CARBASIC_COMMAND_VALUE_DIRECTION_FORWARD)
				set_motor_right_direction_forward();
			else if(command.value_int == CARBASIC_COMMAND_VALUE_DIRECTION_REVERSE)
				set_motor_right_direction_reverse();
			else
				disable_motor_right();
		}	break;
		case CARBASIC_COMMAND_DIRECTION_LEFT_FORWARD: {
			if(command.value_int == CARBASIC_COMMAND_VALUE_DIRECTION_FORWARD)
				set_motor_left_direction_forward();
			else if(command.value_int == CARBASIC_COMMAND_VALUE_DIRECTION_REVERSE)
				set_motor_left_direction_reverse();
			else
				disable_motor_left();
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
