<div align="center">

<img src="logo/AzimuthLogo_Dark.png#gh-light-mode-only" alt="Azimuth" width="240">
<img src="logo/AzimuthLogo_Light.png#gh-dark-mode-only" alt="Azimuth" width="240">

</div>

# Azimuth

Azimuth enables high-quality PC head tracking for sims, flight games, and anything that supports [OpenTrack](https://github.com/opentrack/opentrack).

It is a cheap but powerful tracker you can build yourself: a small ESP32-based board and an advanced IMU that gives stable head orientation, with open hardware and firmware so you can build, fix, or change the project instead of relying on a closed commercial device.

This repository holds firmware, PCB designs, 3D print designs, and documentation.

<div align="center">
<table>
  <tr>
    <td align="center"><img src="img/BoardTop.png" alt="PCB top" width="380"></td>
    <td align="center"><img src="img/BoardBottom.png" alt="PCB bottom" width="380"></td>
  </tr>
</table>
</div>

### Development status (toward V1)

| Area | Progress | Notes |
|------|:--------:|--------|
| Hardware / BOM | 100% | Parts chosen ([docs/parts-list.md](docs/parts-list.md)). |
| Custom PCB | ~95% | KiCad aligned with [docs/wiring.md](docs/wiring.md); **panelization** (fab-ready panel) is the remaining PCB task before ordering. |
| Firmware | ~30% | SPI IMU, USB debug, **Hatire + WiFi → OpenTrack UDP** (see [Building and flashing](#building-and-flashing)); `include/secrets.h` for WiFi/host. **Board I/O** (LED, button, buzzer), **battery/ADC**, and **on-device settings** still ahead ([roadmap](docs/roadmap.md)). |
| 3D enclosure | 0% | Not started. Plan: **battery-sized** shell first; optional slimmer **wired-only** enclosure if PH2 is omitted on those builds ([roadmap](docs/roadmap.md)). |

```
Hardware/BOM   [████████████████████] 100%
PCB            [███████████████████░] ~95%
Firmware       [██████░░░░░░░░░░░░░░] ~30%
Enclosure      [░░░░░░░░░░░░░░░░░░░░] 0%
```

**Roadmap & checklists:** [docs/roadmap.md](docs/roadmap.md)

---

## Goals

- **Performance** — Low-latency, stable orientation for gaming and desktop use.
- **Accessibility** — Cheap, easy-to-source parts; open firmware and tooling so people can build and modify their own trackers.
- **Interop** — First-class support for **OpenTrack** (and the same ecosystem of games and sims that already support standard head-tracking pipelines).
- **Evolvability** — Room for richer calibration, on-device UX, and PC-side apps without dragging forward the old platform’s constraints.
- **Lineage** — Successor to **[Nano33_PC_Head_Tracker](https://github.com/FugLong/Nano33_PC_Head_Tracker)**: same broad idea—DIY, affordable, OpenTrack-first—but the old **Nano 33 BLE** + onboard **LSM9DS1** stack topped out; fusion tuning couldn’t pull enough quality out of that hardware and software. **Azimuth** starts over on **ESP32** and an external **BNO08x**-class IMU, with new firmware and no legacy code path.

---

## What’s here now

| Area | Status |
|------|--------|
| **Firmware** | PlatformIO project for **Seeed XIAO ESP32-C3** + **BNO08x** over **SPI**: fused yaw / pitch / roll from the rotation-vector report. |
| **OpenTrack** | `xiao_esp32c3_hatire`: **Hatire Arduino** over USB (30-byte frames) **and** optional **WiFi → UDP** to the PC using OpenTrack’s **UDP over network** input (6× `double`, port 4242 by default). |
| **Debug** | Text telemetry over USB serial (`xiao_esp32c3` build). |
| **Hardware docs** | **[docs/wiring.md](docs/wiring.md)** (signals, power, GPIO map) · **[docs/parts-list.md](docs/parts-list.md)** (BOM + passives notes) · **[docs/kicad.md](docs/kicad.md)** (custom KiCad libs + collaboration). |

Planned work (board I/O, battery, web flashing / settings UX, enclosure, richer calibration) is tracked in **[docs/roadmap.md](docs/roadmap.md)**.

---

## Hardware (this revision)

- **MCU:** Seeed **XIAO ESP32-C3** (USB CDC serial, compact footprint).
- **IMU:** **BNO086** on the custom PCB (or **BNO08x** breakout in **SPI** mode with **PS0** / **PS1** → **3.3 V**).

Summary SPI / control pin map (full table, battery, buttons, LED, buzzer in **[docs/wiring.md](docs/wiring.md)**; BOM in **[docs/parts-list.md](docs/parts-list.md)**):

| Signal | XIAO pin | GPIO |
|--------|----------|------|
| SCK | D8 | 8 |
| MISO | D9 | 9 |
| MOSI | D10 | 10 |
| CS | D3 | 5 |
| H_INT | D4 | 6 |
| PS0 / WAKE | D2 | 4 |
| NRST | D7 | 20 |

---

## Building and flashing

Requires [PlatformIO](https://platformio.org/). Default environment: **`xiao_esp32c3`** (debug text).

**Debug (serial monitor, yaw/pitch/roll):**

```bash
python3 -m platformio run -e xiao_esp32c3 -t upload
python3 -m platformio device monitor
```

**OpenTrack — Hatire (USB) + optional UDP (WiFi):**

```bash
python3 -m platformio run -e xiao_esp32c3_hatire -t upload
```

Copy **`include/secrets.h.example`** to **`include/secrets.h`** and set **`WIFI_SSID`**, **`WIFI_PASSWORD`**, and **`OPENTRACK_UDP_HOST`** (your PC’s LAN IP). `secrets.h` is **gitignored** so credentials are not committed. Leave **`WIFI_SSID` empty (`""`)** for USB-only (no WiFi). UDP port **`OPENTRACK_UDP_PORT`** is set in **`platformio.ini`** (default **4242**, OpenTrack’s usual UDP input port).

- **USB:** Input **Hatire Arduino**, **115200**, **DTR** on; start tracking and **recenter** after the filter settles. Do not leave a text serial monitor open on that port.
- **WiFi / UDP:** Input **UDP over network**, same port as in **`platformio.ini`** (`OPENTRACK_UDP_PORT`); allow the port through the PC firewall. Hatire and UDP both run from the same firmware.

---

## Firmware layout

- **`src/main.cpp`** — IMU bring-up, rotation vector, Hatire + optional OpenTrack UDP; `kPinCs` / `kPinInt` / `kPinRst` match the **ESP32_BNO086** PCB (see [docs/wiring.md](docs/wiring.md)).
- **`platformio.ini`** — `espressif32`, `seeed_xiao_esp32c3`, **SparkFun BNO08x** library.
- **`include/secrets.h`** — local WiFi + OpenTrack host (copy from `secrets.h.example`; not tracked by git).

If you move SPI off the default D8–D10 pins, call `SPI.begin(sck, miso, mosi, -1)` **before** `imu.beginSPI(...)` so the bus matches your board (the SparkFun driver initializes `SPI` internally; ESP32 keeps an already-started bus).
