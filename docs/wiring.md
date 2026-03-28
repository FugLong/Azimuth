# Wiring — Azimuth PCB (`kicad/ESP32_BNO086`)

This document matches the **Azimuth** custom PCB (KiCad project `ESP32_BNO086`) and the firmware defaults in `src/main.cpp`. It also applies if you wire a **XIAO ESP32-C3** to a **BNO08x breakout** by hand using the same net names.

---

## MCU

| Item | Detail |
|------|--------|
| Board | Seeed **XIAO ESP32-C3** (USB CDC) |
| Footprint in KiCad | `1_MyFootPrints:XIAO-ESP32-C3-DIP-SMD` (SMD pads + castellated holes) |

---

## BNO086 (IC1) — SPI

The bare **BNO086** (LGA) is on the bottom side in the PCB layout. On this PCB, **`PS1`**, **`VDD`**, and **`VDDIO`** share the **3.3 V** net (SPI strap + supplies). **`PS0/WAKE`** is routed to **D2** **and** has a **10 kΩ pull-up to 3.3 V** so SPI strap timing is valid while remaining MCU-drivable after reset.

| BNO086 function | XIAO pin | GPIO | Firmware (`main.cpp`) |
|-----------------|----------|------|------------------------|
| **H_CSN** (CS) | **D3** | 5 | `kPinCs` |
| **H_INTN** | **D4** | 6 | `kPinInt` |
| **NRST** | **D7** | 20 | `kPinRst` |
| **PS0 / WAKE** | **D2** | 4 | (strap / WAKE; not a `kPin*` in current firmware) |
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
5. After assembly, run **`azimuth_debug`** (`pio run -e azimuth_debug`) and confirm serial prints before switching to **`azimuth_main`** for OpenTrack / Wi‑Fi.
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
