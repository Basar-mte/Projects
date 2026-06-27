#include "Mixer.h"
#include "Config.h"

int Mixer::pulseToSigned(uint16_t pulseUs, int outMax) {
  // Clamp into the nominal stick range first.
  if (pulseUs < RC_PULSE_MIN) pulseUs = RC_PULSE_MIN;
  if (pulseUs > RC_PULSE_MAX) pulseUs = RC_PULSE_MAX;

  int delta = (int)pulseUs - RC_PULSE_CENTER;
  if (abs(delta) <= RC_DEADBAND) return 0;

  // Shift out the deadband so motion ramps from 0 rather than jumping.
  if (delta > 0) delta -= RC_DEADBAND;
  else           delta += RC_DEADBAND;

  long span = (long)(RC_PULSE_MAX - RC_PULSE_CENTER) - RC_DEADBAND;
  if (span <= 0) span = 1;  // guard against a misconfigured deadband

  long out = (long)delta * outMax / span;
  if (out >  outMax) out =  outMax;
  if (out < -outMax) out = -outMax;
  return (int)out;
}

int Mixer::pulseToRange(uint16_t pulseUs, int outMin, int outMax) {
  if (pulseUs < RC_PULSE_MIN) pulseUs = RC_PULSE_MIN;
  if (pulseUs > RC_PULSE_MAX) pulseUs = RC_PULSE_MAX;

  long out = (long)(pulseUs - RC_PULSE_MIN) * (outMax - outMin)
             / (RC_PULSE_MAX - RC_PULSE_MIN) + outMin;
  return (int)out;
}

void Mixer::arcade(int throttle, int steer, int range, int &left, int &right) {
  int l = throttle + steer;
  int r = throttle - steer;

  // If either side overshoots the range, scale both down equally.
  int maxMag = max(abs(l), abs(r));
  if (maxMag > range && maxMag > 0) {
    l = (int)((long)l * range / maxMag);
    r = (int)((long)r * range / maxMag);
  }

  left  = l;
  right = r;
}
