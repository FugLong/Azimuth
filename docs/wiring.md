# Wiring — DIY (XIAO + BNO08x) and Azimuth custom PCB

This document matches the **GPIO netlist** in **`include/azimuth_hw.h`**.

| Setup | What it is | Firmware |
|-------|------------|----------|
| **DIY** | **Seeed XIAO ESP32-C3** + **BNO08x** breakout (**SPI**) — breadboard, perfboard, or any hand-wired layout | **`azimuth_*_diy`** |
| **Azimuth custom PCB** | [**`kicad/Azimuth_Design`**](../kicad/Azimuth_Design/) — **ESP32-C3-WROOM-02**, **BNO086**, RGB LED, buzzer, button | **`azimuth_*_pcb`** |

SPI and IMU use the **same GPIO map** in firmware. The **Azimuth PCB** adds on-board **RGB LED**, **buzzer**, and **FUNC** button; **default DIY** is usually **IMU only** (no Azimuth-style RGB). Optional DIY **buzzer** / **button** use the same GPIOs as the custom PCB (see below). [**hardware-profiles.md**](hardware-profiles.md) has the feature overview.

---

## MCU

| Item | Detail |
|------|--------|
| DIY | Seeed **XIAO ESP32-C3** (USB CDC) — the board Arduino calls **`seeed_xiao_esp32c3`** |
| Azimuth custom PCB | **ESP32-C3-WROOM-02** (or compatible) — schematic **U1** in **`Azimuth.kicad_pro`**; flash with **`azimuth_main_pcb`** |

### XIAO pin map (authoritative — matches Azimuth firmware)

All **D→GPIO** assignments below match Seeed’s published table for **XIAO ESP32-C3** ([Getting Started — Pin Map](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)). **`include/azimuth_hw.h`** uses these GPIO numbers.

| XIAO pin | GPIO | Seeed notes (short) |
|----------|------|---------------------|
| D0 | **2** | ADC1_CH2 |
| D1 | **3** | ADC1_CH3 |
| D2 | **4** | ADC1_CH4, FSPIHD, MTMS |
| D3 | **5** | ADC2_CH0, FSPIWP, MTDI |
| D4 | **6** | FSPICLK, MTCK |
| D5 | **7** | FSPID, MTDO |
| D6 | **21** | U0TXD |
| D7 | **20** | U0RXD |
| D8 | **8** | SPI SCK |
| D9 | **9** | SPI MISO |
| D10 | **10** | MOSI, FSPICS0 |

**Strapping (Seeed + ESP32-C3 datasheet):** **GPIO2**, **GPIO8**, and **GPIO9** can affect boot / download behavior at reset. Firmware uses **D0/GPIO2** (battery sense when that circuit exists), **D8–D10/GPIO8–10** (SPI). Keep IMU and straps valid at reset (see IMU passives below).

---

## BNO086 / BNO08x — SPI

**Signal names** below match the **BNO086** land pattern used on the **Azimuth custom PCB**; a **BNO08x** breakout may label pins differently—map by function (CS, INT, RST, SCK, MISO, MOSI, PS0, power).

On the **Azimuth** PCB, **IC1** is the bare **BNO086** (often on the bottom layer). There **`PS1`**, **`VDD`**, and **`VDDIO`** are on **3.3 V** (SPI strap), and **`PS0/WAKE`** goes to **D2** with a **10 kΩ** pull-up to **3.3 V** so the strap is valid at reset while the MCU can drive the pin later.

For **DIY** with a **breakout**, follow the breakout’s SPI wiring; tie **PS0** / **PS1** per the breakout docs (often **3.3 V** for SPI mode).

| BNO086 function | XIAO pin | GPIO | Firmware (`azimuth_hw.h`) |
|-----------------|----------|------|---------------------------|
| **H_CSN** (CS) | **D3** | 5 | `kPinImuCs` |
| **H_INTN** | **D4** | 6 | `kPinImuInt` |
| **NRST** | **D7** | 20 | `kPinImuRst` |
| **PS0 / WAKE** | **D2** | 4 | `kPinImuPs0Wake` (strap / WAKE; not asserted in current firmware) |
| **H_SCL / SCK** | **D8** | 8 | FSPI `SCK` |
| **H_SDA / MISO** | **D9** | 9 | FSPI `MISO` |
| **SA0 / H_MOSI** | **D10** | 10 | FSPI `MOSI` |
| **VDD**, **VDDIO**, **PS1** | 3V3 | — | Same 3.3 V rail on PCB |
| **GND** | GND | — | Common ground |

Arduino core pins: `SCK`/`MISO`/`MOSI` match **D8/D9/D10** on this XIAO variant.

### D7 / GPIO20

**D7** is **GPIO20**, which overlaps the **UART RX** function on this board. Using it for **NRST** is fine if you are not using hardware UART RX on that pin.

---

## Power — battery (optional, on Azimuth PCB)

**DIY** builds are often **USB-powered** only; no battery section required.

The **Azimuth custom PCB** can include a **JST PH2.0** (**`PH2.0`**) and slide switch **PWR1** so a cell can be disconnected. Net names on **`Azimuth.kicad_sch`** include **`/Bat+`**, **`VBAT`**, **`VBAT_SW`**, etc.—see the schematic.

