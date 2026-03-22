# Parts list — Azimuth PCB (`ESP32_BNO086`)

Values and references match the KiCad schematic **`kicad/ESP32_BNO086/ESP32_BNO086.kicad_sch`** (commit your latest KiCad save before treating this as the single source of truth). Footprints are as assigned in the layout.

---

## Active & modules

| Qty | Ref | Part / value | Package / footprint | Notes |
|-----|-----|----------------|---------------------|--------|
| 1 | **U1** | Seeed **XIAO ESP32-C3** | `1_MyFootPrints:XIAO-ESP32-C3-DIP-SMD` | MCU + USB |
| 1 | **IC1** | **BNO086** | `Package_LGA:LGA-28_5.2x3.8mm_P0.5mm` | IMU, bottom-mounted in layout |
| 1 | **PH2.0** | JST **PH** 2-pin battery input | `Connector_JST:JST_PH_S2B-PH-SM4-TB_1x02-1MP_P2.00mm_Horizontal` | `Bat+` / GND |
| 1 | **S1** | **JS102011SAQN** slide switch | `1_MyFootPrints:SW_JS102011SAQN` | C&K; battery / power path |
| 1 | **LED1** | LED (generic) | `LED_SMD:LED_0402_1005Metric` | Status; bottom side |
| 1 | **LS1** | **SMT-0540-T-9-R** | `1_MyFootPrints:XDCR_SMT-0540-T-9-R` | Pui Audio magnetic buzzer |
| 1 | **RST/Center1** | Tact **EVQP2** class | `Button_Switch_SMD:SW_SPST_EVQP2_MiddlePushTravel_H2.5mm` | Panasonic-style SMD tact |
| 1 | **FUNC1** | Same as above | same | Second button |

---

## Resistors

| Qty | Ref | Value | Footprint |
|-----|-----|--------|-----------|
| 2 | **R1**, **R2** | **220 kΩ** | `Resistor_SMD:R_0603_1608Metric` |
| 1 | **R3** | **470 Ω** | `Resistor_SMD:R_0402_1005Metric` |

**R1 / R2** — divider for **battery voltage** into **GPIO2 (D0)**.  
**R3** — current limit for **LED1** from **GPIO3 (D1)**.

---

## Capacitors

| Qty | Ref | Value | Footprint | Role (from nets) |
|-----|-----|--------|-----------|------------------|
| 1 | **C1** | **10 µF** | `Capacitor_SMD:C_0603_1608Metric` | Bulk on **`Bat+`** |
| 1 | **C2** | **0.1 µF** | `Capacitor_SMD:C_0603_1608Metric` | HF bypass on **`Bat+`** (near divider) |
| 1 | **C3** | **0.1 µF** | `Capacitor_SMD:C_0402_1005Metric` | Local decoupling at **BNO086 VDD** |

---

## Passives vs “no passives” blocks

The **only** capacitors on this rev are **battery / divider related (C1, C2)** and **one IMU VDD cap (C3)**. The **LED** has **R3** only.

Everything below connects to the ESP32 with **no** extra resistors or capacitors in series/parallel on those signals:

| Block | What’s on the PCB | Typical practice |
|-------|-------------------|-------------------|
| **BNO086** | **C3** at **VDD** only | Internal SH-2 front-end still wants local **0.1 µF**; you have that. Optional: strap **VDDIO** per datasheet (see [wiring.md](wiring.md) note). No RC on **SPI** lines (normal at short length / 3 MHz). |
| **Tact switches** | Switch to **GND** | Rely on **internal pull-ups** on GPIO in firmware (`INPUT_PULLUP`). Optional: external **10–100 kΩ** if you need noise immunity or very long traces. |
| **Buzzer LS1** | **LOAD+** to GPIO, **LOAD−** to GND | Many magnetic buzzers are fine with GPIO **if** current & duty match the part. Check **SMT-0540-T-9-R** datasheet for max current; if it’s too high, add a **transistor + base resistor** (and sometimes a flyback diode for inductive types). No cap is required for a simple magnetic buzzer in many cases. |
| **LED** | **R3** only | Standard. |

If you add **I²C** or long wires later, consider **series resistors** on **SCK/MOSI/MISO** for overshoot and **stronger pull-up** on **H_INTN** (the SparkFun library already configures a pull-up on the MCU side).

---

## Firmware GPIO map (PCB)

| GPIO | XIAO | Connected to |
|------|------|----------------|
| 2 | D0 | Battery divider tap (ADC) |
| 3 | D1 | LED (via R3) |
| 4 | D2 | BNO086 **CS** |
| 5 | D3 | BNO086 **H_INTN** |
| 6 | D4 | **RST/Center1** button |
| 7 | D5 | Buzzer **LOAD+** |
| 8–10 | D8–D10 | **SPI** SCK / MISO / MOSI |
| 20 | D7 | BNO086 **NRST** |
| 21 | D6 | **FUNC1** button |

Align `src/main.cpp` `kPin*` values with any future re-route.
