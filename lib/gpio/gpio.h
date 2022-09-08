#pragma once

#include "Arduino.h"

#define BUTTON1 46
#define BUTTON2 0
#define BUTTON3 47
#define BUTTON4 48
#define GPIO_INPUT_PIN_SEL                                                     \
  ((1ULL << BUTTON2) | (1ULL << BUTTON3) | (1ULL << BUTTON4))
#define LED_R 5
#define LED_G 2
#define LED_B 8
#define GPIO_OUTPUT_PIN_SEL                                                    \
  ((1ULL << LED_R) | (1ULL << LED_G) | (1ULL << LED_B))
#define ESP_INTR_FLAG_DEFAULT 0

static uint8_t uLedParams;

typedef enum { RED, GREEN, BLUE } color_t;

static struct led_t {
  color_t color;
  uint32_t freq;
} led;

void gpios_init(void);