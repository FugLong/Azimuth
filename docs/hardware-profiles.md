# Hardware profiles — DIY vs Azimuth custom PCB

Azimuth uses **one firmware tree** with **PlatformIO environments** that differ only by **board definition** (Arduino core variant, USB serial, flash layout). **GPIO numbers** for the IMU and board I/O are fixed in **`include/azimuth_hw.h`** so every supported build stays aligned as long as each PCB routes those nets to the same ESP32-C3 GPIOs.

Naming: **`_diy`** = **Seeed XIAO ESP32-C3** + **BNO08x** breakout in **SPI** mode — wire it on a breadboard, perfboard, or any layout you like; firmware is the same. **`_pcb`** = **Azimuth custom PCB** ([**`kicad/Azimuth_Design`**](../kicad/Azimuth_Design/)) — **main platform** — with soldered **ESP32-C3** module (e.g. **WROOM-02**). **`main`** = full firmware (portal, Hatire, UDP); **`debug`** = serial IMU angles only.

---

## DIY vs Azimuth custom PCB (what ships on the board)

| | **DIY** (`*_diy`) — XIAO + BNO08x breakout | **Azimuth custom PCB** (`*_pcb`) — **`Azimuth_Design`** |
|---|------------------------------------------------------------------|--------------------------------------------------------|
| **MCU** | Seeed **XIAO ESP32-C3** (USB) | **ESP32-C3-WROOM-02** (or compatible), USB on module |
| **IMU** | **BNO08x** breakout (SPI), hand-wired | **BNO086** on board |
| **RGB LED** | **No** — not part of the default DIY setup; the XIAO does not implement the Azimuth RGB LED circuit | **Yes** — **LED1** (TZ-P4-1615RGB; see [wiring.md](wiring.md)) |
| **Buzzer** | **Optional** — wire to **GPIO21** (XIAO **D6**) if you add one | **Yes** — **BUZZER1** |
| **FUNC button** | **Optional** — wire to **GPIO7** (XIAO **D5**) if you add one | **Yes** — **FUNC1** |

**Firmware:** **`azimuth_main_diy`** and **`azimuth_main_pcb`** build the **same** application (Hatire, portal, UDP). Head tracking does **not** depend on LED, buzzer, or button. When those parts are absent (typical DIY breadboard), the device still runs as a head tracker.

---

## Which profile should I use?

| You have | PlatformIO environment | Notes |
|----------|------------------------|--------|
| **Seeed XIAO ESP32-C3** + **BNO08x** breakout (SPI; IMU only or IMU + optional buzzer/button) | **`azimuth_main_diy`** or **`azimuth_debug_diy`** | `board = seeed_xiao_esp32c3` |
| **Azimuth custom PCB** ([**`Azimuth.kicad_pro`**](../kicad/Azimuth_Design/Azimuth.kicad_pro)) — **ESP32-C3** module (e.g. **WROOM-02**), USB on the module | **`azimuth_main_pcb`** or **`azimuth_debug_pcb`** | `board = esp32-c3-devkitc-02` — 4 MB **ESP32-C3** module + native USB |

```bash
# DIY — XIAO + BNO08x breakout
python3 -m platformio run -e azimuth_main_diy -t upload

# Azimuth custom PCB (KiCad Azimuth_Design)
python3 -m platformio run -e azimuth_main_pcb -t upload
```

**USB serial (Hatire, flasher, serial monitor):** Firmware sets **`-DARDUINO_USB_MODE=1`** and **`-DARDUINO_USB_CDC_ON_BOOT=1`** for all profiles so **`Serial`** is USB CDC on both the XIAO and the DevKit-class board file used for the module PCB. The stock **`esp32-c3-devkitc-02`** board JSON does not enable USB mode by itself; those flags are required.

**Published USB web flasher:** CI and GitHub Pages still build **`azimuth_main_diy`** (XIAO) by default. If you ship a module-only product, add a second manifest entry or a forked flasher once you publish **`azimuth_main_pcb`** binaries — same `esp-web-tools` flow, different artifact path.

---

## GPIO contract (do not change without a firmware update)

Pins are **GPIO numbers** on ESP32-C3, not package pin numbers. XIAO silk **D*** names are shown for reference only.

