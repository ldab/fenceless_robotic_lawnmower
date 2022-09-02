#include <Arduino.h>

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
  ESP_LOGI(__func__, "Wifi Controller status: %d\n", status);
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

  // wifi_init_sta();

  wifi_init_softap();

  wifi_controller_init("mower", "verysafepass", WIFI_CONTROLLER_AP);

  wifi_controller_register_connection_callback(&handle_wifi_controller_status);
}

void loop()
{

  // WiFiClient client;
  // const int httpPort = 80;
  // const char *host   = "192.168.4.1";
  // if (!client.connect(host, httpPort)) {
  //   ESP_LOGE(__func__, "connection failed");
  // } else {
  //   // We now create a URI for the request
  //   String url = "/input/";

  //   Serial.print("Requesting URL: ");
  //   Serial.println(url);

  //   // This will send the request to the server
  //   client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host +
  //                "\r\n" + "Connection: close\r\n\r\n");
  //   unsigned long timeout = millis();
  //   while (client.available() == 0) {
  //     if (millis() - timeout > 5000) {
  //       Serial.println(">>> Client Timeout !");
  //       client.stop();
  //       return;
  //     }
  //   }

  //   // Read all the lines of the reply from server and print them to Serial
  //   while (client.available()) {
  //     String line = client.readStringUntil('\r');
  //     Serial.print(line);
  //   }
  // }

  // ESP_LOGI(__func__, "TxPower: %d", WiFi.getTxPower());
  // ESP_LOGI(__func__, "RSSI: %ddBm", WiFi.RSSI());

  // delay(5000);
}