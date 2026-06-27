# Wiring guide

This describes how to wire the Arduino Mega 2560, three L298N boards, six DC
motors, the RC receiver, and power. Pin numbers come from
[`Config.h`](../RoverControl/Config.h) — see [PINOUT.md](PINOUT.md) for the table.

> ⚠️ **Read the safety section at the bottom before powering up.**

---

## 1. Power architecture

There are two power domains:

- **Logic power** — the Arduino Mega (via USB or a regulated 5 V on `5V`/`VIN`).
- **Motor power** — the battery that feeds the L298N `+12V` (motor supply)
  terminals and the motors.

```
 Motor battery (+) ──┬── L298N #1  +12V
                     ├── L298N #2  +12V
                     └── L298N #3  +12V

 Motor battery (−) ──┬── L298N #1  GND ──┐
                     ├── L298N #2  GND ──┤
                     └── L298N #3  GND ──┤
                                         ├── Arduino Mega GND   (COMMON GROUND)
 Arduino USB / 5V  ───────── Mega 5V ────┘
```

**Common ground is mandatory.** The Mega, all three L298N boards, the RC
receiver, and the motor battery negative must share a ground. Without it the
PWM/direction signals have no reference and motors behave erratically.

### The L298N 5V jumper
- Each L298N has a `5V-EN` jumper next to a 5 V regulator.
- **If your motor supply is ≤ 12 V:** you may leave the jumper ON, and the
  board's onboard regulator produces 5 V on its `5V` pin. Do **not** also feed
  5 V from the Mega into that pin in this case.
- **Simplest reliable setup:** remove the jumper and power the Mega separately
  (USB or its own 5 V), with only the common ground shared. This repo assumes
  the Mega is powered independently.

---

## 2. L298N → motor / signal connections

Each L298N drives **two** motors (channel A and channel B). You have three
boards for six motors.

For each channel:

| L298N terminal | Connect to                         |
|----------------|-------------------------------------|
| `ENA` / `ENB`  | the motor's `EN` (PWM) pin on Mega  |
| `IN1` / `IN2`  | the motor's `IN1` pin on Mega       |
| `IN3` / `IN4`  | the motor's `IN2` pin on Mega       |
| `OUT1`/`OUT2`  | motor A terminals                   |
| `OUT3`/`OUT4`  | motor B terminals                   |

> The board labels `IN1/IN2` for channel A and `IN3/IN4` for channel B. In this
> firmware every motor just has an `IN1` and `IN2` — map them to whichever
> board input pair drives that motor.

### Board → motor mapping used by `Config.h`

```
L298N #1 (LEFT bogie)    chA = Front-Left 1  (EN 5,  IN1 22, IN2 23)
                         chB = Front-Left 2  (EN 6,  IN1 24, IN2 25)

L298N #2 (RIGHT bogie)   chA = Front-Right 1 (EN 7,  IN1 26, IN2 27)
                         chB = Front-Right 2 (EN 8,  IN1 28, IN2 29)

L298N #3 (REAR axle)     chA = Rear-Left     (EN 9,  IN1 30, IN2 31)
                         chB = Rear-Right    (EN 10, IN1 32, IN2 33)
```

> **Remove the ENA/ENB jumpers** on the L298N. Those jumpers tie EN to 5 V
> (always full speed). You need EN connected to the Mega's PWM pin instead.

### Fixing a backwards wheel
After a bench test, if a wheel spins the wrong way for "forward", either swap
that motor's two `OUT` wires, **or** flip its entry in `MOTOR_INVERT[]` in
`Config.h` and re-upload. No rewiring needed for the software fix.

---

## 3. RC receiver

Connect the signal pin of each used channel to the Mega interrupt pins:

| Receiver channel | Signal → Mega pin |
|------------------|-------------------|
| CH1 (steer)      | 2                 |
| CH3 (throttle)   | 3                 |
| CH5 (arm switch) | 18                |
| CH6 (speed knob) | 19                |

- Power the receiver from a clean 5 V (the Mega `5V` pin is fine for a typical
  receiver) and share ground.
- Only the **signal** wire of each channel goes to the Mega; the receiver's
  `+`/`−` come from the 5 V/GND rail (do not back-feed multiple 5 V sources).

---

## 4. Recommended extras

- **Motor decoupling capacitors:** solder a 0.1 µF ceramic cap across each
  motor's terminals (and optionally to the can) to cut brush noise that can
  glitch the logic / RC signals.
- **Bulk capacitor:** a 470–1000 µF electrolytic across the motor supply at the
  L298N inputs steadies voltage under current spikes.
- **Heatsinks:** the L298N runs hot; keep the supplied heatsinks on and ensure
  airflow.

---

## 5. Safety

1. **First power-up with wheels off the ground.** Prop the chassis up.
2. Verify `FS:0` and centered `thrUs`/`strUs` (~1500) on the Serial Monitor
   before arming.
3. Keep the arm switch (`USE_ARM_SWITCH = 1`) as a kill switch at all times.
4. Disconnect the **motor battery** first when finishing, logic last.
5. The L298N is fine for small/medium DC motors but is lossy and current
   limited. For high-current motors, move to dedicated drivers (e.g. BTS7960,
   Cytron, or a VESC/ODrive for BLDC) — the firmware's per-motor abstraction
   makes that swap localized to `MotorDriver`.
