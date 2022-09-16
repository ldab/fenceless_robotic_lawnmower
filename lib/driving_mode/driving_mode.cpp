#include "driving_mode.h"
#include "Arduino.h"
#include "esp_system.h"
#include "gpio.h"
#include "rover_config.h"

#define FULL 255

enum MotorDirection { MOTOR_IDLE, MOTOR_FORWARD, MOTOR_BACKWARD };

static MotorDirection motor_state   = MOTOR_IDLE;
static RoverMode current_rover_mode = DRIVE_TURN_NORMAL;

void rover_driving_move(driveSignal_t *signal)
{
  log_v("steer %d direction %d", signal->steer, signal->direction);

  // steer right side of controller, 1000 to 2000;
  // direction left side of controller, 1000 to 2000;
  float m           = (float)(FULL + 0) / (2000 - 1500);
  float _direction  = m * signal->direction - m * 1500;
  int16_t direction = static_cast<int16_t>(_direction);
  float _steer      = m * signal->steer - m * 1500;
  int16_t steer     = static_cast<int16_t>(_steer);

  if (signal->steer == 1500) {
    // forward and backward
    if (direction > 0) {
      digitalWrite(MOTOR_STANDBY, HIGH);
      analogWrite(MOTOR_LEFTFORW, direction);
      analogWrite(MOTOR_LEFTREV, LOW);
      analogWrite(MOTOR_RIGHTFORW, LOW);
      analogWrite(MOTOR_RIGHTREV, direction);
    } else {
      digitalWrite(MOTOR_STANDBY, HIGH);
      analogWrite(MOTOR_LEFTFORW, LOW);
      analogWrite(MOTOR_LEFTREV, abs(direction));
      analogWrite(MOTOR_RIGHTFORW, abs(direction));
      analogWrite(MOTOR_RIGHTREV, LOW);
    }
  } else if (signal->direction == 1500) {
    // spin/rotate
    if (steer > 0) {
      steer_spin(CLOCKWISE, abs(steer));
    } else {
      steer_spin(ANTICLOCKWISE, abs(steer));
    }
  } else {
    if (direction > 0 && steer > 0) {
      digitalWrite(MOTOR_STANDBY, HIGH);
      analogWrite(MOTOR_LEFTFORW, direction);
      analogWrite(MOTOR_LEFTREV, LOW);
      analogWrite(MOTOR_RIGHTFORW, LOW);
      analogWrite(MOTOR_RIGHTREV, direction - steer);
    } else if (direction > 0 && steer < 0) {
      digitalWrite(MOTOR_STANDBY, HIGH);
      analogWrite(MOTOR_LEFTFORW, direction - steer);
      analogWrite(MOTOR_LEFTREV, LOW);
      analogWrite(MOTOR_RIGHTFORW, LOW);
      analogWrite(MOTOR_RIGHTREV, direction);
    } else if (direction < 0 && steer > 0) {
      digitalWrite(MOTOR_STANDBY, HIGH);
      analogWrite(MOTOR_LEFTFORW, LOW);
      analogWrite(MOTOR_LEFTREV, abs(direction));
      analogWrite(MOTOR_RIGHTFORW, abs(direction) + steer);
      analogWrite(MOTOR_RIGHTREV, LOW);
    } else {
      digitalWrite(MOTOR_STANDBY, HIGH);
      analogWrite(MOTOR_LEFTFORW, LOW);
      analogWrite(MOTOR_LEFTREV, abs(direction) + steer);
      analogWrite(MOTOR_RIGHTFORW, abs(direction));
      analogWrite(MOTOR_RIGHTREV, LOW);
    }
  }
}

void steer_spin(SpinDirection_t direction, uint8_t speed)
{
  if (direction == CLOCKWISE) {
    digitalWrite(MOTOR_STANDBY, HIGH);
    analogWrite(MOTOR_LEFTFORW, speed);
    analogWrite(MOTOR_LEFTREV, LOW);
    analogWrite(MOTOR_RIGHTFORW, LOW);
    analogWrite(MOTOR_RIGHTREV, LOW);
  } else {
    digitalWrite(MOTOR_STANDBY, HIGH);
    analogWrite(MOTOR_LEFTFORW, LOW);
    analogWrite(MOTOR_LEFTREV, LOW);
    analogWrite(MOTOR_RIGHTFORW, LOW);
    analogWrite(MOTOR_RIGHTREV, speed);
  }
}