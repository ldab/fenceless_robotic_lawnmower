#include <Arduino.h>

#include <HTTPClient.h>

#include "wifi_helper.h"
#include "wifi_remote_control.h"

#include "rover_config.h" // TODO

static bool wifi_control_enabled = false;

static uint16_t rc_values[RC_NUM_CHANNELS][RC_FILTER_SAMPLES];

static void handle_controller_disconnected(uint16_t last_sampled_signal)
{
  // Set all RC values to mid position if signal is 0 (controller disconnected)
  if (last_sampled_signal == 0) {
    for (uint8_t channel = 0; channel < RC_NUM_CHANNELS; channel++) {
      for (uint8_t sample = 0; sample < RC_FILTER_SAMPLES; sample++) {
        rc_values[channel][sample] = RC_CENTER;
      }
    }
  }
}

static void handle_wifi_controller_status(WifiControllerStatus status)
{
  ESP_LOGI(__func__, "Wifi Controller status: %s\n",
           status ? "DISCONNECTED" : "CONNECTED");
  if (status != WIFI_CONTROLLER_CONNECTED) {
    handle_controller_disconnected(0);
    wifi_control_enabled = false;
  } else {
    wifi_control_enabled = true;
  }
}

void setup()
{

  Serial.begin(115200);

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("dhcpc", ESP_LOG_DEBUG);
  esp_log_level_set("dhcps", ESP_LOG_DEBUG);
  esp_log_level_set("lwip", ESP_LOG_DEBUG);
  esp_log_level_set("wifi", ESP_LOG_DEBUG);

  wifi_init_sta();

  wifi_controller_init(NULL, NULL, WIFI_CONTROLLER_AP);
  wifi_controller_register_connection_callback(&handle_wifi_controller_status);
}

void loop()
{

  if (wifi_sta_connected) {
    HTTPClient http;

    http.begin("http://httpbin.org/get"); // HTTP

    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n",
                    http.errorToString(httpCode).c_str());
    }

    http.end();
    log_i("RRSI: %ddBm", WiFi.RSSI());
  }
}