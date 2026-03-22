# Azimuth

**Open, inexpensive PC head tracking**—built for sims, flight games, and anything that speaks [OpenTrack](https://github.com/opentrack/opentrack). The aim is the same one that drove the earlier **Nano 33 BLE** prototype in this workspace: **high-quality 6‑DOF head pose** without a four-figure price tag, with room to grow into calibration, companion software, and optional wireless paths.

This repository is a **clean reboot**: new hardware (ESP32-class MCU + external IMU), new firmware, and documentation that will accumulate here as the stack matures.

---

## Goals

- **Performance** — Low-latency, stable orientation for gaming and desktop use.
- **Accessibility** — Cheap, easy-to-source parts; open firmware and (eventually) tooling so people can build and modify their own trackers.
- **Interop** — First-class support for **OpenTrack** (and the same ecosystem of games and sims that already support standard head-tracking pipelines).
- **Evolvability** — Room for richer calibration, on-device UX, and PC-side apps without carrying legacy constraints from the first hardware generation.

The original attempt—**Arduino Nano 33 BLE**, onboard **LSM9DS1**, optional BLE + a Rust PC app, guided calibration via LEDs—is documented in [`../Nano33_PC_Head_Tracker/readme.md`](../Nano33_PC_Head_Tracker/readme.md). That project validated the product direction (wired Hatire, OpenTrack curves, wireless UDP, etc.). **Azimuth** keeps that direction while standardizing on **ESP32** and a **BNO08x** fusion IMU for this generation of hardware.

---

## What’s here now

| Area | Status |
|------|--------|
| **Firmware** | PlatformIO project for **Seeed XIAO ESP32-C3** + **BNO08x** over **SPI**: fused yaw / pitch / roll from the rotation-vector report. |
| **OpenTrack (USB)** | **Hatire Arduino**-compatible binary frames (`xiao_esp32c3_hatire` build)—same 30-byte layout used previously for USB head tracking. |
| **Debug** | Text telemetry over USB serial (`xiao_esp32c3` build). |
| **Hardware docs** | Pin-level wiring: **[docs/wiring.md](docs/wiring.md)**. |

Roadmap items (not implemented in this repo yet) may include companion flashing/calibration tools, wireless transport, enclosure guides, and deeper OpenTrack tuning notes—similar in spirit to the Nano project’s PC app and wireless workflow, rebuilt for this platform.

---

## Hardware (this revision)

- **MCU:** Seeed **XIAO ESP32-C3** (USB CDC serial, compact footprint).
- **IMU:** **BNO08x** breakout in **SPI** mode (**PS0** and **PS1** tied to **3.3 V** per the breakout datasheet).

Summary pin map (detail and notes in **[docs/wiring.md](docs/wiring.md)**):

| Signal | XIAO pin | GPIO |
|--------|----------|------|
| SCK | D8 | 8 |
| MISO | D9 | 9 |
| MOSI | D10 | 10 |
| CS | D2 | 4 |
| H_INT | D3 | 5 |
| RST | D7 | 20 |

---

## Building and flashing

Requires [PlatformIO](https://platformio.org/). Default environment: **`xiao_esp32c3`** (debug text).

**Debug (serial monitor, yaw/pitch/roll):**

```bash
python3 -m platformio run -e xiao_esp32c3 -t upload
python3 -m platformio device monitor
```

**OpenTrack (Hatire over USB, 115200 baud):**

```bash
python3 -m platformio run -e xiao_esp32c3_hatire -t upload
```

In OpenTrack: input **Hatire Arduino**, enable **DTR**, then start tracking and **recenter** after the filter settles. Hatire mode is binary-only—do not leave the serial monitor open while using OpenTrack.

---

## Firmware layout

- **`src/main.cpp`** — IMU bring-up, rotation vector, optional Hatire packing; adjust `kPinCs` / `kPinInt` / `kPinRst` if your wiring differs.
- **`platformio.ini`** — `espressif32`, `seeed_xiao_esp32c3`, **SparkFun BNO08x** library.

If you move SPI off the default D8–D10 pins, call `SPI.begin(sck, miso, mosi, -1)` **before** `imu.beginSPI(...)` so the bus matches your board (the SparkFun driver initializes `SPI` internally; ESP32 keeps an already-started bus).
