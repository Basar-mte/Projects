# Pin assignment — Arduino Mega 2560

All pins are defined in [`RoverControl/Config.h`](../RoverControl/Config.h).
Change them there; this table is documentation only.

## Motor pins (3 × L298N)

Each motor channel uses one PWM `EN` pin and two direction pins (`IN1`, `IN2`).

| Motor | Index | EN (PWM) | IN1 | IN2 | L298N board | Side  |
|-------|-------|----------|-----|-----|-------------|-------|
| Front-Left 1  | `M_FRONT_LEFT_1`  | 5  | 22 | 23 | #1 | LEFT  |
| Front-Left 2  | `M_FRONT_LEFT_2`  | 6  | 24 | 25 | #1 | LEFT  |
| Rear-Left     | `M_REAR_LEFT`     | 9  | 30 | 31 | #3 | LEFT  |
| Front-Right 1 | `M_FRONT_RIGHT_1` | 7  | 26 | 27 | #2 | RIGHT |
| Front-Right 2 | `M_FRONT_RIGHT_2` | 8  | 28 | 29 | #2 | RIGHT |
| Rear-Right    | `M_REAR_RIGHT`    | 10 | 32 | 33 | #3 | RIGHT |

### Why these PWM pins?
On the Mega 2560, the `EN` pins (5, 6, 7, 8, 9, 10) are driven by Timer2/3/4
(pin 5 → Timer3; pins 6/7/8 → Timer4; pins 9/10 → Timer2). Pins **4** and **13**
were intentionally avoided because they share **Timer0**, which runs `millis()`
/ `micros()` / `delay()`. Using them for PWM is possible but risks disturbing
timing if the PWM frequency is changed. (Timer1 — pins 11/12 — is left free.)

## RC receiver pins

Standard servo PWM (1000–2000 µs), one wire per channel. These pins **must**
be external-interrupt-capable on the Mega: `2, 3, 18, 19, 20, 21`.

| Channel    | Index         | Mega pin | Typical Tx source        |
|------------|---------------|----------|--------------------------|
| Steering   | `RC_STEER`    | 2        | CH1 — right stick L/R    |
| Throttle   | `RC_THROTTLE` | 3        | CH3 — left stick U/D     |
| Arm switch | `RC_ARM`      | 18       | CH5 — 2-position switch  |
| Speed knob | `RC_SPEED`    | 19       | CH6 — rotary knob / VrA  |

> Pins 18 & 19 are also `Serial1` (TX1/RX1). That is fine here because the
> sketch does not use `Serial1`. Pins 20 & 21 are `SDA`/`SCL`; only use them
> for extra channels if you are not using I2C.

## Reserved / in use by the core

| Pin(s) | Use                         |
|--------|-----------------------------|
| 0, 1   | USB Serial (Serial Monitor) |
| 13     | On-board LED                |

## Adding more channels
1. Pick another interrupt pin (e.g. 20 or 21) and add it to `RC_PIN[]`.
2. Increase `RC_CHANNEL_COUNT` and add a matching enum entry in `Config.h`.
3. In `RCReceiver.cpp` add a `RC_DEFINE_ISR(n)` and a table entry, then update
   the `static_assert`. (The build will fail with a clear message until you do.)
