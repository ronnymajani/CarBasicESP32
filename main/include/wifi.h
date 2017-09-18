/*
 * wifi.h
 *
 *  Created on: Sep 15, 2017
 *      Author: ronnymajani
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#define WIFI_SSID "CarBasicESP32"
#define WIFI_PASS "CarBasic2018"
#define WIFI_MAX_STA_CONN 2

#define WIFI_TEST_STA_SSID "Majani Family"
#define WIFI_TEST_STA_PASS "ramarajab"

void wifi_init_softap();
void wifi_init_sta();

#endif /* MAIN_WIFI_H_ */
