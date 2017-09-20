/*
 * motor_driver.h
 *
 *  Created on: Sep 16, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_DRIVERS_INCLUDE_MOTOR_DRIVER_H_
#define MAIN_DRIVERS_INCLUDE_MOTOR_DRIVER_H_

#include "driver/ledc.h"

// Low Level Pin Assignments
#define MOTOR_DRIVER_GPIO_IN1		16
#define MOTOR_DRIVER_GPIO_IN2		17
#define MOTOR_DRIVER_GPIO_PWM1		04
#define MOTOR_DRIVER_PWM1_CHANNEL	LEDC_CHANNEL_0

#define MOTOR_DRIVER_GPIO_IN3		18
#define MOTOR_DRIVER_GPIO_IN4		19
#define MOTOR_DRIVER_GPIO_PWM2		05
#define MOTOR_DRIVER_PWM2_CHANNEL	LEDC_CHANNEL_1

#define MOTOR_DRIVER_PWM_MODE	LEDC_LOW_SPEED_MODE
#define MOTOR_DRIVER_PWM_TIMER	LEDC_TIMER_0
#define MOTOR_DRIVER_PWM_FREQ	5000
#define MOTOR_DRIVER_PWM_BITNUM	LEDC_TIMER_10_BIT
#define MOTOR_DRIVER_MAX_PWM	1023	// (duty range is 0 ~ ((2**bit_num)-1)


// Motor Abstracted Pin Assignments
#define MOTOR_RIGHT_F_GPIO		MOTOR_DRIVER_GPIO_IN3	// Forward
#define MOTOR_RIGHT_R_GPIO		MOTOR_DRIVER_GPIO_IN4	// Reverse
#define MOTOR_RIGHT_PWM_GPIO	MOTOR_DRIVER_GPIO_PWM2
#define MOTOR_RIGHT_PWM_CHANNEL	MOTOR_DRIVER_PWM2_CHANNEL
#define MOTOR_RIGHT_GPIO_PIN_SELECT	((1<<MOTOR_RIGHT_F_GPIO) | (1<<MOTOR_RIGHT_R_GPIO))

#define MOTOR_LEFT_F_GPIO		MOTOR_DRIVER_GPIO_IN1	// Forward
#define MOTOR_LEFT_R_GPIO		MOTOR_DRIVER_GPIO_IN2	// Reverse
#define MOTOR_LEFT_PWM_GPIO		MOTOR_DRIVER_GPIO_PWM1
#define MOTOR_LEFT_PWM_CHANNEL	MOTOR_DRIVER_PWM1_CHANNEL
#define MOTOR_LEFT_GPIO_PIN_SELECT	((1<<MOTOR_LEFT_F_GPIO) | (1<<MOTOR_LEFT_R_GPIO))



/* Exported Functions */
void init_motor_driver();

void set_motor_right_direction_forward();
void set_motor_right_direction_reverse();
int set_motor_right_pwm(int pwm);
void enable_motor_right();
void disable_motor_right();

void set_motor_left_direction_forward();
void set_motor_left_direction_reverse();
int set_motor_left_pwm(int pwm);
void enable_motor_left();
void disable_motor_left();

void set_motors_direction_forward();
void set_motors_direction_reverse();
int set_motors_pwm(int pwm);
void enable_motors();
void disable_motors();



#endif /* MAIN_DRIVERS_INCLUDE_MOTOR_DRIVER_H_ */
