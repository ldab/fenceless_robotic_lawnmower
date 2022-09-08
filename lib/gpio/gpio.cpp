#include "gpio.h"

void TaskLed(void *pvParameters)
{
  (void)pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    switch (led.color) {
    case RED:
      digitalWrite(LED_R, !digitalRead(LED_R));
      break;
    case BLUE:
      digitalWrite(LED_B, !digitalRead(LED_B));
      break;
    case GREEN:
      digitalWrite(LED_G, !digitalRead(LED_G));
      break;
    default:
      digitalWrite(LED_B, !digitalRead(LED_B));
      digitalWrite(LED_R, !digitalRead(LED_R));
      digitalWrite(LED_G, !digitalRead(LED_G));
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void gpios_init(void)
{
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);

  xTaskCreate(TaskLed, "TaskLed", 1024, NULL, 0, NULL);
}