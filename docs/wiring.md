# Wiring and pinout

GPIO and nets follow **`kicad/Azimuth_Design/Azimuth.kicad_sch`**; this page and **`include/azimuth_hw.h`** match that project.

Assignments match **`include/azimuth_hw.h`**. PlatformIO environments: [**hardware-profiles.md**](hardware-profiles.md).

Pinout is split into **two paths** below—follow **either** the DIY (XIAO) section **or** the PCB (U1) section; they describe the same GPIO map in the form you need for that build.

---

## DIY path: Seeed XIAO + BNO08x breakout

For a **breadboard or hand-wired** setup: **Seeed XIAO ESP32-C3** + **BNO08x** breakout in **SPI** mode. Flash **`azimuth_*_diy`**. USB to the PC is the XIAO’s USB-C.

### XIAO pins (D → GPIO)

Per Seeed’s [XIAO ESP32-C3 pin map](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/).

| XIAO | GPIO | Typical use in Azimuth |
|------|------|-------------------------|
| D0 | **2** | **PS0 / WAKE** — wire to IMU **PS0**; add **10 kΩ** to **3V3** (same as **Azimuth_Design** **R14** on **IO2**) |
| D1 | **3** | — |
| D2 | **4** | Battery divider tap if you add a sense network (same net as **Azimuth_Design** **IO4**) |
| D3 | **5** | IMU CS |
| D4 | **6** | IMU INT |
| D5 | **7** | FUNC (optional tact switch) |
| D6 | **21** | Buzzer drive / **TXD** (optional); on **PCB**, **IO21** → FET gate — not buzzer **+**; also UART TX |
| D7 | **20** | IMU NRST; same pin as UART RX—do not use both |
| D8 | **8** | SPI SCK |
| D9 | **9** | SPI MISO |
| D10 | **10** | SPI MOSI |

**Strapping:** On ESP32-C3, **GPIO2**, **GPIO8**, and **GPIO9** can affect boot. **DIY** matches the PCB pin plan (**PS0** on **GPIO2**, battery sense on **GPIO4**). **D8–D10** are SPI—keep IMU power/straps valid at reset.

### IMU breakout → XIAO

Map breakout pins by **function** (your silk may differ from **BNO086** names):

| IMU function | GPIO | XIAO | `azimuth_hw.h` |
|--------------|------|------|----------------|
| CS | 5 | D3 | `kPinImuCs` |
| INT | 6 | D4 | `kPinImuInt` |
| NRST | 20 | D7 | `kPinImuRst` |
| PS0 / WAKE | 2 | D0 | `kPinImuPs0Wake` — **10 kΩ** to **3V3** recommended (matches **R14** on PCB) |
| SCK / MISO / MOSI | 8 / 9 / 10 | D8 / D9 / D10 | FSPI |
| VDD, VDDIO, PS1 | — | 3V3 | SPI straps (see breakout doc) |
| GND | — | GND | |

Arduino **FSPI** defaults match **D8 / D9 / D10** on this XIAO. Tie **PS0** / **PS1** per your breakout’s datasheet (often **3.3 V** for SPI).

If you use different pins for SPI, call **`SPI.begin(sck, miso, mosi, -1)`** before **`imu.beginSPI(...)`**.

