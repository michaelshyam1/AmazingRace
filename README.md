# The A-maze-ing Race — CG1111A Engineering Principles & Practice I

> An autonomous maze-solving robot built on the mBot platform, capable of navigating a maze using ultrasonic and IR proximity sensors, detecting coloured waypoint challenges, and executing precise turns — all without bumping into walls.

---

## 📋 Project Overview

This project was completed as part of **CG1111A Engineering Principles & Practice I** at NUS. The mBot must autonomously navigate an unknown maze, stop at waypoint challenges encoded as coloured paper, decode the correct turn direction, and celebrate upon reaching the finish line.

---

## ✨ Features

- **Wall-following navigation** using a Makeblock ultrasonic sensor (one side) and a custom-built IR proximity sensor (other side)
- **Straight-line correction** algorithm to prevent zig-zag driving between waypoints
- **Colour sensing** via a custom LDR + RGB LED circuit mounted beneath the mBot, shielded from ambient light
- **Waypoint challenge decoding** — identifies 5 colours and executes the corresponding turn:

  | Colour     | Action                              |
  |------------|-------------------------------------|
  | Red        | Left turn                           |
  | Green      | Right turn                          |
  | Orange     | 180° U-turn within the same grid    |
  | Pink       | Two successive left turns (2 grids) |
  | Light Blue | Two successive right turns (2 grids)|
  | White      | End of maze — stop & play tune 🎵   |

- **Ambient IR compensation** — IR emitter toggled on/off to establish a baseline voltage, making wall detection robust across varying lighting conditions
- **Celebratory tune** played upon reaching the maze end

---

## 🔧 Hardware Components

| Component | Purpose |
|-----------|---------|
| mBot (ATmega328 / mCore) | Main robotic platform & microcontroller |
| Makeblock Ultrasonic Sensor | Side wall distance measurement (one side) |
| Makeblock Line Sensor | Black strip detection at waypoints |
| Custom IR Proximity Sensor (LTE-302 emitter + LTR-301 detector) | Side wall proximity (other side) |
| HD74LS139P 2-to-4 Decoder IC | Multiplexes 4 outputs (R/G/B LEDs + IR emitter) using only 2 signal pins |
| L293D Motor Driver IC | Drives IR emitter at up to 50 mA (bypasses decoder's 8 mA limit) |
| Light Dependent Resistor (LDR) | Senses reflected colour from paper below mBot |
| Red, Green, Blue LEDs | Illuminate colour paper for LDR to read |
| 170-wire Mini-breadboard | Houses custom sensor circuits |
| RJ25 Adapters | Interfaces custom circuits to mCore signal pins |

---

## ⚙️ Key Algorithms

### Wall Following & Straight-Line Correction
The mBot continuously reads both the ultrasonic sensor and IR proximity sensor. Motor speeds are adjusted proportionally when the robot drifts too close to or too far from either wall, keeping it centred in the corridor.

### Ambient IR Compensation
The IR emitter is toggled off at regular intervals to capture a **baseline voltage** (ambient IR only). When the emitter is on, the **dip** from this baseline indicates wall proximity — making readings robust even as lighting conditions change across different parts of the maze.

### Colour Sensing
With the decoder cycling through R → G → B LEDs, the LDR voltage is sampled under each colour. The three readings are compared against calibrated thresholds to classify the paper colour. A black paper chimney surrounds the circuit to eliminate ambient light interference.

### Turn Execution
Turns are time/encoder-based to achieve consistent 90° and 180° rotations. For double-turn challenges (pink/light blue), the second turn is hard-coded to trigger automatically after the first completes, with no black strip present in the second grid.

---

## 🛠️ Setup & Upload

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) and the [Makeblock library](https://github.com/Makeblock-official/Makeblock-Libraries)
2. Connect the mBot to your computer via USB
3. Open `src/amazeing_race.ino` in Arduino IDE
4. Select **Board: Arduino Uno** and the correct COM port
5. Upload the sketch

---

## 📐 Circuit Notes

- Ultrasonic sensor → RJ25 Port 1 or 2 (digital pins only)
- Line sensor → RJ25 Port 1 or 2 (digital pins only)
- LDR voltage output → RJ25 Port 3 or 4 (analog pin)
- IR detector voltage output → RJ25 Port 3 or 4 (analog pin)
- Decoder A/B select inputs → two remaining signal pins from Port 3/4
- LED current-limiting resistors sized to keep decoder output current **< 8 mA**
- IR emitter resistor sized for **≤ 50 mA** continuous current through L293D

## 📄 License

This project was submitted for academic assessment at NUS. Code and designs are shared for portfolio purposes only — please do not copy for your own coursework submission.
