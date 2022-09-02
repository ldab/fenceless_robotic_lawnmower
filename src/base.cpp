#include <Arduino.h>

#include <HTTPClient.h>

#include "reset_reason.h"
#include "wifi_helper.h"
#include "gpio.h"

WiFiServer server(80);

void TaskStatus(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_R, !digitalRead(LED_R));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  gpios_init();

  xTaskCreate(TaskStatus, "TaskStatus", 1024, NULL, 2, NULL);

  verbose_print_reset_reason();

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("dhcpc", ESP_LOG_DEBUG);
  esp_log_level_set("dhcps", ESP_LOG_DEBUG);
  esp_log_level_set("lwip", ESP_LOG_DEBUG);
  esp_log_level_set("wifi", ESP_LOG_DEBUG);

  wifi_init_softapsta();

  server.begin();
}

void loop()
{

  // if (wifi_sta_connected) {
  //   HTTPClient http;

  //   http.begin("http://httpbin.org/get"); // HTTP

  //   int httpCode = http.GET();

  //   // httpCode will be negative on error
  //   if (httpCode > 0) {
  //     // HTTP header has been send and Server response header has been handled
  //     Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  //   } else {
  //     Serial.printf("[HTTP] GET... failed, error: %s\n",
  //                   http.errorToString(httpCode).c_str());
  //   }

  //   http.end();
  // }

  WiFiClient client = server.available(); // listen for incoming clients
  if (client) {                           // if you get a client,

    String currentLine =
        ""; // make a String to hold incoming data from the client
    while (client.connected()) { // loop while the client's connected
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        if (c == '\n') {         // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a
          // row. that's the end of the client HTTP request, so send a
          // response:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            break;
          } else { // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') { // if you got anything else but a carriage
                                // return character,
          currentLine += c;     // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(5, HIGH); // GET /H turns the LED on
        }
      }
    }
    // close the connection:
    client.stop();
  }

  delay(5000);
}