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
#include "sys/time.h"

#include "motor_driver.h"
#include "carbasic_protocol.h"
#include "carbasic_state.h"
#include "tcp_service.h"
#include "ultrasonic_driver.h"
#include "servo_driver.h"
#include "../tasks/include/tcp_service.h"

#define REFRESH_INTERVAL_MS 75

/* Function Prototypes */
static int rotate_servo();
static void update_state();
static void broadcast_state();

/* Global Variables */
static const char* TAG = "APPLICATION_TASK";
carbasic_state_t car_state;



void application_task(void* pvParameters) {
	ESP_LOGI(TAG, "Beginning Application Task");
	car_state = carbasic_new_state();

	// Task Loop
	for(;;) {
		update_state();
		if(is_connected()) {
			broadcast_state();
		}
		vTaskDelay(REFRESH_INTERVAL_MS / portTICK_PERIOD_MS);
	}
}




static void update_state() {
	static int new_reading = 0;
	// update location
//	move_car();
	// rotate servo
	car_state.sensor_orientation = rotate_servo();
	// update sensor measurement
	if(new_reading == 0) {
		ultrasonic_trigger();
		new_reading = 1;
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
	if(ultrasonic_measurement_ready()) {
		double distance = ultrasonic_get_measurement();
		car_state.sensor_measurement = (int)distance;
//		ESP_LOGI(TAG, "Measured Distance: %dcm", (int)distance);
		new_reading = 0;
	}
}


static int rotate_servo() {
	static int i = 0;
	static int direction = -1;

	set_servo_orientation(i);

	if (i >= 60)
		direction = -1;
	else if(i <= -60)
		direction = 1;

	i += (direction) * 10;
	return i;
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
		commands[3].value_type = INT;
		commands[3].value_int = car_state.sensor_measurement;
	commands[4].command_type = CARBASIC_STATE_SENSOR_ORIENTATION;
		commands[4].value_type = FLOAT;
		commands[4].value_float = car_state.sensor_orientation;
	commands[5].command_type = CARBASIC_STATE_SPEED;
		commands[5].value_type = FLOAT;
		commands[5].value_float = car_state.speed;
	commands[6].command_type = CARBASIC_STATE_PWM_RIGHT;
		commands[6].value_type = FLOAT;
		commands[6].value_float = 100.0f * (float)car_state.pwm_right / MOTOR_DRIVER_MAX_PWM;
	commands[7].command_type = CARBASIC_STATE_PWM_LEFT;
		commands[7].value_type = FLOAT;
		commands[7].value_float = 100.0f * (float)car_state.pwm_left / MOTOR_DRIVER_MAX_PWM;

	send_command_list(commands, 8);
}
