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
| 1 | **FUNC1** | Tact **EVQP2** class | `Button_Switch_SMD:SW_SPST_EVQP2_MiddlePushTravel_H2.5mm` | Single user button |

---

## Resistors

| Qty | Ref | Value | Footprint |
|-----|-----|--------|-----------|
| 2 | **R1**, **R2** | **220 kΩ** | `Resistor_SMD:R_0603_1608Metric` |
| 1 | **R3** | **470 Ω** | `Resistor_SMD:R_0402_1005Metric` |
| 3 | **R4**, **R5**, **R6** | **10 kΩ** | `Resistor_SMD:R_0402_1005Metric` |
| 2 | **R7**, **R8** | **4.7 kΩ** | `Resistor_SMD:R_0402_1005Metric` |

**R1 / R2** — divider for **battery voltage** into **GPIO2 (D0)**.  
**R3** — current limit for **LED1** from **GPIO3 (D1)**.  
**R4** — pull-up **BNO086 `BOOTN`** → **3.3 V** (normal boot, not IMU DFU).  
**R5** — pull-up **BNO086 `PS0/WAKE`** → **3.3 V** (ensures SPI strap high at reset, still MCU-drivable after reset).  
**R6** — pull-up **BNO086 `CLKSEL0`** → **3.3 V** (selects internal oscillator with `CLKSEL1` left unconnected).  
**R7 / R8** — pull-ups **BNO086 `ENV_SCL` / `ENV_SDA`** → **3.3 V** (secondary environmental I2C bus idle-high, as recommended by datasheet).

---

## Capacitors

| Qty | Ref | Value | Footprint | Role (from nets) |
|-----|-----|--------|-----------|------------------|
| 1 | **C1** | **10 µF** | `Capacitor_SMD:C_0603_1608Metric` | Bulk on **`Bat+`** |
| 1 | **C2** | **0.1 µF** | `Capacitor_SMD:C_0603_1608Metric` | HF bypass on **`Bat+`** (near divider) |
| 1 | **C3** | **10 µF** | `Capacitor_SMD:C_0603_1608Metric` | Bulk on **3.3 V** at **BNO086** (`VDD` / `PS1` / `VDDIO` net) |
| 1 | **C4** | **0.1 µF** | `Capacitor_SMD:C_0603_1608Metric` | HF decoupling on same **3.3 V** net to **GND** |
| 1 | **C5** | **100 nF** | `Capacitor_SMD:C_0402_1005Metric` | **BNO086 `CAP`** pin → **GND** (datasheet) |

---

## Passives vs “no passives” blocks

| Block | What’s on the PCB | Typical practice |
|-------|-------------------|-------------------|
| **BNO086** | **C3**, **C4** on **3.3 V**; **C5** on **`CAP`**; **R4/R5/R6** straps on **`BOOTN`/`PS0`/`CLKSEL0`**; **R7/R8** pull-ups on **`ENV_SCL`/`ENV_SDA`** | No series RC on **SPI** lines for short traces / 3 MHz. |
| **Tact switch FUNC1** | Switch to **GND** | Use **internal pull-ups** on GPIO in firmware (`INPUT_PULLUP`). Optional: external **10–100 kΩ** for noisy environments. |
| **Buzzer LS1** | **LOAD+** to GPIO, **LOAD−** to GND | Check **SMT-0540-T-9-R** max current vs GPIO; add transistor + base resistor if needed. |
| **LED** | **R3** only | Standard. |

If you add long wires later, consider **series resistors** on **SCK/MOSI/MISO** and a **stronger pull-up** on **H_INTN** (the SparkFun library can configure a pull-up on the MCU side).

---

## Firmware GPIO map (PCB)

| GPIO | XIAO | Connected to |
|------|------|----------------|
| 2 | D0 | Battery divider tap (ADC) |
| 3 | D1 | LED (via R3) |
| 4 | D2 | BNO086 **PS0 / WAKE** |
| 5 | D3 | BNO086 **CS** |
| 6 | D4 | BNO086 **H_INTN** |
| 7 | D5 | **FUNC1** button |
| 8–10 | D8–D10 | **SPI** SCK / MISO / MOSI |
| 20 | D7 | BNO086 **NRST** |
| 21 | D6 | Buzzer **LOAD+** |

Align `src/main.cpp` `kPinCs` / `kPinInt` / `kPinRst` with any future re-route.
