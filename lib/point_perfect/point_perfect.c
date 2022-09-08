#include "point_perfect.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "Arduino.h"

// HTTPS url for ZTP API request
#define THINGSTREAM_SERVER  "api.thingstream.io"
#define THINGSTREAM_ZTPPATH "/ztp/pointperfect/credentials"
const char THINGSTREAM_ZTPURL[] =
    "https://" THINGSTREAM_SERVER THINGSTREAM_ZTPPATH;

// HTTPS url for Amazon root CA (not really needed, as long as we dont verify
// the certificates)
#define AWSTRUST_SERVER     "www.amazontrust.com"
#define AWSTRUST_ROOTCAPATH "/repository/AmazonRootCA1.pem"
const char AWSTRUST_ROOTCAURL[] =
    "https://" AWSTRUST_SERVER AWSTRUST_ROOTCAPATH;

const unsigned short MQTT_BROKER_PORT = 8883;
const int MQTT_MAX_MSG_SIZE           = 9 * 1024;

#define PP_BROKER_URL        "pp.services.u-blox.com"
#define KEY_TOPIC            "/pp/key/ip"
#define KEY_TOPIC_UBX        "/pp/ubx/0236/ip"
#define ASSIST_NOW_TOPIC     "/pp/ubx/mga"
#define CORRECTIONS_EU_TOPIC "/pp/Lp/eu"

const char CONFIG_VALUE_ZTPTOKEN[]   = "ztpToken";
const char CONFIG_VALUE_BROKERHOST[] = "brokerHost";
const char CONFIG_VALUE_STREAM[]     = "stream";
const char CONFIG_VALUE_ROOTCA[]     = "rootCa"; // not using the root CA
const char CONFIG_VALUE_CLIENTCERT[] = "clientCert";
const char CONFIG_VALUE_CLIENTKEY[]  = "clientKey";
const char CONFIG_VALUE_CLIENTID[]   = "clientId";

char clientId[]                      = "ebf8071d-f02e-4882-8cf8-54ff397f0e45";

const char privateKey[] =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEpQIBAAKCAQEAnxKGVp6At36hOZZnYP5cdCvsaG9dAC97Nc5ljO7qun5m/Y21\n"
    "KHFD7OXl7xn4srYIJSDaDK6/8ntFFK9AyuMf0fHVZDx3NVNoDL2XKPUDQQsaAaBi\n"
    "BeEZOzN6CAPTIZkPx7vknCd6Sd+FHOVKCoX1hlaJog9/bPdIo/1ww8Ki8myjKr12\n"
    "Vpp1skjaTsnyW4nv8N5MtSWY2XoWE13a0DxH+tW7XpkuFXkgi8+r5tbqV7Y2hncV\n"
    "aIQxBtiu9J6j4FY3yV2M0vUf8SXdQMdRrRLoLouRQUtXPVTtE7Ba4CQhaqYTQbhG\n"
    "490Dhm9fM2vjj9n3L46pTDaKnAkbLslPcuH9vQIDAQABAoIBAHzpaBJGvx7YUjRN\n"
    "dfqFYwZao3bS1D3cCy2SWM4Vor81mRxMFdCfOM6jLS7XvyJerQCyzmcdJl98CyAW\n"
    "CpQHTCppc78VeCqox1ER48xmsdp2pEJYtgCV5WFGp/H5RubgMbMPSZKRsYam/flV\n"
    "2BFxZf7Kn6Nh9hcUC7eywcUHZztjdKs6ft5E+bMmzP967yuOXW8GK57PuojDKrBL\n"
    "rB0dvZ1tojr/IRJP87kMq/H+iAHlDBvWLqKG8GDU3nLrTvUmINm6bcs0WQo2Pg0b\n"
    "YOMThj9Uyzxtfv82+AoT+dOJGMg/ro7awiTNrzDf04b8xQJzBxg9NCNDxJuQ1uvb\n"
    "2ZsXG6UCgYEAzs9BMiWRBw+a1YFhO/eiHguS5l2MYsvj7yiMpkUcZfhyGGAqzaJi\n"
    "X6Eb6HqqFdYjT3j6oNTbi+++RnRzOhss12oSbbJTZ7I2YjfJM3MAlbZswmpX78Gr\n"
    "cNpASy4ZlGt8Xi4lSAR/oCQVsxBfnsd4GmyHu4QxSq/FTUEBgOjwEI8CgYEAxOiG\n"
    "bb6JKUVpTlQK9LATnCosNjWi3WVhzbzvowq61ApNZIWJDevZNF5monQd78TboklX\n"
    "nL3vCexmuYugf1J5rdCt+y3QNGe/2VWAoz3Ghl5MO1dY/D2JTeWoc752eYzF75A3\n"
    "pRS4uV1e6x/Ran93dW0hvT5J2RRbT6yDDDPQWvMCgYEAo4fC99I4waqXNmid6HtB\n"
    "2BwyxrIGRkEPNZeM0BUqBX/VSG0Sq5PR0ehlVwlX4Ph452i0VeS6zgZ/INsIGlRO\n"
    "uu+HLvKpfP+wfS27jVFMjxW3HmxUqKMt24jY+hmz5Gax6w72L+JOwOSHvBYgZj84\n"
    "OzzCf4ZiK7qdex8B/1syKQUCgYEAhIrIQgLpLMCuO61SmpYS/SFeFTdRY5mklepB\n"
    "nAvS6nvs0GcadCZ3VEre5ycmJ4jpFnor4TPwPUvFccvMwydvBFLdaZ/S8BOODesB\n"
    "5d/lS/kZczA+k7uQOsEF/LMN6I+bhMAc1J/4H7378GTAf+FBNee5uDq/VsHfHOhQ\n"
    "ZdxSuYUCgYEAmewCEPma4Zlx/+KBkEhgKEDu+A9pLNxi8lO9foT7i1W2Aq3Df+FQ\n"
    "0VFvtM1IGKirkDZkLioCbh8bXaO4XFIqIjP2FlkH9hzRgGaxNtX889bOLXVSoc+O\n"
    "u0PjMQ/0rYAPWmgHSTpfTXTGowS//pfWRP7g2lgnE9zKsHmx3/rKxH8=\n"
    "-----END RSA PRIVATE KEY-----\n";

