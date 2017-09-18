#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

/* Application Includes */
#include "carbasic_main.h"
#include "wifi.h"
#include "tasks/include/application_task.h"
#include "tasks/include/tcp_server.h"


/* Static Definitions */
static const char* TAG = "MAIN";

/* Function Prototypes */
static void setup();
static void setLogLevels();
static void createTasks();


void app_main()
{
	ESP_LOGV(TAG, "Starting Main Task");

	setup();
	setLogLevels();
	createTasks();

    ESP_LOGV(TAG, "Starting RTOS Scheduler");
}



static void setLogLevels() {
//	esp_log_level_set("*", ESP_LOG_VERBOSE);
}



static void createTasks() {
	BaseType_t retCode;

	/* Application Task */
	ESP_LOGV(TAG, "Creating Application Task");
	retCode = (xTaskCreate(&application_task, TASK_APP_NAME, TASK_APP_STACK_SIZE, NULL, TASK_APP_PRIORITY, NULL));

	if(retCode != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Application Task");
		for(;;);
	}

	init_tcp_service();
}



static void setup() {
	ESP_LOGV(TAG, "Starting Setup Function");
	wifi_init_sta();
}
