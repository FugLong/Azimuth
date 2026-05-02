# Parts list

Two sections: **DIY** (Seeed XIAO + BNO08x breakout) and the **Azimuth integrated PCB**. How to wire: [wiring.md](wiring.md). Which firmware: [hardware-profiles.md](hardware-profiles.md).

---

## DIY: Seeed XIAO + BNO08x (wired)

The **wired-only** build (breadboard or hand-wired, power from USB) is fine for **anyone comfortable with a soldering iron** or solderless breadboard.

| Item | Where to buy · notes |
|------|----------------------|
| **Seeed XIAO ESP32-C3** | [Seeed Studio](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) · [Mouser **113991054**](https://www.mouser.com/ProductDetail/Seeed-Studio/113991054?qs=3Rah4i%252BhyCHVBerMrpzCkw%3D%3D) — MCU + USB-C. Pinout and docs: [Seeed Wiki — XIAO ESP32-C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/). |
| **BNO08x breakout (SPI)** | Example modules: [Amazon (GY-BNO085)](https://a.co/d/0f5FjqAz) · [AliExpress listing](https://www.aliexpress.us/item/3256806188033662.html) — strapped for **SPI**; wire **PS0** to **GPIO2** (XIAO **D0**) with **10 kΩ** to **3V3** like **Azimuth_Design** **R14** — full map: [wiring.md](wiring.md). Other **BNO080 / BNO085 / BNO086** breakouts work if you set SPI straps per the vendor. |

Also: breadboard or perfboard, jumper wire, **USB data** cable to the PC.

**Firmware:** **`azimuth_main_diy`** — default on the [web flasher](https://fuglong.github.io/Azimuth/).

### Optional: battery on the XIAO (wireless)

**DIY path only** — there is **no** Azimuth **JST PH** connector on this build.

Soldering a **1S LiPo** to the **battery pads on the back of the XIAO** is the add-on that enables **unplugged / wireless** use (same Wi‑Fi + UDP firmware path; see [power-and-thermal.md](power-and-thermal.md#battery-runtime-planning) for rough runtime). **Recommended for intermediate makers** — the USB-only build above is the approachable path for beginners.

The XIAO includes **charge management**; after you connect the pack correctly, routine charge/discharge is handled on-board. **Match +/− to Seeed’s pad labels** on the XIAO. You can solder leads directly or attach your own **JST-PH** pigtail—**you** are responsible for polarity on those pads. **Never short the battery leads** together (even briefly).

The **JST PH polarity rules** in [Off-board pack (Azimuth PCB)](#off-board-pack-azimuth-pcb-wireless) below apply to the **integrated Azimuth PCB**, not to the XIAO’s solder pads.

---

## Azimuth PCB (`Azimuth_Design`)

**Manufacturing:** **V0.1** — design finalized; **assembled boards received**; **bring-up / testing in progress**. Status: **[roadmap](roadmap.md)**.

Integrated board: **`kicad/Azimuth_Design/`**. Values and footprints below match **`Azimuth.kicad_sch`** / **`Azimuth.kicad_pcb`** in the repo (**ERC/DRC clean** in the current snapshot — re-run both after edits). **Tracked BOM (KiCad CLI):** [`kicad/Azimuth_Design/fab/Azimuth_bom.csv`](../kicad/Azimuth_Design/fab/Azimuth_bom.csv) — regenerate with **`./scripts/export_azimuth_bom.sh`** after schematic edits (that script also refreshes **[`Azimuth_bom_cost.txt`](../kicad/Azimuth_Design/fab/Azimuth_bom_cost.txt)** — **1× board, parts-only USD** ballpark from symbol **`Unit Price (USD)`** or the LCSC table in **`scripts/update_azimuth_sch_bom_fields.py`**). Board workflow: [kicad.md](kicad.md).

To sync **unit prices** onto schematic instances before exporting the BOM: `python3 scripts/update_azimuth_sch_bom_fields.py` (manual snapshot; not live LCSC).

### PCB / assembly quote — “Other Parameters” (PCBWay-style)

Use these for **instant quote** fields that ask for **unique parts**, **SMD**, **BGA/QFP**, and **THT** (you can leave blanks for a manual quote). Numbers match the committed BOM **`fab/Azimuth_bom.csv`** and **`Azimuth.kicad_pcb`** in this repo.

| Form field | Value | Notes |
|------------|------|--------|
| **Number of Unique Parts** (components) | **33** | Distinct order lines: **32** different **LCSC Part** codes on stuffed lines + **1** line with no LCSC (**IC1** BNO086). |
| **Number of SMD Parts** (tooltip: *SMT Pads*) | **52** |
| **Number of BGA/QFP Parts** (SMT BGA/QFP) | **0** | **IC1** is **LGA**, not BGA/QFP. |
| **Number of Through-Hole Parts** | **0** | All stuffed parts are surface-mount. |

Source of truth: [`fab/README.md`](../kicad/Azimuth_Design/fab/README.md).

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
| 1 | **Q3** | AO3401A | `1_MyFootPrints:SOT-23-3_L2.9-W1.3-P1.90-LS2.4-BR` | Reverse-polarity PMOS on battery input (LCSC **C347476**) |
| 1 | **D1** | MBR0530_C5204746 | `1_MyFootPrints:SOD-123_L2.7-W1.6-LS3.7-RD-1` | Schottky |
| 1 | **F1** | 0805L050WR | `1_MyFootPrints:R0805` | Fuse |
| 1 | **LED1** | TZ-P4-1615RGBTCA1-0.55T | `rgb_led_C779813:LED-SMD_4P-L1.6-W1.5-BR_TZ-P4-1615` | RGB; **COM+** to **3V3** (common anode) |
| 1 | **CHG1** | XL-0201SURC | `1_MyFootPrints:LED-SMD_L0.65-W0.35-P0.41_XL-0201SURC` | Charge indicator LED |
| 1 | **BUZZER1** | MLT-5020 | `1_MyFootPrints:BUZ-SMD_L5.0-W5.5-P4.60` | Passive magnetic buzzer (LCSC **C94598**) — **+**→**3V3**, **−**→**Q2** drain; see [Buzzer (BUZZER1)](#buzzer-buzzer1) |
| 1 | **Q2** | AO3400A | `1_MyFootPrints:SOT-23-3_L2.9-W1.3-P1.90-LS2.4-BR` | Low-side NMOS for **BUZZER1** (LCSC **C20917**); gate from **IO21** via **R20** (same cell as **`ao3400_C20917`** in libs) |
| 1 | **D2** | B5819WS | `1_MyFootPrints:SOD-323_L1.8-W1.3-LS2.5-RD` | Flyback Schottky (LCSC **C64886**) — **cathode**→**3V3**/**BUZZER1 +**, **anode**→drain/**BUZZER1 −** (same cell as **`b5819ws_C64886`** in libs) |
| 1 | **FUNC1** | HX 3X4X2-2P-1.6N TACTILE SWITCH | `1_MyFootPrints:KEY-SMD_L4.0-W3.0-LS4.9-1` | User button |
| 1 | **RST1** | HX 3X4X2-2P-1.6N TACTILE SWITCH | `1_MyFootPrints:KEY-SMD_L4.0-W3.0-LS4.9-1` | Reset / strap (per schematic) |

Exact **LCSC** / **Manufacturer** fields live in the KiCad symbol properties.

### Off-board pack (Azimuth PCB, wireless)

**This section is for the integrated Azimuth PCB only** (`Azimuth_Design`). DIY XIAO users do not get this connector out of the box—they solder to the XIAO or add their own pigtail; the silkscreen and JST rules below are **for the custom PCB**.

The board exposes **JST PH2.0** (**`PH2.0`**) for a **1S** pack when you are not on USB. Use a **protected** cell where possible. Regulation, charging, and the full power path follow **`Azimuth.kicad_sch`** and [wiring.md](wiring.md) (PCB path).

#### Polarity (read this before plugging anything in)

**Adafruit and many other US hobby LiPo packs with JST-PH are wired the opposite way from this PCB.** Do **not** use Adafruit batteries (or any pack that follows the same red/black convention as Adafruit) with the Azimuth **PH2.0** connector—they **will not work** with this layout and **can damage the board or battery** if forced.

Just **behind** the **PH2.0** connector, the PCB silk shows **+** and **−**. The pack’s **red** lead must line up with the **+** marking. If your connector’s red wire would sit on the **−** side, **do not plug it in**—you need a pack wired like our reference vendor below, or you must re-pin / rewire the JST to match.

#### Recommended source and capacity

A known-good match for **JST pinout and polarity** is **YDL Battery** wiring. Example pack (check their site for current stock and specs):

- [**YDL — 3.7 V 800 mAh 802346 LiPo**](https://ydlbattery.com/products/3-7v-800mah-802346-lithium-polymer-ion-battery) — use packs that match **YDL’s** red/black orientation on **JST-PH** relative to the Azimuth **+ / −** silk.

**Capacity:** **~400 mAh to ~900 mAh** is the practical recommended range for this build (runtime vs size on the head). Other capacities can be fine electrically; still verify **JST polarity** against the silk **before** first connect.

**Charge rate:** **U3** (**TP4054**) is set for **~500 mA** via **R4** (**PROG**). Compare that to your cell’s **C** rating and datasheet. Respect **protected** packs and cell quality.

---

### Appendix: resistors (matches `Azimuth.kicad_sch`)

Default footprint for listed **R** parts: **`Resistor_SMD:R_0201_0603Metric`**. Exception: **F1** uses **`1_MyFootPrints:R0805`** (fuse body).

| Ref | Value (KiCad) | Notes |
|-----|---------------|--------|
| **R1** | 220K | Battery divider (tap to **GPIO4** with **R2**) |
| **R2** | 220K | Battery divider (**VBAT_SW** → **GPIO4** tap → **GND**) |
| **R3** | 100k | Power / **U2** support (per schematic nets) |
| **R4** | 2K | **U3** **PROG** — **TP4054**: **I_BAT ≈ 1000 / R4(kΩ) mA** → **~500 mA** |
| **R5** | 150R | **CHG1** LED current limit (**3V3** side) |
| **R6** | 220R | **LED1** blue cathode ↔ **IO3** |
| **R7** | 100R | **LED1** red cathode ↔ **IO0** |
| **R8** | 68R | **LED1** green cathode ↔ **IO1** |
| **R9** | 5.1K | **J1** **CC** (e.g. CC2 branch) |
| **R10** | 5.1K | **J1** **CC** (e.g. CC1 branch) |
| **R11** | 22R | USB data series (**U1** ↔ **J1** **D+** path) |
| **R12** | 22R | USB data series (**U1** ↔ **J1** **D−** path) |
| **R13** | 10k | **IC1** strap (per schematic) |
| **R14** | 10k | **PS0/WAKE** pull-up to **3V3** (**IO2**); shared with Espressif **GPIO2** strap |
| **R15** | 10k | **IC1** **CLKSEL0** pull-up to **3V3** |
| **R16** | 4.7k | **IC1** support (per schematic) |
| **R17** | 4.7k | **IC1** support (per schematic) |
| **R18** | 10k | **U2** / power (per schematic) |
| **R19** | 10k | **U2** / power (per schematic) |
| **R20** | 330R | **IO21** → **Q2** gate (series) |
| **R21** | 100k | **Q2** gate → **GND** (pull-down) |
| **R22** | 100k | **Q3** gate → **GND** (reverse-polarity PMOS bias) |

---

### Appendix: capacitors (matches `Azimuth.kicad_sch`)

Default footprint: **`Capacitor_SMD:C_0201_0603Metric`**.

| Ref | Value (KiCad) | Role (verify nets in schematic) |
|-----|---------------|----------------------------------|
| **C1** | 10uf | Battery / switched rail |
| **C2** | 0.1uf | HF bypass |
| **C3** | 100nF | **USB_5V** (post-fuse **VBUS**) HF bypass to **GND** — not the IMU rail |
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
| **BNO086 (IC1)** | **C11** on **CAP**; **3V3** decoupling via shared rail (**C5**, **C9**, **C10**, **C12** per schematic); **R13**–**R17** straps / **ENV** | **R14** 10 kΩ **PS0** (**IO2**); **R15** 10 kΩ **CLKSEL0**; **R16**/**R17** 4.7 kΩ **ENV**; see **`Azimuth.kicad_sch`** |
| **USB-C (J1) / USB_5V** | **C3** 100 nF (**USB_5V** bypass); **R9**, **R10** 5.1 kΩ (**CC**); **R11**, **R12** 22 Ω (data series) | **6-pin** footprint vs 16-pin symbol may produce **DRC** “no pad for pin” on extra **GND** pins — electrically rely on shell / pour if intentional |
| **RGB (LED1)** | **R6** 220 Ω (**IO3** blue), **R7** 100 Ω (**IO0** red), **R8** 68 Ω (**IO1** green) | **COM+** on **3V3** — firmware should treat cathode pins as **active low** (sink); values tuned for **TZ-P4-1615RGBTCA1** on **3V3** |
| **Charge LED (CHG1)** | **R5** 150 Ω | From **3V3** to **CHG1** anode net |
| **Battery path** | **PH2.0**, **PWR1**, **F1**, **C1**, **C2**, **R1**, **R2** | Divider tap → **IO4** (ADC); **PS0** on **IO2** via **R14** (see [wiring.md](wiring.md)) |
| **3.3 V regulation** | **U2**, **Q1**, **D1**, **R3**, **R18**, **R19**, **C4**, **C5**, **C8**, **C9**, **C10**, **C12** | Exact nets: **`Azimuth.kicad_sch`** |
| **Charging** | **U3**, **R4**, **C6**, **C7** | **R4** = **2 kΩ** on **PROG** → **~500 mA** (see [Off-board pack](#off-board-pack-azimuth-pcb-wireless)) |
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
