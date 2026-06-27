#include "Drivetrain.h"

void Drivetrain::begin() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    _motors[i].attach(MOTOR_EN_PIN[i], MOTOR_IN1_PIN[i],
                      MOTOR_IN2_PIN[i], MOTOR_INVERT[i]);
    _motors[i].begin();
  }
  _leftTarget = _rightTarget = 0;
  _leftCurrent = _rightCurrent = 0;
  _lastUpdate = millis();
}

void Drivetrain::drive(int leftTarget, int rightTarget) {
  _leftTarget  = constrain(leftTarget,  -DRIVE_MAX_SPEED, DRIVE_MAX_SPEED);
  _rightTarget = constrain(rightTarget, -DRIVE_MAX_SPEED, DRIVE_MAX_SPEED);
}

void Drivetrain::stop() {
  _leftTarget  = 0;
  _rightTarget = 0;
}

void Drivetrain::update() {
  unsigned long now = millis();
  unsigned long dt  = now - _lastUpdate;
  if (dt == 0) return;            // nothing to integrate yet this millisecond
  _lastUpdate = now;

  // Clamp dt before the multiply below: a long stall (or a debugger pause)
  // could otherwise overflow the 16-bit int when narrowing 'step'. A single
  // tick never needs to move more than the full speed range anyway.
  if (dt > 100UL) dt = 100UL;

  // Largest speed change allowed this tick, scaled by elapsed time.
  long step = ((long)SLEW_RATE_PER_SEC * (long)dt) / 1000L;
  if (step < 1)               step = 1;                // always make progress
  if (step > DRIVE_MAX_SPEED) step = DRIVE_MAX_SPEED;  // cannot exceed full scale
  int maxStep = (int)step;

  _leftCurrent  = slew(_leftCurrent,  _leftTarget,  maxStep);
  _rightCurrent = slew(_rightCurrent, _rightTarget, maxStep);

  applySide(LEFT_MOTORS,  LEFT_MOTOR_COUNT,  _leftCurrent);
  applySide(RIGHT_MOTORS, RIGHT_MOTOR_COUNT, _rightCurrent);
}

void Drivetrain::emergencyBrake() {
  _leftTarget = _rightTarget = 0;
  _leftCurrent = _rightCurrent = 0;
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    _motors[i].brake();
  }
  _lastUpdate = millis();
}

void Drivetrain::applySide(const uint8_t* idx, uint8_t count, int speed) {
  for (uint8_t i = 0; i < count; i++) {
    _motors[idx[i]].setSpeed(speed);
  }
}

int Drivetrain::slew(int current, int target, int maxStep) {
  if (current < target) {
    current += maxStep;
    if (current > target) current = target;
  } else if (current > target) {
    current -= maxStep;
    if (current < target) current = target;
  }
  return current;
}