| Net (typical) | Role |
|---------------|------|
| **`/Bat+`** | Raw pack positive: **JST PH2.0** pin 1 and **PWR1** input |
| Switched rail | After **PWR1** — to MCU / divider / bulk (exact label on schematic) |

| Item | Detail |
|------|--------|
| Connector | **JST PH** 2-pin (example: **`S2B-PH-SM4-TB`** SMD) |
| Path | **`Bat+`** → **PWR1** → switched rail → MCU battery input — **see `Azimuth.kicad_sch`** |

**Battery voltage sense** (for ADC on the ESP32):

- **R1**, **R2** — **220 kΩ** (0603), divider from the switched rail to **GND**, tap to **D0** (GPIO2). When **PWR1** is off, the switched rail is not driven by the cell through this path, so the ADC does not read a meaningful pack voltage until the switch is on (or USB powers the board separately).
- **C2** — **0.1 µF** (0603) from the switched rail to **GND** — HF bypass at the divider / input node.

**Bulk on the switched rail:**

- **C1** — **10 µF** (0603) from the switched rail to **GND** (local bulk after the switch; raw **`Bat+`** is only at the connector and switch input pad).

**Power / charging:** Final **3.3 V regulation** and **Li-ion charging** are board-specific; **Azimuth_Design** may still be in progress on that block — follow the schematic and [**hardware-profiles.md**](hardware-profiles.md).

---

## User I/O — DIY vs Azimuth custom PCB

| | **DIY** (XIAO + breakout) | **Azimuth custom PCB** (`Azimuth_Design`) |
|---|---------------------------|-------------------------------------------|
| **RGB LED** | **Not in scope** for default DIY — no on-board Azimuth RGB; do not assume GPIO **0 / 1 / 3** are wired | **LED1** — **TZ-P4-1615RGBTCA1-0.55T**; **R3** / **R14** / **R15** to **IO0**, **IO1**, **IO3**; **COM+** to **3V3**; silk “2-R, 3-G, 4-B” |
| **FUNC1** | Optional — tact switch **D5** (**GPIO7**) | **FUNC1** on PCB |
| **BUZZER1** | Optional — e.g. **MLT-5020** **+** to **D6** (**GPIO21**), **−** to **GND** | **BUZZER1** on PCB |

**GPIO21** is **UART TX** on the XIAO; using it for a buzzer is fine if you do not need that UART for debug.

---

## IMU passives (3.3 V rail)

| Ref | Value | Role |
|-----|-------|------|
| **C3** | **10 µF** (0603) | Bulk on **3.3 V** near **IC1** (`VDD` / `PS1` / `VDDIO` net) |
| **C4** | **0.1 µF** (0603) | HF decoupling on the same **3.3 V** net to **GND** |
| **C5** | **100 nF** (0402) | **CAP** (pin 9) → **GND** |
| **R4** | **10 kΩ** (0402) | **BOOTN** → **3.3 V** (normal run; not IMU DFU) |
| **R5** | **10 kΩ** (0402) | **PS0/WAKE** → **3.3 V** (SPI strap high at reset, then WAKE use) |
| **R6** | **10 kΩ** (0402) | **CLKSEL0** → **3.3 V** (internal oscillator selection with `CLKSEL1` NC) |
| **R7** | **4.7 kΩ** (0402) | **ENV_SCL** → **3.3 V** pull-up |
| **R8** | **4.7 kΩ** (0402) | **ENV_SDA** → **3.3 V** pull-up |

---

## Bring-up checklist

1. **SPI strap/checks:** **PS1** (and **VDD** / **VDDIO**) at **3.3 V**, **PS0** held high at reset (via pull-up + D2 routing), and **BOOTN** high for application boot.
2. **Clock/checks:** **CLKSEL0** pulled high (internal clock selection with `CLKSEL1` left unconnected), and **CAP** has dedicated **100 nF** to GND.
3. **ENV bus/checks:** **ENV_SCL** / **ENV_SDA** have pull-ups (R7/R8) even if no external environmental sensor is populated.
4. **SCK**, **INT**, and **CS** traces short; solid **GND** return.
5. After assembly, run **`azimuth_debug_diy`** or **`azimuth_debug_pcb`** (match your hardware) and confirm serial prints before switching to **`azimuth_main_diy`** / **`azimuth_main_pcb`** for OpenTrack / Wi‑Fi.
6. If init fails: check **3.3 V**, **NRST**, **H_INTN**, **SPI** order; on a custom PCB, re-run **ERC** / **DRC** in KiCad.

---

## Logical diagram (SPI + control)

```
BNO086                XIAO ESP32-C3
────────              ───────────────
SCK, MISO, MOSI  ───  D8, D9, D10
CS, H_INT, NRST  ───  D3, D4, D7
PS0/WAKE         ───  D2
VDD, VDDIO, PS1  ───  3V3 (on PCB)
GND              ───  GND
```

For breadboard bring-up with a **breakout**, follow the breakout silkscreen; tie **PS0/PS1** to **3.3 V** for SPI if that is how the breakout is strapped.
