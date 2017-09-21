/*
 * ultrasonic_driver.h
 *
 *  Created on: Sep 20, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_DRIVERS_INCLUDE_ULTRASONIC_DRIVER_H_
#define MAIN_DRIVERS_INCLUDE_ULTRASONIC_DRIVER_H_

#define ULTRASONIC_DRIVER_GPIO_TRIG  22//todo set the correct pin number
#define ULTRASONIC_DRIVER_GPIO_ECHO  23//todo set the correct pin number

#define ULTRASONIC_DRIVER_TIMER_DIVIDER		16
#define ULTRASONIC_DRIVER_TIMER_GROUP		TIMER_GROUP_0
#define ULTRASONIC_DRIVER_TIMER_ID			TIMER_0
#define ULTRASONIC_DRIVER_TIMER_GROUP_OBJ	TIMERG0
#define ULTRASONIC_DRIVER_TIMER_OBJ			ULTRASONIC_DRIVER_TIMER_GROUP_OBJ.hw_timer[ULTRASONIC_DRIVER_TIMER_ID]

/* Exported Functions */
void ultrasonic_driver_init();
void ultrasonic_trigger();
int ultrasonic_measurement_ready();
double ultrasonic_get_measurement();


#endif /* MAIN_DRIVERS_INCLUDE_ULTRASONIC_DRIVER_H_ */
