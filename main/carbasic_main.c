#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

/* Application Includes */
#include "wifi.h"
#include "tasks/application_task.h"

/* Macro Definitions */
#define STACK_SIZE configMINIMAL_STACK_SIZE

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

	ESP_LOGV(TAG, "Creating Application Task");
	retCode = (xTaskCreate(&application_task, "APP", STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL));

	if(retCode != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Application Task");
		abort();
	}
}



static void setup() {
	ESP_LOGV(TAG, "Starting Setup Function");
	wifi_init_softap();
}
