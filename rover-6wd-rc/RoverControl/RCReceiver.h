#ifndef RC_RECEIVER_H
#define RC_RECEIVER_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
//  RCReceiver
//  Interrupt-driven reader for standard 1000-2000us servo PWM channels coming
//  from an RC receiver. Each configured channel is measured on a CHANGE
//  interrupt, so reading is non-blocking (no pulseIn() stalls).
//
//  Includes a failsafe based on signal freshness: if the throttle/steer
//  channels stop updating (receiver unplugged, Tx off, out of range) the
//  caller is told to stop the rover.
// ---------------------------------------------------------------------------
namespace RC {

  // Attach interrupts and initialise channels to a safe neutral.
  void begin();

  // Latest valid pulse width (us) for a channel (0..RC_CHANNEL_COUNT-1).
  // Returns RC_PULSE_CENTER for an out-of-range index.
  uint16_t pulse(uint8_t channel);

  // Milliseconds since this channel last produced a valid pulse.
  // Returns a very large value if it has never been seen.
  unsigned long age(uint8_t channel);

  // True if the essential channels (throttle & steer) are stale -> stop.
  bool failsafe();

}  // namespace RC

#endif  // RC_RECEIVER_H
