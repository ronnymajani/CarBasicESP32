/*
 * servo_driver.c
 *
 *  Created on: Sep 21, 2017
 *      Author: ronnymajani
 */

#include "driver/ledc.h"
#include "esp_log.h"

#include "servo_driver.h"

void servo_driver_init() {
	/* Initialize PWM (LEDC module) */
	// PWM Timer config
	ledc_timer_config_t pwm_timer;
	pwm_timer.bit_num = SERVO_DRIVER_PWM_BITNUM;
	pwm_timer.freq_hz = SERVO_DRIVER_PWM_FREQ;
	pwm_timer.speed_mode = SERVO_DRIVER_PWM_MODE;
	pwm_timer.timer_num = SERVO_DRIVER_PWM_TIMER;
	ledc_timer_config(&pwm_timer);

	// PWM Channels config
	ledc_channel_config_t pwm_channel;
	pwm_channel.channel = SERVO_DRIVER_PWM_CHANNEL;
	pwm_channel.duty = 0;
	pwm_channel.gpio_num = SERVO_DRIVER_GPIO_PWM;
	pwm_channel.intr_type = LEDC_INTR_DISABLE;
	pwm_channel.timer_sel = SERVO_DRIVER_PWM_TIMER;
	pwm_channel.speed_mode = SERVO_DRIVER_PWM_MODE;
	ledc_channel_config(&pwm_channel);
	// set servo to middle
	ledc_set_duty(SERVO_DRIVER_PWM_MODE, SERVO_DRIVER_PWM_CHANNEL, SERVO_DRIVER_ORIENTATION_MID_PWM);
	ledc_update_duty(SERVO_DRIVER_PWM_MODE, SERVO_DRIVER_PWM_CHANNEL);
}


/**
 * Sets the orientation of the servo in the given angle
 * @param orientation The angle (in multiples of 10) to set the servo at. Must be in the range [-90, 90]
 * If the given orientation is out of range, the function will clip it to the MAX or MIN value
 */
void set_servo_orientation(int orientation) {
	// Clip the given orientation if it is larger than the maximum allowed value
	if(orientation > SERVO_DRIVER_ORIENTATION_MAX) {
		orientation = SERVO_DRIVER_ORIENTATION_MAX;
	}
	// Clip the given orientation if it is less than the minimum allowed value
	if(orientation < SERVO_DRIVER_ORIENTATION_MIN) {
		orientation = SERVO_DRIVER_ORIENTATION_MIN;
	}
	int pwm = (orientation/10) * SERVO_DRIVER_ORIENTATION_10_PWM + SERVO_DRIVER_ORIENTATION_MID_PWM;

	ledc_set_duty(SERVO_DRIVER_PWM_MODE, SERVO_DRIVER_PWM_CHANNEL, pwm);
	ledc_update_duty(SERVO_DRIVER_PWM_MODE, SERVO_DRIVER_PWM_CHANNEL);
}
