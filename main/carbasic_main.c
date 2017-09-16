#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

/* Application Includes */
#include "wifi.h"
#include "tasks/include/application_task.h"
#include "tasks/include/tcp_server.h"

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

	/* Application Task */
	ESP_LOGV(TAG, "Creating Application Task");
	retCode = (xTaskCreate(&application_task, "APP", STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL));

	if(retCode != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Application Task");
		for(;;);
	}

	/* TCP Server Task */
	ESP_LOGV(TAG, "Creating TCP Server Task");
	retCode = (xTaskCreate(&tcp_server_task, "TCP_Server", 3000, NULL, 2, NULL));

	if(retCode != pdPASS) {
		ESP_LOGE(TAG, "Failed to create TCP Server Task");
		for(;;);
	}

	/* TCP Listener Task */
	ESP_LOGV(TAG, "Creating TCP Listener Task");
	retCode = (xTaskCreate(&tcp_listener_task, "TCP_Listener", 3000, NULL, 3, NULL));

	if(retCode != pdPASS) {
		ESP_LOGE(TAG, "Failed to create TCP Listener Task");
		for(;;);
	}

	//TODO: Adjust the priorities of the Tasks

}



static void setup() {
	ESP_LOGV(TAG, "Starting Setup Function");
	wifi_init_sta();
}