const char clientCert[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDWTCCAkGgAwIBAgIUUbzDbsFbCwj8JM+Ai7AnX0bSYUgwDQYJKoZIhvcNAQEL\n"
    "BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
    "SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIyMDkwNzA5MzA1\n"
    "OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
    "ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJ8ShlaegLd+oTmWZ2D+\n"
    "XHQr7GhvXQAvezXOZYzu6rp+Zv2NtShxQ+zl5e8Z+LK2CCUg2gyuv/J7RRSvQMrj\n"
    "H9Hx1WQ8dzVTaAy9lyj1A0ELGgGgYgXhGTszeggD0yGZD8e75JwneknfhRzlSgqF\n"
    "9YZWiaIPf2z3SKP9cMPCovJsoyq9dlaadbJI2k7J8luJ7/DeTLUlmNl6FhNd2tA8\n"
    "R/rVu16ZLhV5IIvPq+bW6le2NoZ3FWiEMQbYrvSeo+BWN8ldjNL1H/El3UDHUa0S\n"
    "6C6LkUFLVz1U7ROwWuAkIWqmE0G4RuPdA4ZvXzNr44/Z9y+OqUw2ipwJGy7JT3Lh\n"
    "/b0CAwEAAaNgMF4wHwYDVR0jBBgwFoAU4tvnG1lu7N4aGf+2z63oZr7Mg4MwHQYD\n"
    "VR0OBBYEFPMyT4b/AeEaZI0+cnRDqf8T/4jsMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
    "AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAIhCfN8NziqXJG3o9dWKd3jZ7f\n"
    "2U0CjRh4s6zCRNaZkE4kyga9XnhDKHQGq5NaIcWeqtrHkIVpN8/tGMp83SlO1Q+s\n"
    "9YMcv/JJB4rp2x85u5ieOFct1zCSXioiUTVxuQToNqZfJPoepqvN1H3cDl54xLlv\n"
    "7PhHmF43WpFJ68qLa5YcP0h+919TaPvH5paRuRB1noloZPn7xjSc1FLGn/xpYEQ6\n"
    "hOrj61tl9VmWWasE2id8V7JQwEZq9jH/YysxiGk6mL2RooizWGhVmOd8u95UH6w9\n"
    "mXeK+o89T+59a06I6xf42bjgcPxJv92qeWg7zIqyxc67sGffrPH/8zAUuUEC\n"
    "-----END CERTIFICATE-----\n";

const char rootCa[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
    "-----END CERTIFICATE-----\n";

static const char *TAG = "MQTTS_EXAMPLE";

static void log_error_if_nonzero(const char *message, int error_code)
{
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base,
           event_id);
  esp_mqtt_event_handle_t event   = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_subscribe(client, "/pp/Lb/eu", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    printf("MQTT_EVENT_DATA, received %d bytes\r\n", event->data_len);
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    // printf("DATA=");
    // for (size_t i = 0; i < event->data_len; i++)
    //   printf("%02X", event->data[i]);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls",
                           event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack",
                           event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",
                           event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)",
               strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

static void mqtt_app_start(void)
{
  esp_log_level_set(TAG, ESP_LOG_INFO);
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri             = "mqtts://pp.services.u-blox.com:8883",
      .client_id       = "ebf8071d-f02e-4882-8cf8-54ff397f0e45",
      .buffer_size     = 12288,
      .client_cert_pem = clientCert,
      .client_key_pem  = privateKey,
      .cert_pem        = rootCa,
  };

  log_i("[APP] Free memory: %d bytes", esp_get_free_heap_size());
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  esp_mqtt_client_start(client);
}

esp_err_t pp_init(const char *_rootCa, const char *_clientCert,
                  const char *_clientKey)
{
  mqtt_app_start();

  return ESP_OK;
}