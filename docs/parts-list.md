# Parts list — Azimuth PCB (`ESP32_BNO086`)

Values, references, and footprints match the KiCad project **`kicad/ESP32_BNO086/`** (schematic **`ESP32_BNO086.kicad_sch`**, layout **`ESP32_BNO086.kicad_pcb`**). Save KiCad before treating this as the single source of truth.

---

## Active & modules

| Qty | Ref | Part / value | Footprint (KiCad) | Notes |
|-----|-----|----------------|-------------------|--------|
| 1 | **U1** | Seeed **XIAO ESP32-C3** | `1_MyFootPrints:XIAO-ESP32-C3-DIP-SMD` | MCU + USB; MPN e.g. **113991054** (Seeed) |
| 1 | **IC1** | **BNO086** | `Package_LGA:LGA-28_5.2x3.8mm_P0.5mm` | IMU; bottom side in layout |
| 1 | **PH2.0** | JST **PH** 2-pin | `Connector_JST:JST_PH_S2B-PH-SM4-TB_1x02-1MP_P2.00mm_Horizontal` | **`/Bat+`** (pin 1) / **GND** |
| 1 | **PWR1** | **K3-1280S-K1** slide switch | `1_MyFootPrints:SW-SMD_K3-1280S-K1` | Power path: **`/Bat+`** ↔ **`vcc`** (see [wiring.md](wiring.md)); LCSC **C128953** |
| 1 | **LED1** | **YLED0402R** (red, generic **LED** in sch.) | `LED_SMD:LED_0402_1005Metric` | Status; LCSC **C21260817** |
| 1 | **BUZZER1** | **MLT-5020** | `1_MyFootPrints:BUZ-SMD_L5.0-W5.5-P4.60` | Magnetic buzzer; LCSC **C94598** |
| 1 | **FUNC1** | **HX 3×4×2** tactile (2P) | `1_MyFootPrints:KEY-SMD_L4.0-W3.0-LS4.9-1` | User button; LCSC **C49234124** |

---

## Resistors

| Qty | Ref | Value | Footprint |
|-----|-----|--------|-----------|
| 2 | **R1**, **R2** | **220 kΩ** | `Resistor_SMD:R_0603_1608Metric` |
| 1 | **R3** | **470 Ω** | `Resistor_SMD:R_0402_1005Metric` |
| 3 | **R4**, **R5**, **R6** | **10 kΩ** | `Resistor_SMD:R_0402_1005Metric` |
| 2 | **R7**, **R8** | **4.7 kΩ** | `Resistor_SMD:R_0402_1005Metric` |

**R1 / R2** — divider from **`vcc`** (switched battery rail) to **GND**, tap to **GPIO2 (D0)** for ADC.  
**R3** — current limit for **LED1** from **GPIO3 (D1)**.  
**R4** — pull-up **BNO086 `BOOTN`** → **3.3 V** (normal boot, not IMU DFU).  
**R5** — pull-up **BNO086 `PS0/WAKE`** → **3.3 V** (SPI strap high at reset; MCU can drive **D2**).  
**R6** — pull-up **BNO086 `CLKSEL0`** → **3.3 V** (internal oscillator; `CLKSEL1` NC).  
**R7 / R8** — pull-ups **BNO086 `ENV_SCL` / `ENV_SDA`** → **3.3 V**.

---

## Capacitors

| Qty | Ref | Value | Footprint | Role (nets) |
|-----|-----|--------|-----------|----------------|
| 1 | **C1** | **10 µF** | `Capacitor_SMD:C_0603_1608Metric` | Bulk **`vcc`** → **GND** |
| 1 | **C2** | **0.1 µF** | `Capacitor_SMD:C_0603_1608Metric` | HF bypass **`vcc`** → **GND** (near divider) |
| 1 | **C3** | **10 µF** | `Capacitor_SMD:C_0603_1608Metric` | Bulk **3.3 V** at **IC1** (`Net-(IC1-PS1)` / VDD / VDDIO) |
| 1 | **C4** | **0.1 µF** | `Capacitor_SMD:C_0603_1608Metric` | HF **3.3 V** → **GND** |
| 1 | **C5** | **100 nF** | `Capacitor_SMD:C_0402_1005Metric` | **BNO086 `CAP`** → **GND** |

---

## Passives vs “no passives” blocks

| Block | What’s on the PCB | Typical practice |
|-------|-------------------|------------------|
| **BNO086** | **C3**, **C4** on **3.3 V**; **C5** on **`CAP`**; **R4/R5/R6** on **`BOOTN`/`PS0`/`CLKSEL0`**; **R7/R8** on **`ENV_SCL`/`ENV_SDA`** | No series RC on **SPI** for short traces / 3 MHz. |
| **Battery path** | **`/Bat+`** from **PH2.0** through **PWR1** to **`vcc`**; **C1/C2** and **R2** high side on **`vcc`** | Divider reads switched rail; see [wiring.md](wiring.md). |
| **FUNC1** | Switch to **GND** | **Internal pull-up** on GPIO in firmware (`INPUT_PULLUP`). |
| **BUZZER1** | **LOAD+** to GPIO, **LOAD−** to GND | Check **MLT-5020** current vs GPIO; transistor + base resistor if needed. |
| **LED1** | **R3** only | Standard. |

If you add long wires later, consider **series resistors** on **SCK/MOSI/MISO** and a **stronger pull-up** on **H_INTN** (the SparkFun library can configure a pull-up on the MCU side).

---

## Firmware GPIO map (PCB)

| GPIO | XIAO | Connected to |
|------|------|----------------|
| 2 | D0 | Battery divider tap (**R1**/**R2** → ADC) |
| 3 | D1 | **LED1** (via **R3**) |
| 4 | D2 | BNO086 **PS0 / WAKE** |
| 5 | D3 | BNO086 **CS** |
| 6 | D4 | BNO086 **H_INTN** |
| 7 | D5 | **FUNC1** |
| 8–10 | D8–D10 | **SPI** SCK / MISO / MOSI |
| 20 | D7 | BNO086 **NRST** |
| 21 | D6 | **BUZZER1** **LOAD+** |

Align `src/main.cpp` `kPinCs` / `kPinInt` / `kPinRst` with any future re-route.
