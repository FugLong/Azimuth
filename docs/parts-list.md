# Parts list — BOM reference (Azimuth custom PCB)

**Authoritative BOM:** export from KiCad **`kicad/Azimuth_Design/Azimuth.kicad_pro`** (or your fab’s tool) after **ERC** / **DRC**. Line items, references, and footprints must match **`Azimuth.kicad_sch`** / **`Azimuth.kicad_pcb`**.

The tables below are a **human-readable summary** for the **BNO086** and typical support passives—useful for understanding, not a substitute for the schematic. **DIY** (XIAO + breakout) has **no** fixed BOM.

A **battery pack** is usually **not** a PCB line item: the board may expose a **JST PH2.0** (**`PH2.0`**). For **wireless** use you still need a suitable **1S Li-ion / LiPo** and plug (check polarity in the schematic). USB-only builds skip the pack.

---

## Wireless battery (off-board — not PCB BOM)

| Field | Example |
|--------|---------|
| **Role** | Powers the board through **`PH2.0`** when not on USB; use a **protected** 1S cell and a **proper Li-ion/LiPo charger** (not on this PCB). |
| **Adafruit** | [**3898**](https://www.adafruit.com/product/3898) — lithium-ion polymer **3.7 V nominal**, **400 mAh**, **JST-PH** lead (~25 mm). Size about **36 × 17 × 7.8 mm** (per vendor). |
| **DigiKey** | [**1528-2731-ND**](https://www.digikey.com/en/products/detail/adafruit-industries-llc/3898/9685336) — same MPN **3898** / “BATTERY LITH-ION 3.7V 400MAH”. |

Other **JST-PH** 1S packs with the **same polarity** and connector series are fine; pick capacity and dimensions for your enclosure plans ([roadmap](roadmap.md)).

---

## Active & modules (see schematic)

| Qty | Ref | Part / value | Notes |
|-----|-----|----------------|--------|
| 1 | **U1** | **ESP32-C3-WROOM-02** (e.g. **-N4** flash option) | MCU module; USB via module pads — see **`Azimuth.kicad_sch`** |
| 1 | **IC1** | **BNO086** | IMU (**LGA**); often bottom side in layout |
| 1 | **PH2.0** | JST **PH** 2-pin | Battery receptacle if populated — pin 1 **`/Bat+`** per schematic |
| 1 | **PWR1** | **K3-1280S-K1** | Slide switch on battery path |
| 1 | **LED1** | **TZ-P4-1615RGBTCA1** (or sch. value) | **RGB** — [wiring.md](wiring.md), [hardware-profiles.md](hardware-profiles.md) |
| 1 | **BUZZER1** | **MLT-5020** | Magnetic buzzer |
| 1 | **FUNC1** | Tact switch (e.g. **HX** 3×4×2) | User button |
| 1 | **J1** | USB-C (per schematic) | Data + power in — see **`Azimuth.kicad_sch`** |

Exact **MPNs**, **LCSC** codes, and **footprint** strings live in the KiCad project.

---

## Resistors

| Qty | Ref | Value | Footprint |
|-----|-----|--------|-----------|
| 2 | **R1**, **R2** | **220 kΩ** | `Resistor_SMD:R_0603_1608Metric` |
| 1 | **R3** | **470 Ω** | `Resistor_SMD:R_0402_1005Metric` |
| 3 | **R4**, **R5**, **R6** | **10 kΩ** | `Resistor_SMD:R_0402_1005Metric` |
| 2 | **R7**, **R8** | **4.7 kΩ** | `Resistor_SMD:R_0402_1005Metric` |

**R1 / R2** — divider from the switched battery rail to **GND**, tap to **GPIO2** for ADC (see schematic net names).  
**R3** / **R14** / **R15** — **RGB LED** current limit (Azimuth PCB); values in **`Azimuth.kicad_sch`**.  
**R4** — pull-up **BNO086 `BOOTN`** → **3.3 V** (normal boot, not IMU DFU).  
**R5** — pull-up **BNO086 `PS0/WAKE`** → **3.3 V** (SPI strap high at reset; MCU can drive **D2**).  
**R6** — pull-up **BNO086 `CLKSEL0`** → **3.3 V** (internal oscillator; `CLKSEL1` NC).  
**R7 / R8** — pull-ups **BNO086 `ENV_SCL` / `ENV_SDA`** → **3.3 V**.

---

## Capacitors

| Qty | Ref | Value | Footprint | Role (nets) |
|-----|-----|--------|-----------|----------------|
| 1 | **C1** | **10 µF** | `Capacitor_SMD:C_0603_1608Metric` | Bulk on switched rail → **GND** |
| 1 | **C2** | **0.1 µF** | `Capacitor_SMD:C_0603_1608Metric` | HF bypass near divider / rail |
| 1 | **C3** | **10 µF** | `Capacitor_SMD:C_0603_1608Metric` | Bulk **3.3 V** at **IC1** (`Net-(IC1-PS1)` / VDD / VDDIO) |
| 1 | **C4** | **0.1 µF** | `Capacitor_SMD:C_0603_1608Metric` | HF **3.3 V** → **GND** |
| 1 | **C5** | **100 nF** | `Capacitor_SMD:C_0402_1005Metric` | **BNO086 `CAP`** → **GND** |

---

## Passives vs “no passives” blocks

| Block | What’s on the PCB | Typical practice |
|-------|-------------------|------------------|
| **BNO086** | **C3**, **C4** on **3.3 V**; **C5** on **`CAP`**; **R4/R5/R6** on **`BOOTN`/`PS0`/`CLKSEL0`**; **R7/R8** on **`ENV_SCL`/`ENV_SDA`** | No series RC on **SPI** for short traces / 3 MHz. |
| **Battery path** | **`/Bat+`** from **PH2.0** through **PWR1**; **C1/C2** and divider on switched rail | See [wiring.md](wiring.md) and schematic nets. |
| **FUNC1** | Switch to **GND** | **Internal pull-up** on GPIO in firmware (`INPUT_PULLUP`). |
| **BUZZER1** | **LOAD+** to GPIO, **LOAD−** to GND | Check **MLT-5020** current vs GPIO; transistor + base resistor if needed. |
| **LED1** | **R3** / **R14** / **R15** (RGB) | Per **`Azimuth.kicad_sch`**. |

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
