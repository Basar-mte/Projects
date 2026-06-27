#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
//  Rover 6WD RC  -  Build configuration
//  Target board: Arduino Mega 2560
//
//  Everything you are likely to change (pins, limits, tuning) lives here.
//  No code edits should be needed for a normal build - only this file.
//
//  Drivetrain layout (skid-steer / tank style):
//
//        FRONT  (bogie suspension, 4 wheels)
//        +------------------+
//   M1 O |  FL1        FR1  | O M4
//   M2 O |  FL2        FR2  | O M5
//        |                  |
//   M3 O |  RL          RR  | O M6
//        +------------------+
//        REAR  (2 wheels)
//
//  LEFT side  : M1, M2, M3      RIGHT side : M4, M5, M6
//  Steering is achieved by spinning the two sides at different speeds.
//  The bogie/rear split is purely mechanical; the controller only treats
//  the motors as a LEFT group and a RIGHT group.
// ============================================================================


// ---------------------------------------------------------------------------
//  Motor indexing
// ---------------------------------------------------------------------------
#define MOTOR_COUNT 6

enum MotorIndex {
  M_FRONT_LEFT_1  = 0,  // front bogie, left  (outer)
  M_FRONT_LEFT_2  = 1,  // front bogie, left  (inner)
  M_REAR_LEFT     = 2,  // rear left
  M_FRONT_RIGHT_1 = 3,  // front bogie, right (outer)
  M_FRONT_RIGHT_2 = 4,  // front bogie, right (inner)
  M_REAR_RIGHT    = 5   // rear right
};


// ---------------------------------------------------------------------------
//  L298N motor control pins (one entry per motor, indexed by MotorIndex)
//
//    EN  = PWM speed pin     -> MUST be a PWM-capable pin
//    IN1 = direction pin A   -> any digital pin
//    IN2 = direction pin B   -> any digital pin
//
//  PWM pins used (5,6,7,8,9,10) are on Timer2/3/4 (pin 5 -> Timer3,
//  pins 6/7/8 -> Timer4, pins 9/10 -> Timer2) - chosen on purpose so they do
//  NOT disturb Timer0 (pins 4 & 13), which drives millis()/micros()/delay().
//  Avoid pins 2 & 3 here: they are reserved for RC interrupts below.
//
//  Board grouping:
//    L298N #1 -> M_FRONT_LEFT_1 , M_FRONT_LEFT_2   (left  bogie)
//    L298N #2 -> M_FRONT_RIGHT_1, M_FRONT_RIGHT_2  (right bogie)
//    L298N #3 -> M_REAR_LEFT    , M_REAR_RIGHT     (rear axle)
// ---------------------------------------------------------------------------
static const uint8_t MOTOR_EN_PIN[MOTOR_COUNT]  = {  5,  6,  9,  7,  8, 10 };
static const uint8_t MOTOR_IN1_PIN[MOTOR_COUNT] = { 22, 24, 30, 26, 28, 32 };
static const uint8_t MOTOR_IN2_PIN[MOTOR_COUNT] = { 23, 25, 31, 27, 29, 33 };

// Flip an individual motor's direction in software. Use this after a bench
// test if a wheel spins the wrong way - no rewiring needed. The right side
// defaults to inverted because those motors are mounted mirror-image.
static const bool MOTOR_INVERT[MOTOR_COUNT] = { false, false, false,
                                                true,  true,  true  };

// Which motors belong to each side (used for skid-steer mixing).
static const uint8_t LEFT_MOTORS[]  = { M_FRONT_LEFT_1,  M_FRONT_LEFT_2,  M_REAR_LEFT  };
static const uint8_t RIGHT_MOTORS[] = { M_FRONT_RIGHT_1, M_FRONT_RIGHT_2, M_REAR_RIGHT };
#define LEFT_MOTOR_COUNT  (sizeof(LEFT_MOTORS)  / sizeof(LEFT_MOTORS[0]))
#define RIGHT_MOTOR_COUNT (sizeof(RIGHT_MOTORS) / sizeof(RIGHT_MOTORS[0]))


// ---------------------------------------------------------------------------
//  RC receiver (standard 1000-2000us servo PWM, one signal wire per channel)
//
//  Signal pins MUST be external-interrupt-capable on the Mega 2560:
//      2, 3, 18, 19, 20, 21
//  (20/21 double as I2C SDA/SCL - only use them if you are not using I2C.)
//
//  Wire your receiver outputs to the pins below. The default mapping assumes
//  a typical 4-channel-or-more transmitter:
//      Tx CH1 (aileron / right-stick L-R) -> pin 2  -> RC_STEER
//      Tx CH3 (throttle / left-stick U-D) -> pin 3  -> RC_THROTTLE
//      Tx CH5 (2-pos switch)              -> pin 18 -> RC_ARM
//      Tx CH6 (rotary knob / VrA)         -> pin 19 -> RC_SPEED
//
//  NOTE: RCReceiver.cpp defines exactly RC_CHANNEL_COUNT interrupt handlers.
//        If you change this count you must add matching handlers there
//        (a compile-time check will remind you).
// ---------------------------------------------------------------------------
#define RC_CHANNEL_COUNT 4

enum RcChannel {
  RC_STEER    = 0,   // right stick left/right  -> turning
  RC_THROTTLE = 1,   // left  stick up/down     -> forward/back
  RC_ARM      = 2,   // 2-position switch       -> arm / disarm
  RC_SPEED    = 3    // rotary knob             -> max speed limit
};

static const uint8_t RC_PIN[RC_CHANNEL_COUNT] = { 2, 3, 18, 19 };

// Pulse-width interpretation (microseconds)
#define RC_PULSE_MIN        1000   // stick / knob fully one way
#define RC_PULSE_CENTER     1500   // neutral
#define RC_PULSE_MAX        2000   // stick / knob fully other way
#define RC_DEADBAND           40   // +/- us around center treated as zero
#define RC_PULSE_VALID_MIN   900   // below this = glitch, ignore the pulse
#define RC_PULSE_VALID_MAX  2100   // above this = glitch, ignore the pulse
#define RC_TIMEOUT_MS        200   // no fresh valid pulse within this -> failsafe
#define RC_ARM_THRESHOLD    1700   // arm switch pulse >= this = ARMED

// Optional features - set to 0 to disable a channel entirely.
#define USE_ARM_SWITCH        1    // 0 = always armed (no kill switch!)
#define USE_SPEED_KNOB        1    // 0 = always full speed


// ---------------------------------------------------------------------------
//  Drive tuning
// ---------------------------------------------------------------------------
#define MOTOR_MAX_PWM      255   // hard ceiling passed to analogWrite (0-255)
#define DRIVE_MAX_SPEED    255   // max commanded speed magnitude per side

#define SPEED_LIMIT_MIN     60   // PWM ceiling when the speed knob is at minimum
#define SPEED_LIMIT_MAX    255   // PWM ceiling when the speed knob is at maximum

#define SLEW_RATE_PER_SEC  600   // max change in PWM units per second (accel limit)
                                 // lower = gentler ramps, easier on gearboxes

#define BRAKE_ON_FAILSAFE  true  // true = active brake on signal loss, false = coast


// ---------------------------------------------------------------------------
//  Loop timing & debug
// ---------------------------------------------------------------------------
#define CONTROL_LOOP_HZ    100   // control update rate
#define SERIAL_BAUD     115200
#define DEBUG_ENABLED        1   // 1 = print telemetry over Serial, 0 = silent
#define DEBUG_PERIOD_MS    250   // telemetry print interval

#endif  // CONFIG_H
