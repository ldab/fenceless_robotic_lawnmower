#include "wifi_remote_control.h"
#include "WebServer.h"
#include "WiFi.h"
#include "WiFiUdp.h"
#include "assert.h"
#include "esp_log.h"
#include "rover_config.h"
#include "wifi_helper.h"
#include <esp_wifi.h>

#include "ESPAsyncWebServer.h"

#include "index.h"
#include "virtualjoystick.h"

#define MAX_REGISTRATED_CALLBACKS 2

#define UDP_PORT                  8080

static const char *TAG            = "wifi_remote_control";

static const char *broadcast_addr = "192.168.4.255";

static void handle_not_found(AsyncWebServerRequest *request);
static void handle_websocket_event(AsyncWebSocket *server,
                                   AsyncWebSocketClient *client,
                                   AwsEventType type, void *arg,
                                   uint8_t *payload, size_t length);
static void wifiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

static uint16_t channel_values[RC_NUM_CHANNELS] = {0};

static const char *wifi_ssid;
static const char *wifi_password;

static IPAddress local_ip(192, 168, 4, 5);
static IPAddress gateway(192, 168, 4, 1);
static IPAddress subnet(255, 255, 255, 0);

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws

static WiFiUDP udp;

static WifiControllerStatusCb *status_callbacks[MAX_REGISTRATED_CALLBACKS];
static uint8_t num_callbacks;
static bool is_initialized = false;

static bool wifi_connected = false;

String processor(const String &var)
{
  log_d("processor %s", var.c_str());
  if (var == "VIRTUALJOYSTICK")
    log_d("VIRTUALJOYSTICK %s", var.c_str());
    return FPSTR(VIRTUALJOYSTICK);
  // if (var == "INDEX_JS")
  //   return FPSTR(HTTP_JS);
  // if (var == "HTML_HEAD_TITLE")
  //   return FPSTR(HTML_HEAD_TITLE);
  // if (var == "HTML_INFO_BOX") {
  //   String ret = "";
  //   if (WiFi.isConnected()) {
  //     ret = "<strong> Connected</ strong> to ";
  //     ret += WiFi.SSID();
  //     ret += "<br><em><small> with IP ";
  //     ret += WiFi.localIP().toString();
  //     ret += "</small>";
  //   } else
  //     ret = "<strong> Not Connected</ strong>";
  //   return ret;
  // }
  // if (var == "UPTIME") {
  //   String ret = String(millis() / 1000 / 60);
  //   ret += " min ";
  //   ret += String((millis() / 1000) % 60);
  //   ret += " sec";
  //   return ret;
  // }
  // if (var == "CHIP_ID") {
  //   String ret = String((uint32_t)ESP.getEfuseMac());
  //   return ret;
  // }
  // if (var == "FREE_HEAP") {
  //   String ret = String(ESP.getFreeHeap());
  //   ret += " bytes";
  //   return ret;
  // }
  // if (var == "SKETCH_INFO") {
  //   //%USED_BYTES% / &FLASH_SIZE&<br><progress value="%USED_BYTES%"
  //   // max="&FLASH_SIZE&">
  //   String ret = String(ESP.getSketchSize());
  //   ret += " / ";
  //   ret += String(ESP.getFlashChipSize());
  //   ret += "<br><progress value=\"";
  //   ret += String(ESP.getSketchSize());
  //   ret += "\" max=\"";
  //   ret += String(ESP.getFlashChipSize());
  //   ret += "\">";
  //   return ret;
  // }
  // if (var == "HOSTNAME")
  //   return String(WiFi.getHostname());
  // if (var == "MY_MAC")
  //   return WiFi.macAddress();
  // if (var == "MY_RSSI")
  //   return String(WiFi.RSSI());
  // if (var == "FW_VER")
  //   return String(FIRMWARE_VERSION);
  // if (var == "SDK_VER")
  //   return String(ESP_ARDUINO_VERSION_MAJOR) + "." +
  //          String(ESP_ARDUINO_VERSION_MINOR) + "." +
  //          String(ESP_ARDUINO_VERSION_PATCH);
  // if (var == "ABOUT_DATE") {
  //   String ret = String(__DATE__) + " " + String(__TIME__);
  //   return ret;
  // }

  return String();
}

