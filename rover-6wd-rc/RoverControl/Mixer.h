#ifndef MIXER_H
#define MIXER_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
//  Mixer
//  Pure, stateless helpers that turn raw RC pulse widths into motor commands.
//  Kept free of hardware access so the math can be reasoned about (and unit
//  tested on a PC) in isolation.
// ---------------------------------------------------------------------------
namespace Mixer {

  // Map an RC pulse (us) to a signed value in [-outMax, +outMax], centered at
  // RC_PULSE_CENTER with a symmetric deadband. Output starts at 0 just past
  // the deadband so there is no sudden jump off neutral.
  int pulseToSigned(uint16_t pulseUs, int outMax);

  // Map an RC pulse (us) linearly to [outMin, outMax]
  // (e.g. a rotary knob -> a maximum-speed ceiling).
  int pulseToRange(uint16_t pulseUs, int outMin, int outMax);

  // Arcade (single-stick) mixing.
  //   throttle, steer : in [-range, +range]
  //   left, right     : outputs in [-range, +range]
  // If a combined channel would exceed 'range', BOTH sides are scaled down
  // proportionally so the turn shape is preserved instead of clipping.
  void arcade(int throttle, int steer, int range, int &left, int &right);

}  // namespace Mixer

#endif  // MIXER_H
