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
#include "tcp_service.h"

static const char* TAG = "APPLICATION_TASK";


void print_a_command() {
	carbasic_command_t cmd = get_command();
	if(cmd.command == CARBASIC_COMMAND_INVALID) {
		printf("Received Invalid Commands: [%d]\n", cmd.command);
	} else {
		printf("Received a Command:\n"
				"--command: %c\n", cmd.command);
		if(cmd.type == INT) {
			printf("--type: INT\n"
					"--value: %d\n", cmd.value_int);
		} else if(cmd.type == FLOAT) {
			printf("--type: FLOAT\n"
					"--value(int): %d\n"
					"--value(float): %f\n", cmd.value_int, cmd.value_float);
		}
	}
}


void application_task(void* p) {
	ESP_EARLY_LOGV(TAG, "Beginning Application Task");
	int i = 0;
	// Task Loop
	for(;;) {
		if(command_is_available()) {
			print_a_command();
		}
		carbasic_command_t cmd;
		cmd.command = 'x';
		cmd.type = INT;
		cmd.value_int = i;
		send_command(cmd);

		cmd.command = 'y';
		send_command(cmd);

		i += 1;
		i %= 150;

		/*
		vTaskDelay(2000/portTICK_PERIOD_MS);
		printf(".");
		fflush(stdout);
		*/
	}
}
