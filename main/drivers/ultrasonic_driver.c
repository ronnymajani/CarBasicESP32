/*
 * ultrasonic_driver.c
 *
 *  Created on: Sep 20, 2017
 *      Author: ronnymajani
 */
#include "driver/gpio.h"
#include "driver/timer.h"
#include "soc/soc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "ultrasonic_driver.h"


/* Global Variables */
static QueueHandle_t measurement_raw_data;

/* Function Prototypes */
IRAM_ATTR void ultrasonic_measurement_interrupt();



void init_ultrasonic_driver() {
	// setup GPIO
	gpio_config_t ultrasonic_pin_config;
		ultrasonic_pin_config.mode = GPIO_MODE_OUTPUT;
		ultrasonic_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
		ultrasonic_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
		ultrasonic_pin_config.pin_bit_mask = ULTRASONIC_DRIVER_GPIO_TRIG;
		ultrasonic_pin_config.intr_type = GPIO_PIN_INTR_DISABLE;
	gpio_config(&ultrasonic_pin_config);
		ultrasonic_pin_config.mode = GPIO_MODE_INPUT;
		ultrasonic_pin_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
		ultrasonic_pin_config.intr_type = GPIO_PIN_INTR_ANYEDGE;
		ultrasonic_pin_config.pin_bit_mask = ULTRASONIC_DRIVER_GPIO_ECHO;
	gpio_config(&ultrasonic_pin_config);
	// setup Timer
	timer_config_t config;
	    config.alarm_en = 0;
	    config.auto_reload = 1;
	    config.counter_dir = TIMER_COUNT_UP;
	    config.divider = ULTRASONIC_DRIVER_TIMER_DIVIDER;
	    config.counter_en = TIMER_PAUSE;
	timer_init(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID, &config);
	timer_pause(ULTRASONIC_DRIVER_TIMER_GROUP, ULTRASONIC_DRIVER_TIMER_ID);
	// Register Interrupts
	gpio_install_isr_service(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_EDGE);
	gpio_isr_handler_add(ULTRASONIC_DRIVER_GPIO_ECHO, &ultrasonic_measurement_interrupt, NULL);
	// Initialize Queue
	measurement_raw_data = xQueueCreate(1, sizeof(uint64_t));

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
	uint64_t val;
	return xQueuePeek(measurement_raw_data, &val, 0);
}

/**
 * Returns the measured distance in cm
 * If data is available
 * If no data is available, this function will block until a new measurement is available
 * Please use @ref ultrasonic_measurement_ready function to check for data availability before you call this function
 */
double ultrasonic_get_measurement() {
	uint64_t timer_val;
	xQueueReceive(measurement_raw_data, &timer_val, portMAX_DELAY);
	// convert value to seconds
	double time = (double) timer_val / (TIMER_BASE_CLK / ULTRASONIC_DRIVER_TIMER_OBJ.config.divider);
	// convert time into distance
	double distance_cm = (time/1000000) / 58.2;
	return distance_cm;
}






/* Interrupts */
IRAM_ATTR void ultrasonic_measurement_interrupt() {
	static char* TAG = "(ISR)Ultrasonic";
	int level = gpio_get_level(ULTRASONIC_DRIVER_GPIO_ECHO);
	if(level == 1) {  // Measurement just started
		ESP_EARLY_LOGV(TAG, "Measurement Started");
		// set timer value to 0
		ULTRASONIC_DRIVER_TIMER_OBJ.cnt_high = 0x0;
		ULTRASONIC_DRIVER_TIMER_OBJ.cnt_low = 0x0;
		ULTRASONIC_DRIVER_TIMER_OBJ.config.enable = 1;
	} else { // Measurement just finished
		ESP_EARLY_LOGV(TAG, "Measurement Finished");
		// get timer value and stop timer
		uint64_t elapsed_time = (((uint64_t)ULTRASONIC_DRIVER_TIMER_OBJ.cnt_high << 32)
				| ((uint64_t)ULTRASONIC_DRIVER_TIMER_OBJ.cnt_low));
		ULTRASONIC_DRIVER_TIMER_OBJ.config.enable = 0;
		// send the time difference to the Queue to be processed by the application
		BaseType_t xHigherPriorityTaskWoken;
		xQueueSendFromISR(measurement_raw_data, &elapsed_time, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken) {
			portYIELD_FROM_ISR();
		}
	}
}
