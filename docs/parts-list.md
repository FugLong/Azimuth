# Parts list

Two sections: **DIY** (Seeed XIAO + BNO08x breakout) and the **Azimuth integrated PCB**. How to wire: [wiring.md](wiring.md). Which firmware: [hardware-profiles.md](hardware-profiles.md).

---

## DIY: Seeed XIAO + BNO08x (wired)

The **wired-only** build (breadboard or hand-wired, power from USB) is fine for **anyone comfortable with a soldering iron** or solderless breadboard.

| Item | Where to buy · notes |
|------|----------------------|
| **Seeed XIAO ESP32-C3** | [Seeed Studio](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) · [Mouser **113991054**](https://www.mouser.com/ProductDetail/Seeed-Studio/113991054?qs=3Rah4i%252BhyCHVBerMrpzCkw%3D%3D) — MCU + USB-C. Pinout and docs: [Seeed Wiki — XIAO ESP32-C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/). |
| **BNO08x breakout (SPI)** | Example modules: [Amazon (GY-BNO085)](https://a.co/d/0f5FjqAz) · [AliExpress listing](https://www.aliexpress.us/item/3256806188033662.html) — strapped for **SPI**; follow the board’s **PS0 / PS1** and wire to the XIAO per [wiring.md](wiring.md) (DIY path). Other **BNO080 / BNO085 / BNO086** breakouts work if you set SPI straps per the vendor. |

Also: breadboard or perfboard, jumper wire, **USB data** cable to the PC.

**Firmware:** **`azimuth_main_diy`** — default on the [web flasher](https://fuglong.github.io/Azimuth/).

### Optional: battery on the XIAO (wireless)

Soldering a **1S LiPo** to the **battery pads on the back of the XIAO** is the only DIY add-on that enables **unplugged / wireless** use (same Wi‑Fi + UDP firmware path; see [using-azimuth.md](using-azimuth.md#power-heat-and-battery) for rough runtime). **Recommended for intermediate makers** — the USB-only build above is the approachable path for beginners.

The XIAO includes **charge management**; after you connect the pack correctly, routine charge/discharge is handled on-board. **Be careful soldering** the battery leads to the pads (**polarity** per Seeed’s silk/wiki). **Never short the battery leads** together (even briefly).

Example **JST-PH** 1S packs (check **polarity** against your connector before powering):

| | |
|:---|:---|
| **Adafruit** [**3898**](https://www.adafruit.com/product/3898) | 3.7 V nominal **400 mAh**, JST-PH; ~**36 × 17 × 7.8 mm** (vendor) |
| **DigiKey** [**1528-2731-ND**](https://www.digikey.com/en/products/detail/adafruit-industries-llc/3898/9685336) | Same MPN **3898** |

---

## Azimuth PCB (`Azimuth_Design`)

Integrated board: **`kicad/Azimuth_Design/`**. Values and footprints below match **`Azimuth.kicad_sch`** / **`Azimuth.kicad_pcb`** in the repo (**ERC/DRC clean** in the current snapshot — re-run both after edits). For each order, **export the BOM from KiCad** so pick-and-place and stock codes stay in sync. Board workflow: [kicad.md](kicad.md).

### What’s on the board (by reference)

| Qty | Ref | Value / MPN (schematic) | Footprint (project) | Notes |
|-----|-----|-------------------------|---------------------|--------|
| 1 | **U1** | ESP32-C3-WROOM-02-N4(4MB) | `1_MyFootPrints:BULETM-SMD_ESPRESSIF_ESP32-C3-WROOM-02-N4-4MB` | MCU; USB on module |
| 1 | **IC1** | BNO086 | `Package_LGA:LGA-28_5.2x3.8mm_P0.5mm` | IMU |
| 1 | **U2** | TLV75733PDBVR | `1_MyFootPrints:SOT-23-5_L2.9-W1.6-P0.95-LS2.8-BL` | 3.3 V LDO |
| 1 | **U3** | TP4054_C5381776 | `1_MyFootPrints:SOT-23-5_L2.9-W1.6-P0.95-LS2.8-BR` | Li-ion charger |
| 1 | **J1** | TYPE-C 16PIN 2MD(073) | `1_MyFootPrints:USB-C-SMD_TYPE-C-6PIN-2MD-073` | USB-C |
| 1 | **PH2.0** | S2B-PH-SM4-TB(LF)(SN) | `Connector_JST:JST_PH_S2B-PH-SM4-TB_1x02-1MP_P2.00mm_Horizontal` | Battery |
| 1 | **PWR1** | K3-1280S-K1 | `1_MyFootPrints:SW-SMD_K3-1280S-K1` | Slide switch |
| 1 | **Q1** | LP0404N3T5G | `1_MyFootPrints:SOT-883-3_L1.0-W0.6-BR` | Power path FET |
| 1 | **D1** | MBR0530_C5204746 | `1_MyFootPrints:SOD-123_L2.7-W1.6-LS3.7-RD-1` | Schottky |
| 1 | **F1** | 0805L050WR | `1_MyFootPrints:R0805` | Fuse |
| 1 | **LED1** | TZ-P4-1615RGBTCA1-0.55T | `rgb_led_C779813:LED-SMD_4P-L1.6-W1.5-BR_TZ-P4-1615` | RGB; **COM+** to **3V3** (common anode) |
| 1 | **CHG1** | XL-0201SURC | `1_MyFootPrints:LED-SMD_L0.65-W0.35-P0.41_XL-0201SURC` | Charge indicator LED |
| 1 | **BUZZER1** | MLT-5020 | `1_MyFootPrints:BUZ-SMD_L5.0-W5.5-P4.60` | Passive magnetic buzzer (LCSC **C94598**) — **+**→**3V3**, **−**→**Q2** drain; see [Buzzer (BUZZER1)](#buzzer-buzzer1) |
| 1 | **Q2** | AO3400A | `ao3400_C20917:SOT-23-3_L2.9-W1.3-P1.90-LS2.4-BR` | Low-side NMOS for **BUZZER1** (LCSC **C20917**); gate from **IO21** via **R20** |
| 1 | **D2** | B5819WS | `b5819ws_C64886:SOD-323_L1.8-W1.3-LS2.5-RD` | Flyback Schottky (LCSC **C64886**) — **cathode**→**3V3**/**BUZZER1 +**, **anode**→drain/**BUZZER1 −** |
| 1 | **FUNC1** | HX 3X4X2-2P-1.6N TACTILE SWITCH | `1_MyFootPrints:KEY-SMD_L4.0-W3.0-LS4.9-1` | User button |
| 1 | **RST1** | HX 3X4X2-2P-1.6N TACTILE SWITCH | `1_MyFootPrints:KEY-SMD_L4.0-W3.0-LS4.9-1` | Reset / strap (per schematic) |
| 2 | **BAT_0**, **GND_0** | DNP | `TestPoint:TestPoint_Pad_D1.0mm` | Optional test points (not populated by default) |

Exact **LCSC** / **Manufacturer** fields live in the KiCad symbol properties.

### Off-board pack (PCB wireless use)

The PCB exposes **JST PH** for a **1S** pack when you are not on USB. Use a **protected** cell and correct **polarity** for **`PH2.0`** in **`Azimuth.kicad_sch`**. The same **class** of off-the-shelf **JST-PH** 1S packs as in [Optional: battery on the XIAO (wireless)](#optional-battery-on-the-xiao-wireless) usually applies—always confirm against **this** footprint.

**Capacity guidance:** Treat **~400 mAh** as a practical **minimum** for this build (runtime vs mass on the head). **~850 mAh** is a comfortable “sweet spot” when you want more time without a large pack; anything **above 400 mAh** is electrically fine. Lighter smaller packs trade runtime for comfort.

**Charge rate:** **U3** (**TP4054**) is set for **~500 mA** fast charge via **R4** (**PROG**). For a **400 mAh** cell that is a little over **1 C**; for **850 mAh** it is **under 0.6 C**. That is conservative compared with how small **1S** packs are often used in RC/drone contexts, but still respect your cell’s datasheet, quality, and **protected** packs only.

Regulation, charging, and the full power path follow **`Azimuth.kicad_sch`** and [wiring.md](wiring.md) (PCB path).

---

### Appendix: resistors (matches `Azimuth.kicad_sch`)

Default footprint for listed **R** parts: **`Resistor_SMD:R_0201_0603Metric`**. Exception: **F1** uses **`1_MyFootPrints:R0805`** (fuse body).

| Ref | Value (KiCad) | Notes |
|-----|---------------|--------|
| **R1** | 220K | Battery divider to **GPIO2** |
| **R2** | 220K | Battery divider |
| **R3** | 100k | Power / **U2** support (per schematic nets) |
| **R4** | 2K | **U3** **PROG** — **TP4054**: **I_BAT ≈ 1000 / R4(kΩ) mA** → **~500 mA** |
| **R5** | 150R | **CHG1** LED current limit (**3V3** side) |
| **R6** | 680R | **LED1** cathode ↔ **IO3** |
| **R7** | 150R | **LED1** cathode ↔ **IO0** |
| **R8** | 150R | **LED1** cathode ↔ **IO1** |
| **R9** | 5.1K | **J1** **CC** (e.g. CC2 branch) |
| **R10** | 5.1K | **J1** **CC** (e.g. CC1 branch) |
| **R11** | 22R | USB data series (**U1** ↔ **J1** **D+** path) |
| **R12** | 22R | USB data series (**U1** ↔ **J1** **D−** path) |
| **R13** | 10k | **IC1** strap (per schematic) |
| **R14** | 10k | **IC1** **ENV_SCL** pull-up |
| **R15** | 10k | **IC1** **ENV_SDA** pull-up |
| **R16** | 4.7k | **IC1** support (per schematic) |
| **R17** | 4.7k | **IC1** support (per schematic) |
| **R18** | 10k | **U2** / power (per schematic) |
| **R19** | 10k | **U2** / power (per schematic) |
| **R20** | 330R | **IO21** → **Q2** gate (series) |
| **R21** | 100k | **Q2** gate → **GND** (pull-down) |

---

### Appendix: capacitors (matches `Azimuth.kicad_sch`)

Default footprint: **`Capacitor_SMD:C_0201_0603Metric`**.

| Ref | Value (KiCad) | Role (verify nets in schematic) |
|-----|---------------|----------------------------------|
| **C1** | 10uf | Battery / switched rail |
| **C2** | 0.1uf | HF bypass |
| **C3** | 100nF | **IC1** / **3V3** decoupling |
| **C4** | 2.2uF | Power / **U2** / **U3** branch |
| **C5** | 2.2uF | Power / **U2** branch |
| **C6** | 2.2uF | **U3** / charger branch |
| **C7** | 2.2uF | **U3** / charger branch |
| **C8** | 10uF | Bulk (output / rail per nets) |
| **C9** | 10uf | Bulk (per nets) |
| **C10** | 0.1uf | HF bypass (per nets) |
| **C11** | 100nf | **IC1** **CAP** pin to **GND** |
| **C12** | 100nf | **U2** / rail (per nets) |

---

### Appendix: passives by block

| Block | Refs | Notes |
|-------|------|--------|
| **BNO086 (IC1)** | **C3**, **C11** on **CAP**; **R13**–**R17** straps / **ENV** | **R14**/**R15** 10 kΩ on **ENV_SCL** / **ENV_SDA**; **R16**/**R17** 4.7 kΩ per schematic |
| **USB-C (J1)** | **R9**, **R10** 5.1 kΩ (**CC**); **R11**, **R12** 22 Ω (data series) | **6-pin** footprint vs 16-pin symbol may produce **DRC** “no pad for pin” on extra **GND** pins — electrically rely on shell / pour if intentional |
| **RGB (LED1)** | **R6** 680 Ω (**IO3**), **R7**/**R8** 150 Ω (**IO0** / **IO1**) | **COM+** on **3V3** — firmware should treat cathode pins as **active low** (sink) |
| **Charge LED (CHG1)** | **R5** 150 Ω | From **3V3** to **CHG1** anode net |
| **Battery path** | **PH2.0**, **PWR1**, **F1**, **C1**, **C2**, **R1**, **R2** | Divider tap → **IO2** |
| **3.3 V regulation** | **U2**, **Q1**, **D1**, **R3**, **R18**, **R19**, **C4**, **C5**, **C8**, **C9**, **C10**, **C12** | Exact nets: **`Azimuth.kicad_sch`** |
| **Charging** | **U3**, **R4**, **C6**, **C7** | **R4** = **2 kΩ** on **PROG** → **~500 mA** (see [Off-board pack](#off-board-pack-pcb-wireless-use)) |
| **FUNC1** | Switch to **GND** | Firmware **`INPUT_PULLUP`** on **IO7** |
| **BUZZER1** + **Q2** + **D2** | **BUZZER1 +**→**3V3**, **−**→**Q2** drain; **Q2** source→**GND**; **IO21**→**R20**→gate, **R21** gate→**GND**; **D2** flyback | See [Buzzer (BUZZER1)](#buzzer-buzzer1) |

Long jumper wires on DIY builds: consider series resistors on **SPI** and a stronger **H_INTN** pull-up if needed (SparkFun driver can enable MCU-side pull-up).

---

### Buzzer (BUZZER1)

**MLT-5020** (LCSC **C94598**) is a **magnetic, passive** buzzer: drive it with **PWM** on the switched rail (vary **frequency** for pitch). Coil current can be on the order of **~100 mA** at **3 V** for this family—too much to hang directly on a GPIO.

**Azimuth_Design (`Azimuth.kicad_sch`) — as built**

- **High side:** **BUZZER1** pin **+** → **3V3**; pin **−** → **Q2** (**AO3400A**, LCSC **C20917**) **drain**; **Q2** **source** → **GND**.
- **Flyback:** **D2** **B5819WS** (LCSC **C64886**) — **cathode** (band) → **3V3** / buzzer **+**, **anode** → buzzer **−** / **Q2** drain (same node).
- **Gate drive:** **U1** **IO21** (symbol **TXD**) → **R20** (**330 Ω**) → **Q2** **gate**; **R21** (**100 kΩ**) **gate** → **GND** (default off when the pin is high-Z).

Firmware: **`ledcWriteTone()`** / **LEDC** PWM on **GPIO21** toggles **Q2** and sets the acoustic frequency. Leave **BUZZER1** pin **NC** unconnected unless the datasheet requires otherwise.

**DIY breadboard:** The table above is the **PCB** netlist. A simple **buzzer +** on **D6** with **−** to **GND** is *not* the same as the PCB (no transistor); use a transistor + diode there too if you mirror this design, or accept weak / risky direct drive.
