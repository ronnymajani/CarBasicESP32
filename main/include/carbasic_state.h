/*
 * carbasic_state.h
 *
 *  Created on: Sep 19, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_INCLUDE_CARBASIC_STATE_H_
#define MAIN_INCLUDE_CARBASIC_STATE_H_

typedef struct {
	float x;
	float y;
	float orientation;
	int sensor_measurement;
	float sensor_orientation;
	float speed;
	int pwm_left;
	int pwm_right;
} carbasic_state_t;



/* Exported Functions */
carbasic_state_t carbasic_new_state();

#endif /* MAIN_INCLUDE_CARBASIC_STATE_H_ */
