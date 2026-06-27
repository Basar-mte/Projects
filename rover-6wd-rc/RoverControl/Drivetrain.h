#ifndef DRIVETRAIN_H
#define DRIVETRAIN_H

#include <Arduino.h>
#include "MotorDriver.h"
#include "Config.h"

// ---------------------------------------------------------------------------
//  Drivetrain
//  Owns all 6 motors and presents the rover as two sides (LEFT / RIGHT) for
//  skid-steer control. Applies a slew-rate (acceleration) limit so commands
//  ramp smoothly instead of slamming the gearboxes.
//
//  Usage each control tick:
//      drivetrain.drive(left, right);   // set targets (-255..255)
//      drivetrain.update();             // ramp toward targets, drive motors
// ---------------------------------------------------------------------------
class Drivetrain {
public:
  // Attach + initialise all motors from Config.h. Leaves the rover stopped.
  void begin();

  // Set target speeds for each side, -DRIVE_MAX_SPEED..+DRIVE_MAX_SPEED.
  void drive(int leftTarget, int rightTarget);

  // Ramp current speeds toward the targets and write them to the motors.
  // Call this regularly (every control loop). Slew is time-based, so an
  // irregular loop rate still produces approximately the configured
  // acceleration (SLEW_RATE_PER_SEC).
  void update();

  // Set both targets to zero (rover coasts to a stop via the slew limiter).
  void stop();

  // Immediate hard stop: active-brake every motor, bypassing the slew limit.
  void emergencyBrake();

  int leftSpeed()  const { return _leftCurrent;  }
  int rightSpeed() const { return _rightCurrent; }

private:
  MotorDriver   _motors[MOTOR_COUNT];
  int           _leftTarget   = 0;
  int           _rightTarget  = 0;
  int           _leftCurrent  = 0;
  int           _rightCurrent = 0;
  unsigned long _lastUpdate   = 0;

  void applySide(const uint8_t* idx, uint8_t count, int speed);
  static int slew(int current, int target, int maxStep);
};

#endif  // DRIVETRAIN_H
