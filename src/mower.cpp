#include <Arduino.h>

#include <HTTPClient.h>

#include "driving_mode.h"
#include "point_perfect.h"
#include "rover_config.h" // TODO
#include "wifi_helper.h"
#include "wifi_remote_control.h"

#include <ubxlib.h>

static bool wifi_control_enabled    = false;
static uint8_t rc_value_index       = 0;
static RoverMode current_rover_mode = DRIVE_TURN_NORMAL;
static uint16_t rc_values[RC_NUM_CHANNELS][RC_FILTER_SAMPLES];

static uint16_t get_controller_channel_value(uint8_t channel)
{
  uint16_t channel_value = 0;

  if (wifi_control_enabled) {
    channel_value = wifi_controller_get_val(channel);
  }

  if (channel_value == 0) {
    channel_value = RC_CENTER;
  }

  return channel_value;
}

static uint16_t filter_signal(uint16_t *signals)
{
  uint32_t signal = 0;
  for (uint8_t i = 0; i < RC_FILTER_SAMPLES; i++) {
    signal += signals[i];
  }
  signal = signal / RC_FILTER_SAMPLES;

  if (signal <= 800)
    return RC_CENTER; // If we are unlucky in timing when RC controller
                      // disconnect we might get some random low signal
  if (signal > 800 && signal < 1010)
    return RC_LOW;
  if (signal > 1990)
    return RC_HIGH;
  if (signal > 1480 && signal < 1520)
    return RC_CENTER;
  return signal;
  ;
}

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

  while (!wifi_sta_connected)
    vTaskDelay(100);

  pp_init(NULL, NULL, NULL);
}

void loop()
{
  if (wifi_control_enabled) {
    uint16_t motor_signal = RC_CENTER;

    rc_values[RC_STEER_CHANNEL][rc_value_index] =
        get_controller_channel_value(RC_STEER_CHANNEL);
    rc_values[RC_MOTOR_CHANNEL][rc_value_index] =
        get_controller_channel_value(RC_MOTOR_CHANNEL);
    rc_values[RC_HEAD_PITCH_CHANNEL][rc_value_index] =
        get_controller_channel_value(RC_HEAD_PITCH_CHANNEL);
    rc_values[RC_HEAD_YAW_CHANNEL][rc_value_index] =
        get_controller_channel_value(RC_HEAD_YAW_CHANNEL);

    if (current_rover_mode == DRIVE_TURN_NORMAL) {
      motor_signal = filter_signal(rc_values[RC_MOTOR_CHANNEL]);
    } else if (current_rover_mode == DRIVE_TURN_SPIN) {
      motor_signal = filter_signal(rc_values[RC_STEER_CHANNEL]);
    }

    rover_driving_move(motor_signal);
    rover_driving_steer(filter_signal(rc_values[RC_STEER_CHANNEL]));

    rc_value_index = (rc_value_index + 1) % RC_FILTER_SAMPLES;

    vTaskDelay(pdMS_TO_TICKS(20));
  }

  if (wifi_sta_connected) {

    // pp_connect("pp.services.u-blox.com", 8883,
    //            "ebf8071d-f02e-4882-8cf8-54ff397f0e45");
    //  pp_onMessageCb();
    // pp_subscribe("/pp/key/ip", 1);

    // HTTPClient http;

    // http.begin("http://httpbin.org/get"); // HTTP

    // int httpCode = http.GET();

    // // httpCode will be negative on error
    // if (httpCode > 0) {
    //   // HTTP header has been send and Server response header has been
    //   handled Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    // } else {
    //   Serial.printf("[HTTP] GET... failed, error: %s\n",
    //                 http.errorToString(httpCode).c_str());
    // }

    // http.getString();

    // http.end();
    // log_i("RRSI: %ddBm", WiFi.RSSI());
  }
}