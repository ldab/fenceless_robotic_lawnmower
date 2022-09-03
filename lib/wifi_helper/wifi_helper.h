#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdbool.h>

extern volatile bool wifi_sta_connected;
extern volatile bool wifi_ap_connected;

void wifi_init_sta(void);
void wifi_init_softap(const char *ssid = NULL, const char *password = NULL);
void wifi_init_softapsta(void);
int8_t wifi_ap_get_sta_rssi(void);