<div align="center">

# Mold Verification and Safety Interlock System

### *A Safer Process Today, A Smarter Factory Tomorrow.*

A **simulation-based industrial safety prototype** that verifies correct mold seating at three critical points before permitting a molding machine to start — replacing manual, operator-dependent checks with an automated, sensor-driven start-permit interlock.

Built and validated entirely in **Proteus 8 Professional**, using an **Arduino Uno** as the controller.

---

![Arduino](https://img.shields.io/badge/ARDUINO-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![Proteus](https://img.shields.io/badge/PROTEUS%208%20PROFESSIONAL-E4A11B?style=for-the-badge)
![Simulation](https://img.shields.io/badge/SIMULATION%20BASED-4CAF50?style=for-the-badge)
![Safety Interlock](https://img.shields.io/badge/SAFETY%20INTERLOCK-2196F3?style=for-the-badge)
![Event Logging](https://img.shields.io/badge/EVENT%20LOGGING-5A0FC8?style=for-the-badge)
![Status](https://img.shields.io/badge/STATUS-SIMULATION%20VALIDATED-BA7517?style=for-the-badge)
![License](https://img.shields.io/badge/LICENSE-MIT-8BC34A?style=for-the-badge)

---

⭐ **If you find this project useful, consider giving it a star!**

</div>

---

## 📖 Table of Contents

- [Overview](#overview)
- [Project Status](#project-status)
- [The Problem](#the-problem)
- [The Solution](#the-solution)
- [Key Features](#key-features)
- [Engineering Principle](#engineering-principle)
- [System Architecture](#system-architecture)
- [Components Used](#components-used)
- [Circuit & Wiring](#circuit--wiring)
- [Logic Flow](#logic-flow)
- [Simulation Test Results](#simulation-test-results)
- [Technologies Used](#technologies-used)
- [Repository Structure](#repository-structure)
- [Running the Simulation](#running-the-simulation)
- [Honest Limitations & Future Scope](#honest-limitations--future-scope)
- [Author](#author)
- [License](#license)

---

## Overview

Before a molding machine begins its cycle, the mold must be correctly and fully seated on the machine bed. This project automates that verification using three proximity sensors placed at three non-collinear points, combined with an Arduino-based safety interlock, an LCD status/event display, and a relay-driven start-permit output — all built and tested through Hardware-in-the-Loop-style simulation, with no physical hardware required.

## Project Status

This project is currently at the **simulation and design-validation stage**. All logic, wiring, and safety behavior described below have been built and verified in Proteus 8 Professional using Hardware-in-the-Loop-style simulation — no physical hardware has been assembled yet. Simulation was chosen deliberately as the first step: it allows the interlock logic, debounce handling, and fault-recovery behavior to be fully validated before committing to physical components. Physical prototyping is the planned next phase (see [Honest Limitations & Future Scope](#honest-limitations--future-scope)).

## The Problem

In many basic setups, mold seating is verified visually by a human operator — slow and error-prone. An incorrectly seated mold (tilted, lifted, or not fully pushed in) can result in:
- Defective products (uneven shape, weak spots)
- Damage to the mold or machine
- Wasted material, time, and production downtime

## The Solution

Three sensors — **Front-Left (FL)**, **Front-Right (FR)**, and **Rear-Centre (RC)** — continuously verify mold contact at three points. The machine is only permitted to start once **all three** confirm correct seating, and a fault at any single point immediately blocks the start-permit relay and alerts the operator with an LED, buzzer, and named-sensor LCD message.

## Key Features

- 🔺 **Three-point verification** — confirms the mold is flat and fully seated, not just present
- 🔒 **Safety start-permit interlock** — AND-logic across all three sensors, the same principle used in real industrial machine safety circuits
- 🚫 **No silent auto-restart** — once a fault occurs, the system stays locked out even after it clears, until the operator gives a fresh, deliberate Start action
- 🎯 **Named-sensor fault reporting** — the LCD identifies exactly which sensor (Front-Left / Front-Right / Rear-Centre) failed, not just a generic fault
- 📋 **Event logging with timestamps** — every state change (Machine Started, Fault, Fault Cleared, Machine Stopped) is logged with an elapsed-time timestamp, visible on-screen and via serial output
- 🧹 **Debounce filtering** — every input (sensors and start switch) is read twice with a short delay and only trusted if both readings agree, filtering out electrical noise and switch bounce
- 🖥️ **20×4 character LCD status display** — shows live system state plus the two most recent logged events

## Engineering Principle

This project demonstrates a **safety start-permit interlock**: a machine is only allowed to start when *every* required safety condition is verified true at the same time (AND-logic) — the same core principle used in real industrial machine safety circuits, applied here to mold seating verification.

## System Architecture

```
Sensing                Processing                  Output
────────                ──────────                  ──────
FL sensor  ─┐                                    ┌─▶ LCD (status + event log)
FR sensor  ─┼──▶  Arduino Uno  ──▶  AND-logic ──┼─▶ Green / Red LED
RC sensor  ─┘     (debounce +     interlock      ├─▶ Buzzer
Start switch ─────  edge detect)                 └─▶ Relay (start-permit, w/ flyback diode)
```

## Components Used

| Component | Role |
|---|---|
| Arduino Uno | Reads sensors/switch, runs interlock logic, drives outputs |
| 3× push-button (simulating proximity sensors) | Represent FL / FR / RC mold-seating detection points |
| 3× 10kΩ resistor | Pull-down resistors for sensor inputs |
| 1× push-button (Start/Stop switch) | Operator's deliberate start action |
| 1× 10kΩ resistor | Pull-down for the start switch |
| Green LED + 220Ω resistor | "Ready / Running" indicator |
| Red LED + 220Ω resistor | "Fault" indicator |
| Buzzer | Audible fault alarm |
| Relay (12V) | Represents the machine's start-permit circuit |
| 1N4007 diode | Flyback protection across the relay coil |
| 20×4 LCD (LM044L) | Displays live status and recent event log |
| 2kΩ resistor | LCD contrast |

**Note:** in a real physical panel, the relay coil would be driven through an NPN transistor stage rather than directly from a microcontroller pin. This was simplified for simulation but is the technically correct real-world approach.

## Circuit & Wiring
<img width="1984" height="888" alt="Circuit" src="https://github.com/user-attachments/assets/bcc75070-bdbc-45a2-9adc-c5467fec1a8a" />


| Signal | Arduino Pin |
|---|---|
| Front-Left sensor | D2 |
| Front-Right sensor | D3 |
| Rear-Centre sensor | D4 |
| Start/Stop switch | D6 |
| Relay (start-permit) | D7 |
| Green LED | D8 |
| Red LED | D9 |
| Buzzer | D10 |
| LCD (RS, E, D4–D7) | A0, A1, A2, A3, A4, A5 |

## Logic Flow

| Condition | Result |
|---|---|
| All 3 sensors OK + fresh Start press | Ready / Running — green LED, relay closes |
| Any sensor fails | Fault — red LED, buzzer, relay blocked, named on LCD, latched |
| Sensors OK, but fault just cleared, no fresh Start press | Still locked — "Fault Cleared, Toggle Switch" |
| Sensors OK, switch OFF | Waiting — operator hasn't started yet |

## Simulation Test Results

Real, recorded results from Proteus simulation trials (Front-Left and Front-Right individually tested; Rear-Centre shares identical logic and was not separately tested):

| Time | Event | Verified behaviour |
|---|---|---|
| 00:01 | Machine started | Normal start |
| 00:11 | Front-Left fault | Correct sensor named, machine stopped |
| 00:16 | Fault cleared | Stayed locked, did not resume |
| 00:34 | Machine started | Deliberate restart, 18s gap after clear |
| 00:47 | Front-Right fault | Correct sensor named, machine stopped |
| 01:01 | Fault cleared | Stayed locked, did not resume |
| 01:11 | Machine started | Deliberate restart, 10s gap after clear |

Both tested fault scenarios confirmed: (1) accurate per-sensor fault identification, and (2) zero silent auto-restarts — the system always required a deliberate operator action to resume after a fault cleared.

## Technologies Used

| Layer | Technology |
|---|---|
| Simulation | Proteus 8 Professional |
| Firmware | Arduino Uno (C++/Arduino IDE) |
| Display | 20×4 character LCD (HD44780-compatible, `LiquidCrystal` library) |
| Logic | Debounced digital I/O, AND-based safety interlock, circular-buffer event log |

## Repository Structure

```
mold-verification-safety-interlock/
├── Mold_Seating.ino
├── Mold Verification & Safety Interlock System.pdsprj
├── Mold Verification & Safety Interlock System Presentation.pdf
└── README.md
```

## Running the Simulation

```bash
# 1. Open the .pdsprj file in Proteus 8 Professional
# 2. Compile Mold_Seating.ino in Arduino IDE (Sketch → Export Compiled Binary)
# 3. Load the generated .hex file into the Arduino Uno component in Proteus
# 4. Run the simulation and toggle the sensor/switch buttons to test each state
```

## Honest Limitations & Future Scope

- No PLC or VFD experience applied yet — Arduino and relay logic were used as the accessible equivalent; the underlying AND-based interlock logic is identical to what a PLC would implement
- Relay coil driven directly from a microcontroller pin in simulation; a real panel would use a transistor driver stage
- Rear-Centre sensor fault path shares identical code with Front-Left/Front-Right but was not independently trial-tested
- No real-time clock — event timestamps are elapsed time since power-on, not wall-clock time
- **Planned next steps:** PLC-based start-permit integration, transistor-driven relay stage, VFD/servo-controlled motor stage, real-time clock for wall-clock event logging

## Author

**Rahul Sahu**
Final-year B.Tech, Electrical and Electronics Engineering
Aditya College of Engineering & Technology, Surampalem

## License

This project is licensed under the **MIT License**.

---

<div align="center">
Ensuring Safety. Enhancing Reliability. Building a Smarter Tomorrow.
</div>
