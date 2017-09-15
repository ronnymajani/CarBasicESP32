/*
 * wifi.c
 *
 *  Created on: Sep 15, 2017
 *      Author: ronnymajani
 */

#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "wifi.h"


static const char* TAG = "WIFI_SETUP";

static esp_err_t event_handler(void* ctx, system_event_t* event) {
	switch(event->event_id) {
		case SYSTEM_EVENT_AP_STACONNECTED:
	        ESP_LOGI(TAG, "station:"MACSTR" join,AID=%d\n",
	                 MAC2STR(event->event_info.sta_connected.mac),
	                 event->event_info.sta_connected.aid);
	        break;
	    case SYSTEM_EVENT_AP_STADISCONNECTED:
	        ESP_LOGI(TAG, "station:"MACSTR"leave,AID=%d\n",
	                 MAC2STR(event->event_info.sta_disconnected.mac),
	                 event->event_info.sta_disconnected.aid);
	        break;
	    case SYSTEM_EVENT_STA_CONNECTED:
	    	ESP_LOGI(TAG, "connected to desired AP");
	    	break;
	    case SYSTEM_EVENT_STA_DISCONNECTED:
	    	ESP_LOGI(TAG, "disconnected from desired AP");
	    	break;
	    case SYSTEM_EVENT_STA_GOT_IP: {
	    	ip4_addr_t ip = event->event_info.got_ip.ip_info.ip;
	    	ESP_LOGI(TAG, "got IP: "IPSTR, IP2STR(&ip));
	    	break;
	    }
	    default:
	        break;
	}

	return ESP_OK;
}



void wifi_init_sta() {
	tcpip_adapter_init();
	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	esp_wifi_set_country(WIFI_COUNTRY_EU);
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	wifi_config_t sta_config = {
		.sta = {
			.ssid = WIFI_TEST_STA_SSID,
			.password = WIFI_TEST_STA_PASS,
			.bssid_set = false
		}
	};
	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
	ESP_ERROR_CHECK( esp_wifi_start() );
	ESP_ERROR_CHECK( esp_wifi_connect() );
}



void wifi_init_softap()
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = 0,
			.ssid_hidden = 0,
            .max_connection = WIFI_MAX_STA_CONN,
            .password = WIFI_PASS,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
			.beacon_interval = 100
        },
    };
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_country(WIFI_COUNTRY_EU);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s \n",
    		WIFI_SSID, WIFI_PASS);
}