void wifi_controller_init(const char *ssid, const char *password,
                          WifiControllerMode mode)
{
  memset(channel_values, 0, sizeof(channel_values));
  num_callbacks = 0;

  wifi_ssid     = ssid;
  password      = password;

  // switch (mode) {
  // case WIFI_CONTROLLER_STATION:
  //   WiFi.begin(wifi_ssid, wifi_password);
  //   WiFi.config(local_ip, gateway, subnet);
  //   WiFi.setAutoReconnect(true);
  //   WiFi.onEvent(wifiEvent);
  //   break;
  // case WIFI_CONTROLLER_AP:
  //   ESP_LOGI(__func__, "Mode: ROVER_SWITCH_STATE_AP\n");

  //   WiFi.mode(WIFI_MODE_AP);

  //   esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR | WIFI_PROTOCOL_11B |
  //                                         WIFI_PROTOCOL_11G |
  //                                         WIFI_PROTOCOL_11N);
  //   WiFi.softAP(ssid, password);
  //   WiFi.softAPConfig(local_ip, gateway, subnet);

  //   ESP_LOGI(__func__, "Start AP: %s", WiFi.softAPSSID().c_str());

  //   break;
  // default:
  //   assert(false);
  //   break;
  // }

  ws.onEvent(handle_websocket_event);
  server.addHandler(&ws);

  // server.on("/", []() { server.send_P(200, "text/html", HTTP_INDEX); });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", HTTP_INDEX, processor);
  });

  server.onNotFound(handle_not_found);
  server.begin();

  udp.begin(UDP_PORT);

  is_initialized = true;
}

void wifi_controller_register_connection_callback(WifiControllerStatusCb *cb)
{
  assert(num_callbacks <= MAX_REGISTRATED_CALLBACKS);
  assert(is_initialized);
  status_callbacks[num_callbacks] = cb;
  num_callbacks++;
}

void wifi_controller_ws_send_bin(uint8_t *data, uint32_t length)
{
  ws.binaryAll(data, length);
}

void wifi_controller_udp_send_bin(uint8_t *data, uint32_t length)
{
  if (wifi_connected) {
    uint16_t udp_buf_size = length + 2;
    uint8_t udp_buf[udp_buf_size];

    // Encapsulate data in start and end tag to validate correct-ish packet on
    // other side.
    udp_buf[0] = '[';
    memcpy(&udp_buf[1], data, length);
    udp_buf[udp_buf_size - 1] = ']';
    udp.beginPacket(broadcast_addr, UDP_PORT);
    udp.write((uint8_t *)udp_buf, udp_buf_size);
    udp.endPacket();
  }
}

uint16_t wifi_controller_get_val(uint8_t channel)
{
  assert(channel < RC_NUM_CHANNELS);
  return channel_values[channel];
}

static void handle_not_found(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", HTTP_INDEX, processor);
}

static void reset_ch_values()
{
  for (uint8_t i = 0; i < RC_NUM_CHANNELS; i++) {
    channel_values[i] = config_default_ch_values[i];
  }
}

static void wifiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    wifi_connected = false;
    if (info.wifi_sta_disconnected.reason == 6) {
      Serial.println("NOT_AUTHED reconnect");
      WiFi.reconnect();
    }
  } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
    wifi_connected = true;
  }
}

static void handle_websocket_event(AsyncWebSocket *server,
                                   AsyncWebSocketClient *client,
                                   AwsEventType type, void *arg,
                                   uint8_t *payload, size_t length)
{
  switch (type) {
  case WS_EVT_DISCONNECT:
    printf("Disconnect\n");
    reset_ch_values();
    for (uint8_t i = 0; i < num_callbacks; i++) {
      status_callbacks[i](WIFI_CONTROLLER_DISCONNECTED);
    }
    break;
  case WS_EVT_CONNECT:
    printf("Connected\n");
    reset_ch_values();
    for (uint8_t i = 0; i < num_callbacks; i++) {
      status_callbacks[i](WIFI_CONTROLLER_CONNECTED);
    }
    break;
  case WS_EVT_DATA: {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (!(info->final && info->index == 0 && info->len == length)) {
      break;
    }
    if (info->opcode == WS_BINARY) {
      if (length >= RC_NUM_CHANNELS * sizeof(uint16_t)) {
        uint16_t *values = (uint16_t *)payload;
        for (uint8_t i = 0; i < RC_NUM_CHANNELS; i++) {
          if (values[i] >= 1000 && values[i] <= 2000) {
            channel_values[i] = values[i];
          } else {
            log_e("Expected channel values to be in range 1000 - 2000 but was: "
                  "%d\n",
                  values[i]);
            channel_values[i] = config_default_ch_values[i];
          }
        }
      } else {
        ESP_LOGI(TAG, "Invalid binary length");
        reset_ch_values();
      }
      // printf("%d, %d \t %d, %d \t %d, %d\n", channel_values[0],
      // channel_values[1], channel_values[2],  channel_values[3],
      // channel_values[4], channel_values[5]);
    } else {
      printf("Data: %.*s\n", length, payload);
    }
    break;
  }
  default:
    break;
  }
}
