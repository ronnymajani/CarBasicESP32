#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

/* Application Includes */
#include "carbasic_main.h"
#include "wifi.h"
#include "motor_driver.h"
#include "ultrasonic_driver.h"
#include "servo_driver.h"
#include "application_task.h"
#include "host_interface_task.h"
#include "tcp_service.h"


/* Static Definitions */
static const char* TAG = "MAIN";

/* Function Prototypes */
static void setup();
static void setLogLevels();
static void createTasks();


void app_main()
{
	ESP_LOGI(TAG, "Starting Main Task");

	setup();
	setLogLevels();
	createTasks();

	ESP_LOGI(TAG, "All Tasks Created");
}



static void setLogLevels() {
//	esp_log_level_set("*", ESP_LOG_VERBOSE);
}



static void createTasks() {
	BaseType_t retCode;

	init_tcp_service();
	/* Application Task */
	ESP_LOGV(TAG, "Creating Application Task");
	retCode = (xTaskCreate(&application_task, TASK_APP_NAME, TASK_APP_STACK_SIZE, NULL, TASK_APP_PRIORITY, NULL));

	if(retCode != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Application Task");
		for(;;);
	}

	/* Host Interface Task */
	ESP_LOGV(TAG, "Creating Host Interface Task");
	retCode = (xTaskCreate(&host_interface_task, TASK_HOST_INTERFACE_NAME, TASK_HOST_INTERFACE_STACK_SIZE, NULL, TASK_HOST_INTERFACE_PRIORITY, NULL));

	if(retCode != pdPASS) {
		ESP_LOGE(TAG, "Failed to create Host Interface Task");
		for(;;);
	}
}



static void setup() {
	ESP_LOGV(TAG, "Starting Setup Function");
	wifi_init_sta();
	motor_driver_init();
	ultrasonic_driver_init();
	servo_driver_init();
}
