#include "wifi_helper.h"
#include <esp_wifi.h>
#include "gpio.h"

#define WIFI_STA_SSID    "SSID"
#define WIFI_STA_PASS    "PASS"
#define WIFI_AP_SSID     "ESP_Gateway"
#define WIFI_AP_PASS     "12345678"
#define WIFI_AP_CHANNEL  11
#define WIFI_AP_MAX_CONN 1

volatile bool wifi_sta_connected = false;
volatile bool wifi_ap_connected  = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {

  case ARDUINO_EVENT_WIFI_AP_START:
    WiFi.softAPsetHostname(WIFI_AP_SSID);
    WiFi.softAPenableIpV6();
    break;
  case ARDUINO_EVENT_WIFI_STA_START:
    WiFi.setHostname(WIFI_AP_SSID);
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    WiFi.enableIpV6();
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    ESP_LOGI(__func__, "STA IPv6: %s", WiFi.localIPv6().toString().c_str());
    break;
  case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
    ESP_LOGI(__func__, "AP IPv6: %s", WiFi.softAPIPv6().toString().c_str());
    break;
  case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
    ESP_LOGI(__func__, "WIFI_AP_STAIPASSIGNED: %d", WiFi.softAPgetStationNum());
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    ESP_LOGI(__func__, "STA IP: %s", WiFi.localIP().toString().c_str());
    set_led(GREEN, SLOW);
    wifi_sta_connected = true;
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    ESP_LOGI(__func__, "WIFI_STA_DISCONNECTED");
    set_led(RED, FAST);
    wifi_sta_connected = false;
    break;
  default:
    break;
  }
}

void wifi_init_softap(const char *ssid, const char *password)
{
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_AP);

  esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G |
                                        WIFI_PROTOCOL_11N);

  if (ssid == NULL) {
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, WIFI_AP_CHANNEL, 0,
                WIFI_AP_MAX_CONN);
  } else {
    WiFi.softAP(ssid, password, WIFI_AP_CHANNEL, false, WIFI_AP_MAX_CONN);
  }
}

void wifi_init_sta(void)
{
  WiFi.setSleep(WIFI_PS_NONE);

  WiFi.onEvent(WiFiEvent);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_MODE_STA);

  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);

  WiFi.begin(WIFI_AP_SSID, WIFI_AP_PASS);
}

void wifi_init_softapsta(void)
{
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_APSTA);

  esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR | WIFI_PROTOCOL_11B |
                                        WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR | WIFI_PROTOCOL_11B |
                                         WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);

  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, WIFI_AP_CHANNEL, true,
              WIFI_AP_MAX_CONN);
}

int8_t wifi_ap_get_sta_rssi(void)
{
  wifi_sta_list_t clients;
  esp_wifi_ap_get_sta_list(&clients);
  return clients.sta->rssi;
}