#include "RCReceiver.h"
#include "Config.h"

// All shared state is volatile because it is written inside ISRs and read
// from the main loop. 16/32-bit reads are NOT atomic on the 8-bit AVR, so the
// accessors below copy them with interrupts briefly disabled.
namespace {

  volatile uint16_t s_pulse[RC_CHANNEL_COUNT];      // last valid width (us)
  volatile uint32_t s_rise[RC_CHANNEL_COUNT];       // micros() at rising edge
  volatile uint32_t s_lastEdgeMs[RC_CHANNEL_COUNT]; // millis() of last valid pulse
  volatile bool     s_seen[RC_CHANNEL_COUNT];       // a valid pulse received yet?

  // One CHANGE-triggered ISR per channel. On a rising edge we timestamp the
  // start; on a falling edge we compute the width and accept it only if it is
  // within the valid window (filters glitches / partial pulses).
  //
  // s_seen is a dedicated "have we ever received a pulse" flag rather than
  // overloading s_lastEdgeMs == 0, because millis() legitimately returns 0 for
  // the first millisecond after boot and once per ~49.7-day rollover - using 0
  // as a sentinel would misread a real pulse landing in that window as lost.
  #define RC_DEFINE_ISR(ch)                                        \
    void rcISR##ch() {                                             \
      if (digitalRead(RC_PIN[ch])) {                               \
        s_rise[ch] = micros();                                     \
      } else {                                                     \
        uint32_t w = micros() - s_rise[ch];                        \
        if (w >= RC_PULSE_VALID_MIN && w <= RC_PULSE_VALID_MAX) {  \
          s_pulse[ch]      = (uint16_t)w;                          \
          s_lastEdgeMs[ch] = millis();                             \
          s_seen[ch]       = true;                                 \
        }                                                          \
      }                                                            \
    }

  RC_DEFINE_ISR(0)
  RC_DEFINE_ISR(1)
  RC_DEFINE_ISR(2)
  RC_DEFINE_ISR(3)

  typedef void (*isr_t)();
  isr_t s_isr[RC_CHANNEL_COUNT] = { rcISR0, rcISR1, rcISR2, rcISR3 };

  // If you change RC_CHANNEL_COUNT, add matching RC_DEFINE_ISR()/table entries.
  static_assert(RC_CHANNEL_COUNT == 4,
    "RCReceiver provides 4 channel ISRs; add more if RC_CHANNEL_COUNT changes.");

}  // namespace

void RC::begin() {
  for (uint8_t ch = 0; ch < RC_CHANNEL_COUNT; ch++) {
    // Pull-up keeps the line defined (HIGH, no edges -> failsafe) if the
    // receiver is disconnected.
    pinMode(RC_PIN[ch], INPUT_PULLUP);
    s_pulse[ch]      = RC_PULSE_CENTER;  // safe neutral until first real pulse
    s_rise[ch]       = 0;
    s_lastEdgeMs[ch] = 0;
    s_seen[ch]       = false;            // not seen yet -> failsafe until first pulse
    attachInterrupt(digitalPinToInterrupt(RC_PIN[ch]), s_isr[ch], CHANGE);
  }
}

uint16_t RC::pulse(uint8_t channel) {
  if (channel >= RC_CHANNEL_COUNT) return RC_PULSE_CENTER;
  uint16_t v;
  noInterrupts();
  v = s_pulse[channel];
  interrupts();
  return v;
}

unsigned long RC::age(uint8_t channel) {
  if (channel >= RC_CHANNEL_COUNT) return 0xFFFFFFFFUL;
  uint32_t last;
  bool     seen;
  noInterrupts();
  seen = s_seen[channel];
  last = s_lastEdgeMs[channel];
  interrupts();
  if (!seen) return 0xFFFFFFFFUL;      // never received a valid pulse
  return millis() - last;              // unsigned subtraction is rollover-safe
}

bool RC::failsafe() {
  if (RC::age(RC_STEER)    > RC_TIMEOUT_MS) return true;
  if (RC::age(RC_THROTTLE) > RC_TIMEOUT_MS) return true;
  return false;
}
