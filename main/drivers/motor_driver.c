/*
 * motor_driver.c
 *
 *  Created on: Sep 16, 2017
 *      Author: ronnymajani
 */

/**
 * Right Side
 * Left Side
 *
 * Forward(orientation)
 * Reverse(orientation)
 * Rotate in place
 * Break/Stop
 */
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "motor_driver.h"

#include "esp_log.h"
static const char* TAG = "MOTOR_DRIVER";


/**
 * Initializes the pins needed for the Motor Driver and performs the necessary configuration
 */
void motor_driver_init() {
	/* Initialize GPIO Pins */
	gpio_config_t motor_pin_config;
	motor_pin_config.intr_type = GPIO_PIN_INTR_DISABLE;
	motor_pin_config.mode = GPIO_MODE_OUTPUT;
	motor_pin_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
	motor_pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
	motor_pin_config.pin_bit_mask = (MOTOR_RIGHT_GPIO_PIN_SELECT | MOTOR_LEFT_GPIO_PIN_SELECT);
	gpio_config(&motor_pin_config);

	/* Initialize PWM (LEDC module) */
	// PWM Timer config
	ledc_timer_config_t pwm_timer;
	pwm_timer.bit_num = MOTOR_DRIVER_PWM_BITNUM;
	pwm_timer.freq_hz = MOTOR_DRIVER_PWM_FREQ;
	pwm_timer.speed_mode = MOTOR_DRIVER_PWM_MODE;
	pwm_timer.timer_num = MOTOR_DRIVER_PWM_TIMER;
	ledc_timer_config(&pwm_timer);

	// PWM Channels config
	ledc_channel_config_t pwm_channel;
		pwm_channel.duty = 0;
		pwm_channel.intr_type = LEDC_INTR_DISABLE;
		pwm_channel.timer_sel = MOTOR_DRIVER_PWM_TIMER;
		pwm_channel.speed_mode = MOTOR_DRIVER_PWM_MODE;
	// Right Channel
		pwm_channel.channel = MOTOR_RIGHT_PWM_CHANNEL;
		pwm_channel.gpio_num = MOTOR_RIGHT_PWM_GPIO;
	ledc_channel_config(&pwm_channel);
	// Left Channel
		pwm_channel.channel = MOTOR_LEFT_PWM_CHANNEL;
		pwm_channel.gpio_num = MOTOR_LEFT_PWM_GPIO;
	ledc_channel_config(&pwm_channel);
}



/**
 * Sets/Unsets the GPIO levels of the two given pins respectively
 * @param motor_pin_X	the number of the pin to set to HIGH
 * @param motor_pin_Y	the number of the pin to set to LOW
 */
static void set_pin_x_clear_pin_y(int motor_pin_X, int motor_pin_Y) {
	gpio_set_level(motor_pin_X, 1);
	gpio_set_level(motor_pin_Y, 0);
}


/**
 * Sets the PWM Duty Cycle for the given Channel if the given value is within the legal range.
 * Returns 1 if the PWM was changed, 0 if the given value is invalid
 */
static int set_pwm(ledc_channel_t channel, int pwm) {
	if(pwm >= 0 && pwm <= MOTOR_DRIVER_MAX_PWM) {
//		ESP_LOGD(TAG, ">> setting pwm %d", pwm);
		ledc_set_duty(MOTOR_DRIVER_PWM_MODE, channel, pwm);
		ledc_update_duty(MOTOR_DRIVER_PWM_MODE, channel);
		return 1;
	}
	return 0;
}


/* Right Motors */
void set_motor_right_direction_forward() {
	// ESP_LOGD(TAG, ">> setting motor right forward");
	set_pin_x_clear_pin_y(MOTOR_RIGHT_F_GPIO, MOTOR_RIGHT_R_GPIO);
}

void set_motor_right_direction_reverse() {
	// ESP_LOGD(TAG, ">> setting motor right reverse");
	set_pin_x_clear_pin_y(MOTOR_RIGHT_R_GPIO, MOTOR_RIGHT_F_GPIO);
}

int set_motor_right_pwm(int pwm) {
//	 ESP_LOGD(TAG, ">> setting motor right pwm %d", pwm);
	return set_pwm(MOTOR_RIGHT_PWM_CHANNEL, pwm);
}

void disable_motor_right() {
	// ESP_LOGD(TAG, ">> disable motor right");
	gpio_set_level(MOTOR_RIGHT_F_GPIO, 0);
	gpio_set_level(MOTOR_RIGHT_R_GPIO, 0);
}


/* Left Motors */
void set_motor_left_direction_forward() {
	// ESP_LOGD(TAG, ">> setting motor left forward");
	set_pin_x_clear_pin_y(MOTOR_LEFT_F_GPIO, MOTOR_LEFT_R_GPIO);
}

void set_motor_left_direction_reverse() {
	// ESP_LOGD(TAG, ">> setting motor left reverse");
	set_pin_x_clear_pin_y(MOTOR_LEFT_R_GPIO, MOTOR_LEFT_F_GPIO);
}

int set_motor_left_pwm(int pwm) {
//	 ESP_LOGD(TAG, ">> setting motor left pwm %d", pwm);
	return set_pwm(MOTOR_LEFT_PWM_CHANNEL, pwm);
}

void disable_motor_left() {
	// ESP_LOGD(TAG, ">> disable motor left");
	gpio_set_level(MOTOR_LEFT_F_GPIO, 0);
	gpio_set_level(MOTOR_LEFT_R_GPIO, 0);
}


/* All Motors */
void set_motors_direction_forward() {
	set_motor_right_direction_forward();
	set_motor_left_direction_forward();
}

void set_motors_direction_reverse() {
	set_motor_right_direction_reverse();
	set_motor_left_direction_reverse();
}

int set_motors_pwm(int pwm) {
	return set_motor_left_pwm(pwm) && set_motor_right_pwm(pwm);
}

void enable_motors() {
	enable_motor_right();
	enable_motor_left();
}

void disable_motors() {
	disable_motor_right();
	disable_motor_left();
}
