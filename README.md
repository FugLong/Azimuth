<div align="center">

<img src="logo/AzimuthLogo_Dark.png#gh-light-mode-only" alt="Azimuth" width="240">
<img src="logo/AzimuthLogo_Light.png#gh-dark-mode-only" alt="Azimuth" width="240">

</div>

# Azimuth

Azimuth is a **DIY head tracker** for flight sims, racing, and anything that works with [OpenTrack](https://github.com/opentrack/opentrack). It pairs a small **ESP32** board with a **BNO08x-class** IMU so you get stable orientation without a closed commercial device. This repo has **firmware**, **PCB** designs, **3D** plans, and **documentation**—build it, change it, or fix it yourself.

---

### Get started

| | |
|:---|:---|
| **Install or update firmware (USB)** | [**Azimuth web flasher**](https://fuglong.github.io/Azimuth/) — use **Chrome** or **Edge** and a **data** USB cable. |
| **Settings (Wi‑Fi, OpenTrack, device)** | [**http://azimuth.local:8080**](http://azimuth.local:8080) — only after the board is on your home network. First time? Connect to **Azimuth-Setup** and follow the [**quick start**](docs/quickstart.md). |
| **Step-by-step: Wi‑Fi → OpenTrack** | [**docs/quickstart.md**](docs/quickstart.md) |

---

<div align="center">
<table>
  <tr>
    <td align="center"><img src="img/BoardTop.png" alt="PCB top" width="380"></td>
    <td align="center"><img src="img/BoardBottom.png" alt="PCB bottom" width="380"></td>
  </tr>
</table>
</div>

## Overview

- **OpenTrack-first** — Send motion over **Wi‑Fi (UDP)** or **USB (Hatire)**; configure most things in the on-device web UI.
- **Affordable, open stack** — Sensible parts list, KiCad project, and firmware you can build with [PlatformIO](https://platformio.org/).
- **No cloud** — No account or hosted service; setup and tracking stay on your network.
- **Successor** to [Nano33_PC_Head_Tracker](https://github.com/FugLong/Nano33_PC_Head_Tracker): same DIY spirit, but rebuilt around **ESP32** and an external fusion IMU for better results than the old Nano 33 BLE + LSM9DS1 path.

## Need to know

- **Flashing** — Browser install needs **Chrome** or **Edge** (Web Serial). If the installer offers **erase flash**, use it for a clean device (same effect as a full settings reset).
- **First Wi‑Fi setup** — Join **Azimuth-Setup**, open **`http://192.168.4.1`**, save your home network. Details: [**quick start**](docs/quickstart.md).
- **OpenTrack** — Use **either** UDP **or** Hatire as the input, not both at once. Defaults and axis mapping: [**Using Azimuth**](docs/using-azimuth.md#opentrack-on-the-pc).
- **Board feels warm** — Normal when Wi‑Fi is active on a small module. More context: [**Using Azimuth → Power & heat**](docs/using-azimuth.md#power-heat-and-battery).

## Documentation

| Doc | Audience |
|-----|----------|
| [**Quick start**](docs/quickstart.md) | Fast path from flash to tracking over Wi‑Fi |
| [**Using Azimuth**](docs/using-azimuth.md) | Settings portal, OpenTrack (USB + Wi‑Fi), tips & troubleshooting |
| [**Development**](docs/development.md) | Building firmware, CI, versioning, repo layout |
| [**Wiring**](docs/wiring.md) · [**Hardware profiles**](docs/hardware-profiles.md) (XIAO vs module PCB) · [**Parts / BOM**](docs/parts-list.md) · [**KiCad**](docs/kicad.md) | Hardware |
| [**Roadmap**](docs/roadmap.md) | Progress, milestones, future work |

## Goals

- **Performance** — Low latency and stable orientation for games and desktop use.
- **Accessibility** — Easy-to-source parts and open tooling so people can build and modify their own trackers.
- **Interop** — First-class support for the same **OpenTrack** pipelines games and sims already use.
- **Evolvability** — Room for better calibration, on-device UX, and companion apps without legacy platform limits.

## Progress (toward V1)

| Area | Progress | Notes |
|------|:--------:|--------|
| Hardware / BOM | 100% | [Parts list](docs/parts-list.md) |
| Custom PCB | ~95% | Matches [wiring](docs/wiring.md); production **panel** still to finalize |
| Firmware | 60–70% | Tracking + Wi‑Fi + settings portal + USB Hatire + release flow; board I/O, battery, and polish on [roadmap](docs/roadmap.md) |
| 3D enclosure | 0% | Not started |

```
Hardware/BOM   [████████████████████] 100%
PCB            [███████████████████░] ~95%
Firmware       [█████████████░░░░░░░] 60–70%
Enclosure      [░░░░░░░░░░░░░░░░░░░░] 0%
```

**Detailed roadmap:** [docs/roadmap.md](docs/roadmap.md)
