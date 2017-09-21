/*
 * servo_driver.h
 *
 *  Created on: Sep 21, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_DRIVERS_INCLUDE_SERVO_DRIVER_H_
#define MAIN_DRIVERS_INCLUDE_SERVO_DRIVER_H_

#define SERVO_DRIVER_GPIO_PWM	15
#define SERVO_DRIVER_ORIENTATION_MIN	-90
#define SERVO_DRIVER_ORIENTATION_MAX	90
#define SERVO_DRIVER_ORIENTATION_MIN_PWM	409	// ~1ms pulse
#define SERVO_DRIVER_ORIENTATION_MID_PWM	614	// 1.5ms pulse
#define SERVO_DRIVER_ORIENTATION_MAX_PWM	819	// ~2ms pulse
#define SERVO_DRIVER_ORIENTATION_10_PWM		23	// PWM Duty Cycle to move 10 degrees

#define SERVO_DRIVER_PWM_CHANNEL	LEDC_CHANNEL_0
#define SERVO_DRIVER_PWM_MODE	LEDC_LOW_SPEED_MODE
#define SERVO_DRIVER_PWM_TIMER	LEDC_TIMER_1
#define SERVO_DRIVER_PWM_FREQ	50	// 20ms period
#define SERVO_DRIVER_PWM_BITNUM	LEDC_TIMER_13_BIT
#define SERVO_DRIVER_MAX_PWM	8191	// (duty range is 0 ~ ((2**bit_num)-1)


/* Exported Functions */
void servo_driver_init();
void set_servo_orientation(int orientation);

#endif /* MAIN_DRIVERS_INCLUDE_SERVO_DRIVER_H_ */
