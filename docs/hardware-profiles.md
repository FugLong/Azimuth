# Hardware profiles

Azimuth is intentionally **two hardware paths**—you only build one:

1. **DIY / breadboard** — A **Seeed XIAO ESP32-C3** and a **BNO08x-class** IMU breakout wired for **SPI**. Parts are easy to source — store links and options are in [**parts-list.md**](parts-list.md). The [**web flasher**](https://fuglong.github.io/Azimuth/) and default release build target this path (**`azimuth_main_diy`**). USB for flashing and **Hatire** is through the XIAO.

2. **Integrated PCB** — The routed board in [**`kicad/Azimuth_Design`**](https://github.com/FugLong/Azimuth/tree/main/kicad/Azimuth_Design) (**ESP32-C3-WROOM-02**, **BNO086** on the board, plus **RGB**, **buzzer**, and **FUNC** where populated). You flash **`azimuth_main_pcb`**. It runs the **same** head-tracking application (portal, UDP, Hatire); on-board extras are only relevant when that hardware exists.

Both paths share **one GPIO map** in **`include/azimuth_hw.h`**, so the IMU and common signals land on the same ESP32-C3 pins whether you follow **XIAO silk** labels or **U1** symbol names on the schematic. **Breadboard wiring** (which **D** pin goes where) and **PCB layout** (which **IOx** net on **U1**) are spelled out separately in [**wiring.md**](wiring.md) so you are not flipping between two stories in one table.

**Build suffixes:** **`_diy`** = XIAO + breakout. **`_pcb`** = Azimuth_Design. **`main`** = full firmware (portal, Hatire, UDP). **`debug`** = USB serial IMU angles only, no Wi‑Fi or portal.

---

## Targets

| Role | **`_diy`** | **`_pcb`** |
|------|------------|------------|
| **MCU** | XIAO ESP32-C3 | WROOM-class module on Azimuth_Design |
| **IMU** | BNO08x breakout | BNO086 on board |
| **Extra I/O** | Buzzer/button optional (same GPIOs as PCB) | RGB LED, buzzer, FUNC on board |

**Firmware:** **`azimuth_main_diy`** and **`azimuth_main_pcb`** are the same application. Tracking does not require LED, buzzer, or button.

| You have | Environment |
|----------|-------------|
| XIAO + BNO08x (SPI) | **`azimuth_main_diy`** or **`azimuth_debug_diy`** — `board = seeed_xiao_esp32c3` |
| Azimuth_Design PCB | **`azimuth_main_pcb`** or **`azimuth_debug_pcb`** — `board = esp32-c3-devkitc-02` |

```bash
python3 -m platformio run -e azimuth_main_diy -t upload    # XIAO + breakout
python3 -m platformio run -e azimuth_main_pcb -t upload    # Azimuth_Design
```

Firmware sets **`-DARDUINO_USB_MODE=1`** and **`-DARDUINO_USB_CDC_ON_BOOT=1`** so **`Serial`** is USB CDC on both targets; the stock **`esp32-c3-devkitc-02`** JSON does not enable that by itself.

**Web flasher:** CI ships **`azimuth_main_diy`** by default. A second **`manifest.json`** entry (or fork) applies if you publish **`azimuth_main_pcb`** binaries.

---

## GPIO contract (change only with firmware + doc update)

Firmware-facing summary: same GPIO numbers on both paths. **Where to wire physically** — XIAO **D** pins vs **U1** schematic pins — is in [**wiring.md**](wiring.md) (DIY section vs PCB section), not duplicated here.

GPIO numbers are for ESP32-C3; **D*** labels refer to XIAO silk only (the module has no **D** names).

| Function | GPIO | XIAO | Notes |
|----------|------|------|--------|
| SPI SCK / MISO / MOSI | 8 / 9 / 10 | D8–D10 | |
| IMU CS / INT / NRST | 5 / 6 / 20 | D3, D4, D7 | |
| PS0 / WAKE | 4 | D2 | |
| RGB (Azimuth board) | 0, 1, 3 | — | Not present on default breadboard build |
| Status / future | 3 | D1 | **`kPinStatusLed`**; full RGB on PCB uses 0, 1, 3 — [wiring.md](wiring.md) |
| FUNC | 7 | D5 | Optional breadboard |
| Buzzer | 21 | D6 | Optional breadboard |
| Battery sense | 2 | D0 | When populated |

The Azimuth RGB LED is on **GPIO 0, 1, 3**—not a drop-in on XIAO silk; **`azimuth_main_diy`** does not drive that RGB layout.

A future PCB that moves a function needs new defines in **`azimuth_hw.h`** and a new env—avoid silent drift from [wiring.md](wiring.md).

**KiCad:** **`kicad/Azimuth_Design/`** — open **`Azimuth.kicad_pro`**. **U1** ↔ **IC1** routing and module notes are in [wiring.md](wiring.md) (**PCB path**). Libraries, panelization, workflow: [kicad.md](kicad.md).

---

## Work checklist (maintenance)

- [x] Shared pin header **`include/azimuth_hw.h`**
- [x] PlatformIO **`azimuth_main_diy`** / **`azimuth_main_pcb`** (+ debug variants)
- [x] **Azimuth_Design**: Docs [**parts-list.md**](parts-list.md) / [**wiring.md**](wiring.md) match **`Azimuth.kicad_sch`** (re-verify after sch edits); ERC/DRC before fab; export BOM from KiCad for pick-and-place
- [ ] Optional: CI build **`azimuth_main_pcb`** artifacts
- [ ] Optional: second **`manifest.json`** entry for retail PCB

**Related:** [wiring.md](wiring.md) · [parts-list.md](parts-list.md) · [development.md](development.md) · [kicad.md](kicad.md)
