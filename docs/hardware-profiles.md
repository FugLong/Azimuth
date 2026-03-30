# Hardware profiles — DIY vs Azimuth PCB

Azimuth uses **one firmware tree** with **PlatformIO environments** that differ only by **board definition** (Arduino core variant, USB serial, flash layout). **GPIO numbers** for the IMU and planned I/O are fixed in **`include/azimuth_hw.h`** so both hardware paths stay aligned as long as the PCB routes those nets to the same ESP32-C3 GPIOs.

Naming: **`_diy`** = Seeed XIAO (breadboard or `ESP32_BNO086`). **`_pcb`** = integrated Azimuth board with a soldered **ESP32-C3** module (e.g. WROOM-02). **`main`** = full firmware (portal, Hatire, UDP); **`debug`** = serial IMU angles only.

---

## Which profile should I use?

| You have | PlatformIO environment | Notes |
|----------|------------------------|--------|
| **Seeed XIAO ESP32-C3** + BNO08x breakout (hand-wired) | **`azimuth_main_diy`** or **`azimuth_debug_diy`** | Default; `board = seeed_xiao_esp32c3` |
| **KiCad `ESP32_BNO086`** (XIAO on PCB) | **`azimuth_main_diy`** | Same as DIY — XIAO footprint |
| **Integrated Azimuth PCB** with **ESP32-C3 module** (e.g. **WROOM-02**), USB on the module | **`azimuth_main_pcb`** or **`azimuth_debug_pcb`** | `board = esp32-c3-devkitc-02` — matches a 4 MB **ESP32-C3** module + native USB |

```bash
# DIY — XIAO / ESP32_BNO086-style PCB
python3 -m platformio run -e azimuth_main_diy -t upload

# Azimuth integrated PCB (WROOM-class layout; see KiCad Azimuth_Design)
python3 -m platformio run -e azimuth_main_pcb -t upload
```

**USB serial (Hatire, flasher, serial monitor):** Firmware sets **`-DARDUINO_USB_MODE=1`** and **`-DARDUINO_USB_CDC_ON_BOOT=1`** for all profiles so **`Serial`** is USB CDC on both the XIAO and the DevKit-class board file used for the module PCB. The stock **`esp32-c3-devkitc-02`** board JSON does not enable USB mode by itself; those flags are required.

**Published USB web flasher:** CI and GitHub Pages still build **`azimuth_main_diy`** (XIAO) by default. If you ship a module-only product, add a second manifest entry or a forked flasher once you publish **`azimuth_main_pcb`** binaries — same `esp-web-tools` flow, different artifact path.

---

## GPIO contract (do not change without a firmware update)

Pins are **GPIO numbers** on ESP32-C3, not package pin numbers. XIAO silk **D*** names are shown for reference only.

| Function | GPIO | XIAO label (reference) |
|----------|------|-------------------------|
| SPI SCK | 8 | D8 |
| SPI MISO | 9 | D9 |
| SPI MOSI | 10 | D10 |
| IMU CS | 5 | D3 |
| IMU INT | 6 | D4 |
| IMU NRST | 20 | D7 (shared with UART RX) |
| PS0 / WAKE (strap + net) | 4 | D2 |
| Status LED (planned) | 3 | D1 |
| Button FUNC (planned) | 7 | D5 |
| Buzzer (planned) | 21 | D6 (shared with UART TX) |
| Battery sense ADC (planned) | 2 | D0 |

If a future PCB must move a function to a different GPIO, add a **second** set of defines in **`azimuth_hw.h`** (or build-flag–selected headers) and a new PlatformIO environment — avoid silent drift between `wiring.md` and code.

---

## KiCad projects in this repo

| Directory | MCU | Purpose |
|-----------|-----|---------|
| **`kicad/ESP32_BNO086/`** | XIAO ESP32-C3 (module on PCB) | Harness / carrier with XIAO footprint; matches [wiring.md](wiring.md) as built today |
| **`kicad/Azimuth_Design/`** | **ESP32-C3-WROOM-02** (SMD module) | Integrated Azimuth PCB — same **electrical** IMU and peripheral nets, proper module + antenna |

---

## Schematic — wiring the **ESP32-C3-WROOM-02** (Azimuth_Design)

Use the **same** BNO086 block and passives as **`ESP32_BNO086`**. Replace the XIAO symbol with **U1** `ESP32-C3-WROOM-02-N4` and connect **by GPIO name**, not by XIAO “D” numbers.

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
| LED anode (via resistor) | **IO3** | 3 |
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
