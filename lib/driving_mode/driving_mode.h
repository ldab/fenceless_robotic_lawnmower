#pragma once

#include <Arduino.h>

typedef enum RoverMode {
  DRIVE_TURN_NORMAL,
  DRIVE_TURN_SPIN,
  ROBOT_ARM,
} RoverMode;

typedef enum SpinDirection_t {
  CLOCKWISE,
  ANTICLOCKWISE,
} SpinDirection;

typedef struct driveSignal_t {
  uint16_t steer;
  uint16_t direction;
} driveSignal;

void rover_driving_move(driveSignal_t *signal);
void steer_spin(SpinDirection_t direction, uint8_t speed);