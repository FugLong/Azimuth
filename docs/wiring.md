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

The bare **BNO086** (LGA) is on the bottom side in the PCB layout. **PS0** and **PS1** are tied to the **3.3 V** rail on the board (SPI / UART mode per datasheet).

| BNO086 function | XIAO pin | GPIO | Firmware (`main.cpp`) |
|-----------------|----------|------|------------------------|
| **H_CSN** (CS) | **D2** | 4 | `kPinCs` |
| **H_INTN** | **D3** | 5 | `kPinInt` |
| **NRST** | **D7** | 20 | `kPinRst` |
| **H_SCL / SCK** | **D8** | 8 | FSPI `SCK` |
| **H_SDA / MISO** | **D9** | 9 | FSPI `MISO` |
| **SA0 / H_MOSI** | **D10** | 10 | FSPI `MOSI` |
| **VDD** | 3V3 | — | Sensor core supply |
| **GND** | GND | — | Common ground |

Arduino core pins: `SCK`/`MISO`/`MOSI` match **D8/D9/D10** on this XIAO variant.

### D7 / GPIO20

**D7** is **GPIO20**, which overlaps the **UART RX** function on this board. Using it for **NRST** is fine if you are not using hardware UART RX on that pin.

---

## Power — battery (PH2.0)

| Item | Detail |
|------|--------|
| Connector | **JST PH** 2-pin (`PH2.0`, `S2B-PH-SM4-TB` horizontal SMD) |
| Net | `Bat+` → slide switch **S1** → XIAO **Bat+** / **Bat−** |

**Battery voltage sense** (for ADC on the ESP32):

- **R1**, **R2** — **220 kΩ** (0603), divider from `Bat+` to **GND**, tap to **D0** (GPIO2).
- **C2** — **0.1 µF** (0603) from `Bat+` to **GND** — HF bypass at the divider / input node.

**Bulk on battery rail:**

- **C1** — **10 µF** (0603) from `Bat+` to **GND**.

---

## User I/O (no extra passives on these nets in v1)

| Ref | Function | XIAO pin | GPIO | Notes |
|-----|----------|----------|------|--------|
| **LED1** + **R3** | Status LED | **D1** | 3 | **R3 = 470 Ω** (0402) in series with LED anode; cathode → GND |
| **RST/Center1** | Tact switch | **D4** | 6 | One side → **GND**, other → GPIO (use **internal pull-up** in firmware) |
| **FUNC1** | Tact switch | **D6** | 21 | Same; shares EVQP2 footprint style |
| **LS1** | Buzzer **SMT-0540-T-9-R** | **D5** | 7 | **LOAD+** → GPIO, **LOAD−** → **GND** |

GPIO21 is also **UART TX**; fine for a button if you do not need that UART.

---

## IMU-only passive

| Ref | Value | Role |
|-----|-------|------|
| **C3** | **0.1 µF** (0402) | Local decoupling at **BNO086 VDD** (to ground / return) |

---

## Design note (from PCB netlist)

**IC1 pin 28 (VDDIO)** is currently **not** tied to 3.3 V in the KiCad netlist. Many BNO08x designs strap **VDDIO** to the same rail as **VDD**. Confirm against the [BNO080/BNO085 datasheet](https://www.ceva-dsp.com/wp-content/uploads/2019/10/BNO080_085-Datasheet.pdf) and your assembly variant before production.

---

## Bring-up checklist

1. **PS0 / PS1** high for SPI (handled on PCB).
2. **SCK**, **INT**, and **CS** traces short; solid **GND** return.
3. After assembly, run firmware **`xiao_esp32c3`** and confirm serial prints before Hatire mode.
4. If init fails: check **3.3 V**, **NRST**, **H_INTN**, **SPI** order, then re-run DRC in KiCad.

---

## Logical diagram (SPI + control)

```
BNO086                XIAO ESP32-C3
────────              ───────────────
SCK, MISO, MOSI  ───  D8, D9, D10
CS, H_INT, NRST  ───  D2, D3, D7
VDD, GND         ───  3V3, GND
PS0, PS1         ───  3V3 (on PCB)
```

For breadboard bring-up with a **breakout**, follow the breakout silkscreen for pin names; tie **PS0/PS1** to **3.3 V** if using SPI.
