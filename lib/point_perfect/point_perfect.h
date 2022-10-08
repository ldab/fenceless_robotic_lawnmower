#ifndef __PP_H__
#define __PP_H__

#include "esp_err.h"
#include "esp_event.h"
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(mqttSubCb(char *));

esp_err_t pp_init(const char *_rootCa, const char *_clientCert,
                  const char *_clientKey);
// esp_err_t pp_connect(const char *brokerUrl, uint16_t port,
//                      const char *clientId);
esp_err_t pp_subscribe(const char *topic, esp_event_handler_t subCb);
// esp_err_t pp_onMessageCb(void *onMqttMessage);

#ifdef __cplusplus
}
#endif

#endif /* __PP_H__ */