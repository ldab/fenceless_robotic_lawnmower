#pragma once

#include <Arduino.h>

typedef enum RoverMode {
  DRIVE_TURN_NORMAL,
  DRIVE_TURN_SPIN,
  ROBOT_ARM,
} RoverMode;

void rover_driving_set_drive_mode(RoverMode mode);
void rover_driving_move(uint16_t signal);
void rover_driving_steer(uint16_t signal);