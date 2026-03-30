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

Integrated board: **`kicad/Azimuth_Design/`**. **Authoritative BOM** = export from **`Azimuth.kicad_pro`** (or your fab tool) after **ERC** / **DRC**. Line items, refs, and footprints must match **`Azimuth.kicad_sch`** / **`Azimuth.kicad_pcb`**. Board layout and workflow: [kicad.md](kicad.md).

### What’s on the board

| Qty | Ref | Part / value | Notes |
|-----|-----|----------------|--------|
| 1 | **U1** | **ESP32-C3-WROOM-02** (e.g. **-N4** flash) | MCU module; USB via module pads |
| 1 | **IC1** | **BNO086** | IMU (**LGA**); often bottom side in layout |
| 1 | **PH2.0** | JST **PH** 2-pin | Battery connector if populated — pin 1 **`/Bat+`** per schematic |
| 1 | **PWR1** | **K3-1280S-K1** | Slide switch on battery path |
| 1 | **LED1** | **TZ-P4-1615RGBTCA1** (or sch. value) | RGB — [wiring.md](wiring.md) |
| 1 | **BUZZER1** | **MLT-5020** | Magnetic buzzer |
| 1 | **FUNC1** | Tact switch (e.g. **HX** 3×4×2) | User button |
| 1 | **J1** | USB-C (per schematic) | Data + power |

Exact **MPNs**, **LCSC** codes, and **footprint** strings live in the KiCad project.

### Off-board pack (PCB wireless use)

The PCB may expose **JST PH2.0** for a **1S** pack when you are not on USB. Use a **protected** cell and correct **polarity** for **`PH2.0`** in **`Azimuth.kicad_sch`**. The same **class** of off-the-shelf **JST-PH** 1S packs as in [Optional: battery on the XIAO (wireless)](#optional-battery-on-the-xiao-wireless) usually applies—always confirm against **this** footprint.

Regulation, charging, and the full power path are **board-specific**; follow the schematic and [wiring.md](wiring.md) (PCB path).

---

### Appendix: resistors (reference)

Values and footprints match **`Azimuth.kicad_sch`** (KiCad strings e.g. **220K**, **680R**, **0.1uf**).

| Qty | Ref | Value | Footprint |
|-----|-----|--------|-----------|
| 2 | **R1**, **R2** | **220 kΩ** (`220K`) | `Resistor_SMD:R_0603_1608Metric` |
| 1 | **R3** | **680 Ω** (`680R`) | `Resistor_SMD:R_0402_1005Metric` |
| 2 | **R4**, **R5** | **150 Ω** (`150R`) | `Resistor_SMD:R_0402_1005Metric` |
| 2 | **R6**, **R7** | **5.1 kΩ** (`5.1K`) | `Resistor_SMD:R_0402_1005Metric` |
| 2 | **R8**, **R9** | **22 Ω** (`22R`) | `Resistor_SMD:R_0402_1005Metric` |
| 4 | **R10**, **R11**, **R12**, **R13** | **10 kΩ** (`10k`) | `Resistor_SMD:R_0402_1005Metric` |
| 2 | **R14**, **R15** | **4.7 kΩ** (`4.7k`) | `Resistor_SMD:R_0402_1005Metric` |

**R1 / R2** — divider from switched battery rail to **GND**, tap **GPIO2** (ADC). **R3**–**R5** — **LED1** RGB. **R6** / **R7** — USB-C **CC**. **R8** / **R9** — USB data series. **R10** — per schematic net. **R11** — **BOOTN** strap. **R12** — **PS0/WAKE**. **R13** — **CLKSEL0**. **R14** / **R15** — **ENV_SCL** / **ENV_SDA**.

---

### Appendix: capacitors (reference)

| Qty | Ref | Value | Footprint | Role (nets in schematic) |
|-----|-----|--------|-----------|---------------------------|
| 1 | **C1** | **10 µF** (`10uf`) | `Capacitor_SMD:C_0603_1608Metric` | Bulk on switched rail → **GND** |
| 1 | **C2** | **0.1 µF** (`0.1uf`) | `Capacitor_SMD:C_0603_1608Metric` | HF bypass near divider / rail |
| 1 | **C3** | **100 nF** (`100nf`) | `Capacitor_SMD:C_0603_1608Metric` | **3.3 V** / **IC1** |
| 1 | **C4** | **10 µF** (`10uf`) | `Capacitor_SMD:C_0603_1608Metric` | Bulk on **3.3 V** |
| 1 | **C5** | **0.1 µF** (`0.1uf`) | `Capacitor_SMD:C_0603_1608Metric` | **U1** decoupling |
| 1 | **C6** | **100 nF** (`100nf`) | `Capacitor_SMD:C_0402_1005Metric` | **BNO086 `CAP`** → **GND** |

---

### Appendix: passives by block

| Block | On PCB | Notes |
|-------|--------|--------|
| **BNO086** | **C3**, **C4**; **C6** on **`CAP`**; **R11**–**R13**; **R14**/**R15** on **ENV** | No series RC on SPI for short traces. |
| **USB-C J1** | **R6**/**R7** (CC), **R8**/**R9** (D+/D−) | Per schematic. |
| **Battery path** | **`/Bat+`** → **PH2.0** → **PWR1**; **C1**/**C2**; **R1**/**R2** | Nets in **`Azimuth.kicad_sch`**. |
| **FUNC1** | Switch to **GND** | Firmware **`INPUT_PULLUP`**. |
| **BUZZER1** | **LOAD+** to GPIO, **LOAD−** to GND | **MLT-5020** current vs GPIO—transistor if needed. |
| **LED1** | **R3** / **R4** / **R5** | Per schematic. |

Long jumper wires later: consider series resistors on **SPI** and a stronger **H_INTN** pull-up if needed (SparkFun driver can enable MCU-side pull-up).
