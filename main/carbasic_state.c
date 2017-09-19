/*
 * carbasic_state.c
 *
 *  Created on: Sep 19, 2017
 *      Author: ronnymajani
 */

#include "carbasic_state.h"


/**
 * Creates a new CarBasic state
 * Returns the pointer to the newly created state
 * Remember to free this state when you're finished with it
 */
carbasic_state_t carbasic_new_state() {
	carbasic_state_t new_state;
	new_state.x = 0.0f;
	new_state.y = 0.0f;
	new_state.orientation = 0.0f;
	new_state.sensor_measurement = 0.0f;
	new_state.sensor_orientation = 0.0f;
	new_state.speed = 0.0f;
	new_state.pwm_left = 0;
	new_state.pwm_right = 0;

	return new_state;
}
