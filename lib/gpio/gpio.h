#pragma once

#include "Arduino.h"

#define BUTTON1 46
#define BUTTON2 0
#define BUTTON3 47
#define BUTTON4 48
#define GPIO_INPUT_PIN_SEL                                                     \
  ((1ULL << BUTTON2) | (1ULL << BUTTON3) | (1ULL << BUTTON4))
#define LED_R 5
#define LED_G 3
#define LED_B 8
#define GPIO_OUTPUT_PIN_SEL                                                    \
  ((1ULL << LED_R) | (1ULL << LED_G) | (1ULL << LED_B))
#define ESP_INTR_FLAG_DEFAULT 0

#define MOTOR_STANDBY         14
#define MOTOR_R1              11
#define MOTOR_R2              47
#define MOTOR_L1              10
#define MOTOR_L2              39

typedef enum { RED, GREEN, YELLOW, BLUE, CYAN, WHITE, PURPLE } colour_t;
typedef enum { SLOW = 2, MEDIUM = 4, FAST = 6 } rate_t;

static struct {
  colour_t colour;
  uint32_t rate;
} led;

void gpios_init(void);
void set_led(colour_t colour, uint32_t rate = 2);