# Rover 6WD RC

Arduino firmware for a **6-wheel-drive rover** with a 4-wheel front bogie
suspension and a 2-wheel rear axle, driven by an RC transmitter.

- **Board:** Arduino Mega 2560
- **Motors:** 6 × DC gear motors
- **Drivers:** 3 × L298N dual H-bridge
- **Input:** RC receiver, standard servo PWM (works with your 10-channel set; the firmware uses 4 channels — steer, throttle, arm, speed)
- **Steering:** skid-steer (tank), arcade single-stick mixing
- **Dependencies:** none (Arduino core only)

```
        FRONT  (bogie suspension, 4 wheels)
        +------------------+
   M1 O |  FL1        FR1  | O M4
   M2 O |  FL2        FR2  | O M5
        |                  |
   M3 O |  RL          RR  | O M6
        +------------------+
        REAR  (2 wheels)

   LEFT side  = M1, M2, M3        RIGHT side = M4, M5, M6
```

The rover turns by running the left and right wheel groups at different
speeds. The mechanical bogie/rear split does not affect the control code — the
firmware only cares about the LEFT group and the RIGHT group.

---

## Features

- Interrupt-driven RC reading (non-blocking, no `pulseIn()` stalls)
- **Failsafe:** motors stop/brake automatically if the RC signal is lost
- **Arming:** a switch channel acts as a kill switch; the rover refuses to arm
  unless the throttle is centered (no lurch on power-up)
- **Speed limiting:** a knob channel sets a live maximum-speed ceiling
- **Acceleration (slew) limiting:** commands ramp smoothly to protect gearboxes
- Per-motor software direction inversion (fix a backwards wheel without rewiring)
- Clean, modular code split into reusable classes
- Serial telemetry for tuning/debugging

---

## Repository layout

```
rover-6wd-rc/
├── README.md
├── LICENSE
├── .gitignore
├── docs/
│   ├── PINOUT.md          # full pin assignment table
│   └── WIRING.md          # how to wire L298N boards, motors, receiver, power
└── RoverControl/          # Arduino sketch folder (open this in the IDE)
    ├── RoverControl.ino   # main program: control loop + arming + failsafe
    ├── Config.h           # ALL pins & tuning live here (edit this, not code)
    ├── MotorDriver.h/.cpp  # one L298N motor channel
    ├── Drivetrain.h/.cpp   # 6 motors -> LEFT/RIGHT sides + slew limiting
    ├── Mixer.h/.cpp        # RC pulse -> speed math + arcade mixing
    └── RCReceiver.h/.cpp   # interrupt-driven RC channel reader + failsafe
```

---

## Build & upload

### Arduino IDE
1. Install the Arduino IDE (1.8.x or 2.x).
2. Open `RoverControl/RoverControl.ino` (keep all files in that folder).
3. Select **Tools → Board → Arduino Mega or Mega 2560**.
4. Select the correct **Port**, then **Upload**.
5. Open **Serial Monitor** at `115200` baud to see telemetry.

### arduino-cli
```bash
arduino-cli core install arduino:avr
arduino-cli compile --fqbn arduino:avr:mega RoverControl
arduino-cli upload  --fqbn arduino:avr:mega -p <PORT> RoverControl
```

---

## Wiring (summary)

Full details, including power and grounding, are in
[docs/WIRING.md](docs/WIRING.md); the complete pin table is in
[docs/PINOUT.md](docs/PINOUT.md).

| RC receiver channel | Mega pin | Function           |
|---------------------|----------|--------------------|
| CH1 (right stick L/R) | 2      | Steering           |
| CH3 (left stick U/D)  | 3      | Throttle           |
| CH5 (2-pos switch)    | 18     | Arm / disarm       |
| CH6 (rotary knob)     | 19     | Max-speed limit    |

Each L298N channel uses `EN` (PWM), `IN1`, `IN2`:

| Motor | EN | IN1 | IN2 | L298N board |
|-------|----|-----|-----|-------------|
| Front-Left 1  (M1) | 5  | 22 | 23 | #1 (left bogie) |
| Front-Left 2  (M2) | 6  | 24 | 25 | #1 (left bogie) |
| Rear-Left     (M3) | 9  | 30 | 31 | #3 (rear)       |
| Front-Right 1 (M4) | 7  | 26 | 27 | #2 (right bogie)|
| Front-Right 2 (M5) | 8  | 28 | 29 | #2 (right bogie)|
| Rear-Right    (M6) | 10 | 32 | 33 | #3 (rear)       |

> **Common ground is mandatory:** the Mega GND, all three L298N GNDs and the
> motor-battery GND must be tied together. See the wiring guide.

---

## First run / tuning checklist

1. **Wheels up!** Prop the rover so the wheels are off the ground for the first test.
2. Power the logic (USB) and motor battery, turn on the transmitter.
3. Open Serial Monitor. With sticks centered you should see `thrUs`/`strUs`
   near `1500` and `FS:0`.
4. Flip the arm switch — `ARM` should change to `1` only when throttle is centered.
5. Nudge throttle: confirm all wheels drive the rover *forward*. For any wheel
   spinning the wrong way, flip its entry in `MOTOR_INVERT[]` in `Config.h`.
6. Nudge steering: confirm the rover yaws the correct way.
7. If a stick is reversed, swap that channel's `RC_PULSE_MIN`/`MAX` meaning by
   reversing the channel on your transmitter (preferred), or adjust in code.
8. Tune `SLEW_RATE_PER_SEC`, `SPEED_LIMIT_*`, and `RC_DEADBAND` to taste.

---

## Safety notes

- The firmware **brakes on signal loss** (`BRAKE_ON_FAILSAFE` in `Config.h`).
- Keep `USE_ARM_SWITCH = 1` so you always have a physical kill switch.
- Never bench-test at full speed with the rover on the ground untethered.
- L298N boards drop ~2 V and get hot under load; size your motor battery and
  heatsinking accordingly. For higher-current motors consider better drivers.

---

## License

MIT — see [LICENSE](LICENSE).
