#include "driving_mode.h"
#include "esp_system.h"
#include "rover_config.h"

static void steer_normal(uint16_t signal);
static void steer_spin(uint16_t signal);

enum MotorDirection { MOTOR_IDLE, MOTOR_FORWARD, MOTOR_BACKWARD };

static MotorDirection motor_state   = MOTOR_IDLE;
static RoverMode current_rover_mode = DRIVE_TURN_NORMAL;

void rover_driving_set_drive_mode(RoverMode mode) { current_rover_mode = mode; }

void rover_driving_move(uint16_t signal)
{
  switch (current_rover_mode) {
  case DRIVE_TURN_NORMAL:
    if (signal < RC_CENTER && signal > 0) { // MOTOR_Backward
      // When going from MOTOR_forward/MOTOR_idle to reverse, motors (ESC) need
      // to be set in reverse mode
      if (motor_state == MOTOR_IDLE) {
        motor_state = MOTOR_BACKWARD;
      }

    } else if (signal > RC_CENTER) {
      motor_state = MOTOR_FORWARD;
    } else {
      motor_state = MOTOR_IDLE;
    }
    break;
  case DRIVE_TURN_SPIN: {
    uint16_t diff = abs((int16_t)RC_CENTER - (int16_t)signal);

    if (signal < RC_CENTER && signal > 0) { // Spin Anticlockwise
      if (motor_state == MOTOR_IDLE) {
        motor_state = MOTOR_FORWARD;
      }
    } else if (signal > RC_CENTER) {
      if (motor_state == MOTOR_IDLE) {
        motor_state = MOTOR_FORWARD;
      }
    } else {
      motor_state = MOTOR_IDLE;
    }
    break;
  }
  default:
    break;
  }
}

void rover_driving_steer(uint16_t signal)
{
  switch (current_rover_mode) {
  case DRIVE_TURN_NORMAL:
    steer_normal(signal);
    break;
  case DRIVE_TURN_SPIN:
    steer_spin(signal);
    break;
  default:
    break;
  }
}

static void steer_normal(uint16_t signal)
{
  uint16_t diff = abs((int16_t)RC_CENTER - (int16_t)signal);
  if (signal < RC_CENTER && signal > 0) { // Left turn
  } else if (signal > RC_CENTER) {        // Right turn
  } else {
  }
}

static void steer_spin(uint16_t signal) {}
