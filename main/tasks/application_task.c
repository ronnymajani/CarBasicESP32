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

static const char* TAG = "APPLICATION_TASK";

void application_task(void* p) {
	ESP_EARLY_LOGV(TAG, "Beginning Application Task");

	// Task Loop
	for(;;) {
		vTaskDelay(2000/portTICK_PERIOD_MS);
		printf(".");
		fflush(stdout);
	}
}