| Function | GPIO | XIAO label (reference) | Typical hardware |
|----------|------|-------------------------|------------------|
| SPI SCK | 8 | D8 | DIY + Azimuth PCB |
| SPI MISO | 9 | D9 | DIY + Azimuth PCB |
| SPI MOSI | 10 | D10 | DIY + Azimuth PCB |
| IMU CS | 5 | D3 | DIY + Azimuth PCB |
| IMU INT | 6 | D4 | DIY + Azimuth PCB |
| IMU NRST | 20 | D7 (shared with UART RX) | DIY + Azimuth PCB |
| PS0 / WAKE (strap + net) | 4 | D2 | DIY + Azimuth PCB |
| RGB LED — R / G / B | **0, 1, 3** | (see note) | **Azimuth PCB only** — **LED1**; **not** used on DIY XIAO builds |
| Status LED (single channel / future use) | 3 | D1 | Constant **`kPinStatusLed`**; full RGB on PCB uses **0, 1, 3** — [wiring.md](wiring.md) |
| Button FUNC | 7 | D5 | Optional DIY; **FUNC1** on Azimuth PCB |
| Buzzer | 21 | D6 (shared with UART TX) | Optional DIY; **BUZZER1** on Azimuth PCB |
| Battery sense ADC | 2 | D0 | When populated — see [wiring.md](wiring.md) |

**Note:** On the **Azimuth custom PCB**, the RGB LED is routed to **GPIO 0, 1, and 3** (not only GPIO 3). The **Seeed XIAO** pinout does **not** match that RGB layout for a drop-in “add Azimuth LED to breadboard” build, and the stock DIY path does **not** include an RGB LED — use **`azimuth_main_diy`** without expecting LED I/O.

If a future PCB must move a function to a different GPIO, add a **second** set of defines in **`azimuth_hw.h`** (or build-flag–selected headers) and a new PlatformIO environment — avoid silent drift between `wiring.md` and code.

---

## KiCad — Azimuth custom PCB

| Directory | Purpose |
|-----------|---------|
| **`kicad/Azimuth_Design/`** | **Azimuth custom PCB** — **ESP32-C3-WROOM-02**, **BNO086**, RGB LED, buzzer, button, USB-C, battery — [wiring.md](wiring.md), [kicad.md](kicad.md) |

---

## Schematic — wiring the **ESP32-C3-WROOM-02** (Azimuth_Design)

On **`Azimuth.kicad_sch`**, **U1** is **`ESP32-C3-WROOM-02-N4`**. Connect the **BNO086** and passives **by GPIO name** (same numbers as DIY XIAO + breakout — [wiring.md](wiring.md)), not by XIAO “D” silk labels.

**BNO086 ↔ module (ESP32-C3 GPIO)**

| BNO086 signal | Connect to U1 pin (symbol name) | ESP32-C3 GPIO |
|---------------|----------------------------------|---------------|
| **H_CSN** (CS) | **IO5** | 5 |
| **H_INTN** | **IO6** | 6 |
| **NRST** | **RXD** (UART0 RX on C3) | **20** |
| **PS0/WAKE** | **IO4** | 4 |
| **H_SCL / SCK** | **IO8** | 8 |
| **H_SDA / MISO** | **IO9** | 9 |
| **SA0 / H_MOSI** | **IO10** | 10 |
| **VDD**, **VDDIO**, **PS1** | **3V3** | — |
| **GND** | **GND** | — |

**Other board nets (match [wiring.md](wiring.md))**

| Net | U1 (symbol) | GPIO |
|-----|-------------|------|
| RGB **LED1** — channels (via **R3** / **R14** / **R15**) | **IO0**, **IO1**, **IO3** | 0, 1, 3 |
| FUNC1 switch | **IO7** | 7 |
| Buzzer + | **TXD** | **21** |
| Battery divider tap | **IO2** | 2 |

**USB:** On ESP32-C3, USB is **GPIO18** / **GPIO19** (wired inside the module to the USB pads). No extra USB UART chip is required; firmware uses **USB CDC** for **`Serial`**.

**Power / reset:** Tie **EN** with RC as in Espressif reference; **3V3** and **GND** with adequate decoupling per [ESP32-C3 hardware design guidelines](https://www.espressif.com/sites/default/files/documentation/esp32-c3_hardware_design_guidelines_en.pdf). **BOOT** / strap pins: follow the WROOM-02 datasheet so the module boots from flash and does not enter download mode unintentionally.

**Antenna:** Keep per-module RF keepout; no copper or metal under the antenna area of **WROOM-02**.

---

## Work checklist (maintenance)

- [x] Shared pin header **`include/azimuth_hw.h`**
- [x] PlatformIO **`azimuth_main_diy`** / **`azimuth_main_pcb`** (+ debug variants)
- [ ] **Azimuth_Design** PCB: ERC/DRC, BOM lock, fab **after** net review against this doc
- [ ] Optional: CI job to build **`azimuth_main_pcb`** and archive artifacts
- [ ] Optional: second **`manifest.json`** entry when a retail module board ships

---

**Related:** [wiring.md](wiring.md) · [development.md](development.md) · [kicad.md](kicad.md)