**Optional on breadboard:** tact switch **D5** (GPIO7). A buzzer on **D6** (GPIO21) is only **pin-compatible** with the PCB net name—the **integrated PCB** drives a **low-side FET** gate from **IO21**, not the buzzer **+** directly; see [PCB path](#pcb-path-azimuth_design-u1-esp32-c3-wroom-02) and [parts-list — Buzzer](parts-list.md#buzzer-buzzer1). There is no Azimuth-style RGB on the default DIY path.

---

## PCB path: Azimuth_Design (U1 ESP32-C3-WROOM-02)

For the integrated PCB: schematic **U1** is **`ESP32-C3-WROOM-02`** (e.g. **-N4**). Flash **`azimuth_*_pcb`**. Route **BNO086** and peripherals by **GPIO number** and **U1 symbol pin names** in **`Azimuth.kicad_sch`**—**not** by XIAO **D** labels (the module does not have those).

### BNO086 (IC1) ↔ U1

| BNO086 signal | U1 symbol pin | GPIO |
|---------------|---------------|------|
| **H_CSN** (CS) | **IO5** | 5 |
| **H_INTN** | **IO6** | 6 |
| **NRST** | **RXD** (UART0 RX) | 20 |
| **PS0/WAKE** | **IO2** | 2 |
| **H_SCL / SCK** | **IO8** | 8 |
| **H_SDA / MISO** | **IO9** | 9 |
| **SA0 / H_MOSI** | **IO10** | 10 |
| **VDD**, **VDDIO**, **PS1** | **3V3** | — |
| **GND** | **GND** | — |

On the board, **IC1** is the bare **BNO086**; **`Azimuth.kicad_sch`** shows straps and passives (**R13**–**R17**, **C11** on **CAP**, shared **3V3** bypass, etc.).

### Other nets ↔ U1

| Function | U1 symbol | GPIO | Notes |
|----------|-----------|------|--------|
| RGB **LED1** cathodes (via **R6** / **R7** / **R8**); **COM+** → **3V3** | **IO0**, **IO1**, **IO3** | 0, 1, 3 | **TZ-P4-1615RGB** — **common anode**; drive cathode nets **active low** in firmware; silk channel map on PCB |
| **FUNC1** | **IO7** | 7 | Tact to GND; firmware pull-up |
| **Buzzer (PWM)** | **TXD** | 21 | **IO21** → **R20** (330 Ω) → **Q2** (**AO3400A**) gate; **R21** (100 kΩ) gate → **GND** |
| **BUZZER1** + **Q2** + **D2** | — | — | **MLT-5020**: **+**→**3V3**, **−**→**Q2** drain; **Q2** source→**GND**; **D2** **B5819WS** flyback (**K**→**3V3**/**+**, **A**→drain/**−**) — [parts-list — Buzzer](parts-list.md#buzzer-buzzer1) |
| Battery divider tap | **IO4** | 4 | **R1**/**R2** (220 kΩ each), **`VBAT_SW`** → tap → **GND** per **`Azimuth.kicad_sch`**; **DIY:** same GPIO (**D2** on XIAO) (**GPIO4** is **not** a strapping pin) |

### Strap pins (ESP32-C3-WROOM-02)

Per the **ESP32-C3-WROOM-02** datasheet (*Boot configurations*, Table 4-3), **GPIO2**, **GPIO8**, and **GPIO9** set **chip boot mode** at reset. Strapping is **latched** when **`CHIP_EN`** rises; pins then behave as normal GPIO. Default pin bias (Table 4-1): **GPIO9** has a **weak internal pull-up** (reads **1** if nothing else dominates); **GPIO2** and **GPIO8** default **floating**.

| Boot mode | GPIO2 | GPIO8 | GPIO9 |
|-----------|-------|-------|-------|
| **SPI boot** (run from flash) | **1** | any | **1** |
| **Joint download boot** (UART0 / USB-Serial-JTAG) | **1** | **1** | **0** |

The same chapter notes that **GPIO2** does **not** choose between SPI boot and joint download (that is **GPIO8**/**GPIO9**), but **Espressif still recommends pulling GPIO2 up** to reduce glitch sensitivity. For **ROM message printing** to UART0, **GPIO8** also participates (see Table 4-4 in the datasheet); default eFuse settings usually match “normal” development expectations.

**How this maps on Azimuth**

- **GPIO9** (**SPI MISO** / **`/IO9`**) — Table 4-3 requires **1** for **SPI boot**. The chip’s **weak pull-up** on **GPIO9** usually wins when the IMU tri-states **MISO** with **CS** deasserted; that matches typical SPI flash boot.
- **GPIO8** (**SPI SCK** / **`/IO8`**) — For **SPI boot**, **GPIO8** may be **any** level; your IMU strapping and SPI idling are consistent with **not** forcing joint download by accident (avoid **GPIO8 = 0** together with **GPIO9 = 0**, which the datasheet calls **invalid** for some modes).
- **GPIO2** (**PS0/WAKE** + pull-up) — **`Azimuth.kicad_sch`** uses **R14** (**10 kΩ**) to **3V3**; breadboard **DIY** should duplicate that on **D0**. Battery sense is on **GPIO4** (**D2** on XIAO), not a strapping pin.

**USB:** Data on **GPIO18** / **GPIO19** inside the module to the USB pads—no separate USB-UART IC. Firmware uses **USB CDC** for **`Serial`**.

**Power / reset:** **EN** RC per Espressif; **3V3** / **GND** decoupling per **`Azimuth.kicad_sch`** and [ESP32-C3 hardware design guidelines](https://www.espressif.com/sites/default/files/documentation/esp32-c3_hardware_design_guidelines_en.pdf). Follow the WROOM-02 datasheet for **BOOT** / strap pins. **Antenna:** keep the module RF keepout clear.

### Battery (PCB)

**JST PH2.0**, slide switch **PWR1**, switched rail, divider **R1**/**R2** to **IO4** (ADC), bulk **C1**, bypass **C2**—exact nets in **`Azimuth.kicad_sch`**. Regulation and charging follow the schematic. Charger **U3** (**TP4054**) uses **R4** = **2 kΩ** on **PROG** for **~500 mA** charge current; pack sizing notes: [parts-list — Off-board pack](parts-list.md#off-board-pack-pcb-wireless-use).

### Schematic passives (`Azimuth.kicad_sch`)

Summary for bring-up; full tables match **`Azimuth.kicad_sch`** in [**parts-list.md**](parts-list.md).

| Block | Refs | Role |
|-------|------|------|
| **USB-C J1** | **R9**, **R10** 5.1 kΩ (**CC**); **R11**, **R12** 22 Ω (USB data series **U1** ↔ **J1**) | |
| **U1** | Decoupling per schematic (e.g. **C5** and related nets) | See KiCad |
| **IC1** BNO086 | **C11** 100 nF on **CAP**; **3V3** rail caps per schematic (**C5**, **C9**, **C10**, **C12**); **R13** 10 kΩ; **R14** 10 kΩ (**PS0** on **IO2**); **R15** 10 kΩ (**CLKSEL0**); **R16**/**R17** 4.7 kΩ (**ENV**) | |
| **LED1** RGB | **R6** 680 Ω (**IO3**), **R7**/**R8** 150 Ω (**IO0** / **IO1**); **R5** 150 Ω is **CHG1**, not RGB | |
| **Charger / charge LED** | **U3** **TP4054**, **R4** 2 kΩ (**PROG**, **~500 mA**), **CHG1**, **R5**, **C6**, **C7** | |
| **Battery path** | **C1** 10 µF, **C2** 0.1 µF, **R1**/**R2** 220 kΩ divider | |
| **Buzzer** | **R20** 330 Ω (**IO21**→gate), **R21** 100 kΩ (gate→**GND**); **Q2** **AO3400A**; **D2** **B5819WS**; **BUZZER1** **MLT-5020** | |

---

## Bring-up

1. IMU: **PS1** and rails at **3.3 V**, **PS0** high at reset, **BOOTN** high, **CLKSEL0** high, **CAP** bypassed (**C11** on PCB per **`Azimuth.kicad_sch`**).
2. Short SPI traces; solid GND return.
3. Run **`azimuth_debug_diy`** or **`azimuth_debug_pcb`** to match your hardware; then **`azimuth_main_*`** for Wi‑Fi / OpenTrack.
4. If init fails: **3.3 V**, **NRST**, **INT**, SPI wiring; on PCB, confirm the built board matches **`Azimuth.kicad_sch`** / layout (re-run **ERC** / **DRC** in KiCad if you changed the design).
