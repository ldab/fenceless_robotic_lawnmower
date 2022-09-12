#include <Arduino.h>

#include <HTTPClient.h>

#include "driving_mode.h"
#include "gpio.h"
#include "point_perfect.h"
#include "rover_config.h" // TODO
#include "wifi_helper.h"
#include "wifi_remote_control.h"

#define U_CFG_APP_GNSS_UART 1

// Bring in the application settings
#include "u_cfg_app_platform_specific.h"

#include <ubxlib.h>

#define ZED_RX   2
#define ZED_TX   46
#define ZED_BAUD 38400

static const uDeviceCfg_t gDeviceCfg = {
    .version    = 0,
    .deviceType = U_DEVICE_TYPE_GNSS,
    .deviceCfg =
        {
            .cfgGnss = {.moduleType     = U_GNSS_MODULE_TYPE_M9,
                        .pinEnablePower = U_CFG_APP_PIN_GNSS_ENABLE_POWER,
                        .pinDataReady   = -1},
        },
    .transportType = U_DEVICE_TRANSPORT_TYPE_UART,
    .transportCfg =
        {
            .cfgUart = {.uart     = U_CFG_APP_GNSS_UART,
                        .baudRate = ZED_BAUD,
                        .pinTxd   = ZED_TX,
                        .pinRxd   = ZED_RX,
                        .pinCts   = U_CFG_APP_PIN_GNSS_CTS,
                        .pinRts   = U_CFG_APP_PIN_GNSS_RTS},
        },
};
// NETWORK configuration for GNSS
static const uNetworkCfgGnss_t gNetworkCfg = {.type = U_NETWORK_TYPE_GNSS,
                                              .moduleType =
                                                  U_GNSS_MODULE_TYPE_M9,
                                              .devicePinPwr       = -1,
                                              .devicePinDataReady = -1};

static uDeviceHandle_t devHandle           = NULL;

static bool wifi_control_enabled           = false;
static uint8_t rc_value_index              = 0;
static RoverMode current_rover_mode        = DRIVE_TURN_NORMAL;
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

static void handle_corrections_sub(void *handler_args, esp_event_base_t base,
                                   int32_t event_id, void *event_data)
{
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  if ((esp_mqtt_event_id_t)event_id == MQTT_EVENT_DATA &&
      strcmp(event->topic, "/pp/Lb/eu")) {
    log_d("/pp/Lb/eu");
  }
}

static void handle_key_sub(void *handler_args, esp_event_base_t base,
                           int32_t event_id, void *event_data)
{
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  if ((esp_mqtt_event_id_t)event_id == MQTT_EVENT_DATA &&
      strcmp(event->topic, "/pp/ubx/0236/Lb")) {
    log_d("/pp/ubx/0236/Lb");
  }
}

static char latLongToBits(int32_t thingX1e7, int32_t *pWhole,
                          int32_t *pFraction)
{
  char prefix = '+';

  // Deal with the sign
  if (thingX1e7 < 0) {
    thingX1e7 = -thingX1e7;
    prefix    = '-';
  }
  *pWhole    = thingX1e7 / 10000000;
  *pFraction = thingX1e7 % 10000000;

  return prefix;
}

void setup()
{
  set_led(RED, FAST);

  Serial.begin(115200);

  gpios_init();

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("dhcpc", ESP_LOG_DEBUG);
  esp_log_level_set("dhcps", ESP_LOG_DEBUG);
  esp_log_level_set("lwip", ESP_LOG_DEBUG);
  esp_log_level_set("wifi", ESP_LOG_DEBUG);

  wifi_init_sta();

  wifi_controller_init(NULL, NULL, WIFI_CONTROLLER_AP);
  wifi_controller_register_connection_callback(&handle_wifi_controller_status);

  while (!wifi_sta_connected && millis() < 20000)
    vTaskDelay(100);

  pp_init(NULL, NULL, NULL);
  pp_subscribe("/pp/Lb/eu", &handle_corrections_sub);
  pp_subscribe("/pp/ubx/0236/Lb", &handle_corrections_sub);

  uLocation_t location;
  int32_t whole    = 0;
  int32_t fraction = 0;
  int32_t returnCode;

  // Set an out of range value so that we can test it later
  location.timeUtc = -1;

  // Initialise the APIs we will need
  uPortInit();
  uDeviceInit();

  // Open the device
  returnCode = uDeviceOpen(&gDeviceCfg, &devHandle);
  uPortLog("Opened device with return code %d.\n", returnCode);

  // You may configure GNSS as required here
  // here using any of the GNSS API calls.

  // Bring up the GNSS network interface
  uPortLog("Bringing up the network...\n");
  if (uNetworkInterfaceUp(devHandle, U_NETWORK_TYPE_GNSS, &gNetworkCfg) == 0) {

    // Get location
    if (uLocationGet(devHandle, U_LOCATION_TYPE_GNSS, NULL, NULL, &location,
                     NULL) == 0) {
      uPortLog("I am here: https://maps.google.com/?q=%c%d.%07d/%c%d.%07d\n",
               latLongToBits(location.latitudeX1e7, &whole, &fraction), whole,
               fraction,
               latLongToBits(location.longitudeX1e7, &whole, &fraction), whole,
               fraction);
    } else {
      uPortLog("Unable to get a location fix!\n");
    }

    // When finished with the GNSS network layer
    uPortLog("Taking down GNSS...\n");
    uNetworkInterfaceDown(devHandle, U_NETWORK_TYPE_GNSS);
  } else {
    uPortLog("Unable to bring up GNSS!\n");
  }
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

    set_led(GREEN, SLOW);

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