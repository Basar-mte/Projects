#include "MotorDriver.h"
#include "Config.h"

MotorDriver::MotorDriver()
  : _en(255), _in1(255), _in2(255), _inverted(false), _attached(false), _speed(0) {}

void MotorDriver::attach(uint8_t enPin, uint8_t in1Pin, uint8_t in2Pin, bool inverted) {
  _en       = enPin;
  _in1      = in1Pin;
  _in2      = in2Pin;
  _inverted = inverted;
  _attached = true;
}

void MotorDriver::begin() {
  if (!_attached) return;
  pinMode(_en,  OUTPUT);
  pinMode(_in1, OUTPUT);
  pinMode(_in2, OUTPUT);
  coast();
}

void MotorDriver::setSpeed(int speed) {
  if (!_attached) return;

  // Clamp to the safe PWM range.
  if (speed >  MOTOR_MAX_PWM) speed =  MOTOR_MAX_PWM;
  if (speed < -MOTOR_MAX_PWM) speed = -MOTOR_MAX_PWM;
  _speed = speed;  // store the commanded (rover-frame) value for telemetry

  // Apply software inversion for mirror-mounted motors.
  int out = _inverted ? -speed : speed;

  if (out == 0) {
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);
    analogWrite(_en, 0);
    return;
  }

  digitalWrite(_in1, out > 0 ? HIGH : LOW);
  digitalWrite(_in2, out > 0 ? LOW  : HIGH);
  analogWrite(_en, abs(out));
}

void MotorDriver::brake() {
  if (!_attached) return;
  digitalWrite(_in1, HIGH);
  digitalWrite(_in2, HIGH);
  analogWrite(_en, MOTOR_MAX_PWM);
  _speed = 0;
}

void MotorDriver::coast() {
  if (!_attached) return;
  digitalWrite(_in1, LOW);
  digitalWrite(_in2, LOW);
  analogWrite(_en, 0);
  _speed = 0;
}
