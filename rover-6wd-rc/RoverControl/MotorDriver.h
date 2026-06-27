#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
//  MotorDriver
//  Controls ONE motor channel of an L298N (or any 2-direction + PWM-enable
//  H-bridge). Speed is signed: positive = forward, negative = reverse.
//
//  An L298N channel uses three MCU pins:
//      EN  (ENA/ENB) -> PWM speed
//      IN1 (IN1/IN3) -> direction
//      IN2 (IN2/IN4) -> direction
// ---------------------------------------------------------------------------
class MotorDriver {
public:
  MotorDriver();

  // Configure the pins for this motor. Call once before begin().
  // 'inverted' flips the meaning of forward/reverse in software.
  void attach(uint8_t enPin, uint8_t in1Pin, uint8_t in2Pin, bool inverted);

  // Set pin modes and leave the motor stopped (coasting).
  void begin();

  // Drive the motor. speed is clamped to [-MOTOR_MAX_PWM, +MOTOR_MAX_PWM].
  // 0 = coast.
  void setSpeed(int speed);

  // Active short brake (both inputs high, enable on). Stronger stop than coast.
  void brake();

  // Free-wheel / coast (enable off).
  void coast();

  // Last commanded speed (sign reflects rover-forward, not wiring).
  int speed() const { return _speed; }

private:
  uint8_t _en;
  uint8_t _in1;
  uint8_t _in2;
  bool    _inverted;
  bool    _attached;
  int     _speed;
};

#endif  // MOTOR_DRIVER_H
