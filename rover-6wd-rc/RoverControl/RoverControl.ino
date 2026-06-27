/*
 * ===========================================================================
 *  Rover 6WD RC  -  Arduino Mega 2560
 * ===========================================================================
 *  Six DC motors (front 4-wheel bogie + rear 2-wheel axle) driven through
 *  3x L298N dual H-bridges, commanded from an RC receiver. Works with a
 *  10-channel set; the firmware uses 4 channels (steer, throttle, arm, speed).
 *
 *  Control model: skid-steer (tank). Arcade mixing turns one stick into
 *  left/right side speeds.
 *
 *  Pipeline each control tick:
 *      RC pulses --> failsafe check --> arming --> mix --> slew --> motors
 *
 *  All wiring, pins and tuning live in Config.h. See README.md / docs/.
 *  No external libraries required.
 * ===========================================================================
 */
#include <Arduino.h>
#include "Config.h"
#include "RCReceiver.h"
#include "Drivetrain.h"
#include "Mixer.h"

Drivetrain drivetrain;

bool     g_armed    = false;
uint32_t g_lastLoop = 0;

// Forward declarations
static bool readArmed();
static int  readSpeedLimit();
#if DEBUG_ENABLED
static uint32_t g_lastDebug = 0;
static void printDebug(int thr, int steer, bool failsafe);
#endif

void setup() {
#if DEBUG_ENABLED
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Rover 6WD RC - booting"));
#endif
  drivetrain.begin();
  RC::begin();
  drivetrain.emergencyBrake();   // guarantee a stopped, braked start state
}

void loop() {
  // Fixed-rate control loop.
  uint32_t now = millis();
  if (now - g_lastLoop < (1000UL / CONTROL_LOOP_HZ)) return;
  g_lastLoop = now;

  const bool fs = RC::failsafe();

  // Read sticks (used for driving and for the arm-on-neutral check).
  const int thrRaw   = Mixer::pulseToSigned(RC::pulse(RC_THROTTLE), DRIVE_MAX_SPEED);
  const int steerRaw = Mixer::pulseToSigned(RC::pulse(RC_STEER),    DRIVE_MAX_SPEED);

  // --- Arming state machine -------------------------------------------------
  // Arm only when: signal is healthy, the arm switch is on, AND the throttle
  // is centered (prevents a lurch if you arm with the stick pushed).
  const bool wantArmed = !fs && readArmed();
  if (!g_armed) {
    if (wantArmed && thrRaw == 0) g_armed = true;
  } else {
    if (!wantArmed) g_armed = false;
  }

  // --- Drive ----------------------------------------------------------------
  if (fs) {
#if BRAKE_ON_FAILSAFE
    drivetrain.emergencyBrake();
#else
    drivetrain.stop();
    drivetrain.update();
#endif
  } else if (!g_armed) {
    drivetrain.stop();
    drivetrain.update();
  } else {
    const int speedLimit = readSpeedLimit();               // PWM ceiling from knob
    const int thr   = (int)((long)thrRaw   * speedLimit / DRIVE_MAX_SPEED);
    const int steer = (int)((long)steerRaw * speedLimit / DRIVE_MAX_SPEED);

    int left = 0, right = 0;
    Mixer::arcade(thr, steer, speedLimit, left, right);
    drivetrain.drive(left, right);
    drivetrain.update();
  }

#if DEBUG_ENABLED
  if (now - g_lastDebug >= DEBUG_PERIOD_MS) {
    g_lastDebug = now;
    printDebug(thrRaw, steerRaw, fs);
  }
#endif
}

// Returns true if the arm switch says "armed". When the switch is disabled in
// Config.h the rover is always armed; a missing switch signal = disarmed.
static bool readArmed() {
#if USE_ARM_SWITCH
  if (RC::age(RC_ARM) > RC_TIMEOUT_MS) return false;
  return RC::pulse(RC_ARM) >= RC_ARM_THRESHOLD;
#else
  return true;
#endif
}

// Returns the current PWM speed ceiling from the knob. A missing/disabled
// knob defaults to full speed.
static int readSpeedLimit() {
#if USE_SPEED_KNOB
  if (RC::age(RC_SPEED) > RC_TIMEOUT_MS) return SPEED_LIMIT_MAX;
  return Mixer::pulseToRange(RC::pulse(RC_SPEED), SPEED_LIMIT_MIN, SPEED_LIMIT_MAX);
#else
  return SPEED_LIMIT_MAX;
#endif
}

#if DEBUG_ENABLED
static void printDebug(int thr, int steer, bool failsafe) {
  Serial.print(F("FS:"));    Serial.print(failsafe ? 1 : 0);
  Serial.print(F(" ARM:"));  Serial.print(g_armed ? 1 : 0);
  Serial.print(F(" THR:"));  Serial.print(thr);
  Serial.print(F(" STR:"));  Serial.print(steer);
  Serial.print(F(" L:"));    Serial.print(drivetrain.leftSpeed());
  Serial.print(F(" R:"));    Serial.print(drivetrain.rightSpeed());
  Serial.print(F(" thrUs:")); Serial.print(RC::pulse(RC_THROTTLE));
  Serial.print(F(" strUs:")); Serial.println(RC::pulse(RC_STEER));
}
#endif
