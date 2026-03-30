# Wiring — Azimuth PCBs and DIY

This document matches the **GPIO netlist** used in firmware (**`include/azimuth_hw.h`**) for:

- **DIY:** **XIAO ESP32-C3** + BNO08x breakout  
- **KiCad `ESP32_BNO086`:** XIAO footprint on the carrier PCB ([**kicad/ESP32_BNO086**](../kicad/ESP32_BNO086/))  
- **Integrated module PCB:** **ESP32-C3-WROOM-02** (or compatible) in [**kicad/Azimuth_Design**](../kicad/Azimuth_Design/) — same GPIOs, different physical pins (see [**hardware-profiles.md**](hardware-profiles.md))

---

## MCU

| Item | Detail |
|------|--------|
| DIY / ESP32_BNO086 | Seeed **XIAO ESP32-C3** (USB CDC) — footprint `1_MyFootPrints:XIAO-ESP32-C3-DIP-SMD` |
| Azimuth_Design (module) | **ESP32-C3-WROOM-02** (or same GPIO map) — schematic **U1**; flash with **`azimuth_main_pcb`** |

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

**Strapping (Seeed + ESP32-C3 datasheet):** **GPIO2**, **GPIO8**, and **GPIO9** can affect boot / download behavior at reset. Azimuth uses **D0/GPIO2** (battery sense), **D8–D10/GPIO8–10** (SPI). This is the same trade-off as routing SPI on the XIAO; ensure IMU and passives keep valid levels at reset (your PCB already mirrors the working XIAO + BNO086 path).

---

## BNO086 (IC1) — SPI

The bare **BNO086** (LGA) is on the bottom side in the PCB layout. On this PCB, **`PS1`**, **`VDD`**, and **`VDDIO`** share the **3.3 V** net (SPI strap + supplies). **`PS0/WAKE`** is routed to **D2** **and** has a **10 kΩ pull-up to 3.3 V** so SPI strap timing is valid while remaining MCU-drivable after reset.

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

## Power — battery (PH2.0)

The schematic splits the cell from the load so the slide switch can cut battery power to the XIAO and the battery-side passives.

| Net (KiCad) | Role |
|-------------|------|
| **`/Bat+`** | Raw pack positive: **JST PH2.0** (`PH2.0`) pin 1 and **PWR1** (slide switch **K3-1280S-K1**) pad **2**. |
| **`vcc`** | Switched rail: **PWR1** pad **1**, XIAO **Bat+** (pin 15), **R2** / **C1** / **C2** high side. |

| Item | Detail |
|------|--------|
| Connector | **JST PH** 2-pin (`PH2.0`, `S2B-PH-SM4-TB` horizontal SMD) |
| Path | **`Bat+`** → **PWR1** → **`vcc`** → XIAO **Bat+** / **Bat−** → GND |

**Battery voltage sense** (for ADC on the ESP32):

- **R1**, **R2** — **220 kΩ** (0603), divider from **`vcc`** (switched rail) to **GND**, tap to **D0** (GPIO2). When **PWR1** is off, **`vcc`** is not driven by the cell through this path, so the ADC does not read a meaningful pack voltage until the switch is on (or USB powers the board separately).
- **C2** — **0.1 µF** (0603) from **`vcc`** to **GND** — HF bypass at the divider / input node.

**Bulk on the switched rail:**

- **C1** — **10 µF** (0603) from **`vcc`** to **GND** (local bulk after the switch; raw **`Bat+`** is only at the connector and switch input pad).

---

## User I/O

| Ref | Function | XIAO pin | GPIO | Notes |
|-----|----------|----------|------|--------|
| **LED1** + **R3** | Status LED | **D1** | 3 | **R3 = 470 Ω** (0402) in series with LED anode; cathode → GND |
| **FUNC1** | Tact switch | **D5** | 7 | One side → **GND**, other → GPIO (use **internal pull-up** in firmware) |
| **BUZZER1** | **MLT-5020** | **D6** | 21 | **LOAD+** → GPIO, **LOAD−** → **GND** |

GPIO21 is also **UART TX**; fine for the buzzer if you do not need that UART for debug.

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
6. If init fails: check **3.3 V**, **NRST**, **H_INTN**, **SPI** order, then re-run DRC in KiCad.

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
