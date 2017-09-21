/*
 * ultrasonic_driver.c
 *
 *  Created on: Sep 20, 2017
 *      Author: ronnymajani
 */
#include <inttypes.h>
#include "rom/ets_sys.h"
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
DRAM_ATTR static uint64_t ultrasonic_echo_time_diff;

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
	// Start the timer
	timer_start(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID);
	// trigger the ultrasonic sensor
	gpio_set_level(ULTRASONIC_DRIVER_GPIO_TRIG, 0);  // set LOW for 5us
	ets_delay_us(5);
	gpio_set_level(ULTRASONIC_DRIVER_GPIO_TRIG, 1);  // set HIGH for 10us
	ets_delay_us(10);
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
	double time = (double) ultrasonic_echo_time_diff / (TIMER_BASE_CLK / ULTRASONIC_DRIVER_TIMER_OBJ.config.divider);
	// convert value to seconds
	ESP_LOGD(TAG, "Elapsed Time: %.8f", time);
	// convert time into distance
	double distance_cm = (time*1000000.0) / 58.2;
	xEventGroupClearBits(data_available, EVENT_DATA_AVAILABLE);
	return distance_cm;
}






/* Interrupts */
IRAM_ATTR void ultrasonic_measurement_interrupt() {
	static char* TAG = "(ISR)Ultrasonic";
	static uint32_t time_start_high = 0x0;
	static uint32_t time_start_low = 0x0;

	int level = gpio_get_level(ULTRASONIC_DRIVER_GPIO_ECHO);

	if(level == 1) {  // Measurement just started
		// save timer's current value
		time_start_high = ULTRASONIC_DRIVER_TIMER_OBJ.cnt_high;
		time_start_low = ULTRASONIC_DRIVER_TIMER_OBJ.cnt_low;
		ESP_EARLY_LOGD(TAG, "start: %d, %d", time_start_high, time_start_low);
	}
	else { // Measurement just finished
		// stop timer
		timer_pause(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID);
		ESP_EARLY_LOGD(TAG, "Measurement Finished");
		// get timer's current value, calculate the difference, and store it
		uint64_t time_start = (uint64_t)time_start_high<<32 | (uint64_t)time_start_low;
		uint64_t time_end = (uint64_t)ULTRASONIC_DRIVER_TIMER_OBJ.cnt_high<<32 | (uint64_t)ULTRASONIC_DRIVER_TIMER_OBJ.cnt_low;
		ultrasonic_echo_time_diff = time_end - time_start;
		ESP_EARLY_LOGD(TAG, "Timer Start: %"PRIu64, time_start);
		ESP_EARLY_LOGD(TAG, "Timer End: %"PRIu64, time_end);
		ESP_EARLY_LOGD(TAG, "Timer Diff: %"PRIu64, ultrasonic_echo_time_diff);
		// reset the timer's value to 0
		timer_set_counter_value(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID, 0x0);
		BaseType_t xHigherPriorityTaskWoken;
		xEventGroupSetBitsFromISR(data_available, EVENT_DATA_AVAILABLE, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken) {
			portYIELD_FROM_ISR();
		}
	}
}
