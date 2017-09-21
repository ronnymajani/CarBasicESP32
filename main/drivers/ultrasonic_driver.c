/*
 * ultrasonic_driver.c
 *
 *  Created on: Sep 20, 2017
 *      Author: ronnymajani
 */
#include <inttypes.h>
#include "driver/gpio.h"
#include "driver/timer.h"
#include "soc/soc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "ultrasonic_driver.h"

#define EVENT_DATA_AVAILABLE (1<<0)


/* Global Variables */
static EventGroupHandle_t data_available;
static const char* TAG = "Ultrasonic Driver";

/* Function Prototypes */
IRAM_ATTR void ultrasonic_measurement_interrupt();



void ultrasonic_driver_init() {
	// setup GPIO
	gpio_config_t ultrasonic_pin_config;
		ultrasonic_pin_config.mode = GPIO_MODE_OUTPUT;
		ultrasonic_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
		ultrasonic_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
		ultrasonic_pin_config.pin_bit_mask = (1<<ULTRASONIC_DRIVER_GPIO_TRIG);
		ultrasonic_pin_config.intr_type = GPIO_PIN_INTR_DISABLE;
	gpio_config(&ultrasonic_pin_config);
		ultrasonic_pin_config.mode = GPIO_MODE_INPUT;
		ultrasonic_pin_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
		ultrasonic_pin_config.intr_type = GPIO_PIN_INTR_ANYEDGE;
		ultrasonic_pin_config.pin_bit_mask = (1<<ULTRASONIC_DRIVER_GPIO_ECHO);
	gpio_config(&ultrasonic_pin_config);
	// setup Timer
	timer_config_t config;
	    config.alarm_en = 0;
	    config.auto_reload = 1;
	    config.counter_dir = TIMER_COUNT_UP;
	    config.divider = ULTRASONIC_DRIVER_TIMER_DIVIDER;
	    config.counter_en = TIMER_START;
	timer_init(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID, &config);
	timer_pause(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID);
	// Register Interrupts
	gpio_install_isr_service(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_EDGE);
	gpio_isr_handler_add(ULTRASONIC_DRIVER_GPIO_ECHO, &ultrasonic_measurement_interrupt, NULL);
	// Initialize Queue
	data_available = xEventGroupCreate();

}



void ultrasonic_trigger() {
	// trigger the ultrasonic sensor
	gpio_set_level(ULTRASONIC_DRIVER_GPIO_TRIG, 0);  // set LOW for 5us
	vTaskDelay(0.005 / portTICK_PERIOD_MS);
	gpio_set_level(ULTRASONIC_DRIVER_GPIO_TRIG, 1);  // set HIGH for 10us
	vTaskDelay(0.01 / portTICK_PERIOD_MS);
	gpio_set_level(ULTRASONIC_DRIVER_GPIO_TRIG, 0);  // set LOW again
}


int ultrasonic_measurement_ready() {
	return (xEventGroupGetBits(data_available) & EVENT_DATA_AVAILABLE) == EVENT_DATA_AVAILABLE;
}

/**
 * Returns the measured distance in cm
 * If data is available
 * If no data is available, this function will block until a new measurement is available
 * Please use @ref ultrasonic_measurement_ready function to check for data availability before you call this function
 */
double ultrasonic_get_measurement() {
	double time;
	timer_get_counter_time_sec(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID, &time);
	// convert value to seconds
	ESP_LOGD(TAG, "Elapsed Time: %.8f", time);
	// convert time into distance
	double distance_cm = (time*1000000.0) / 58.2;
	xEventGroupSetBits(data_available, EVENT_DATA_AVAILABLE);
	return distance_cm;
}






/* Interrupts */
IRAM_ATTR void ultrasonic_measurement_interrupt() {
	static char* TAG = "(ISR)Ultrasonic";
	ESP_EARLY_LOGD(TAG, "ENTERED ISR");
	int level = gpio_get_level(ULTRASONIC_DRIVER_GPIO_ECHO);
	if(level == 1) {  // Measurement just started
		ESP_EARLY_LOGD(TAG, "Measurement Started");
		// set timer value to 0
		timer_start(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID);
	} else { // Measurement just finished
		ESP_EARLY_LOGD(TAG, "Measurement Finished");
		// get timer value and stop timer
		double elapsed_time = 0.0;
		timer_get_counter_time_sec(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID, &elapsed_time);
		timer_pause(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID);
		timer_set_counter_value(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID, 0x0);
		BaseType_t xHigherPriorityTaskWoken;
		xEventGroupSetBitsFromISR(data_available, EVENT_DATA_AVAILABLE, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken) {
			portYIELD_FROM_ISR();
		}
	}
}
