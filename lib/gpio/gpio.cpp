#include "gpio.h"

void TaskLed(void *pvParameters)
{
  (void)pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    switch (led.colour) {
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
    vTaskDelay(pdMS_TO_TICKS(1000 / led.rate));
  }
  vTaskDelete(NULL);
}

void set_led(colour_t colour, uint32_t rate)
{
  led.colour = colour;
  led.rate   = rate;
}

void gpios_init(void)
{
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(15, HIGH);
  digitalWrite(16, HIGH);

  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);

  xTaskCreate(TaskLed, "TaskLed", 1024, NULL, 0, NULL);
}