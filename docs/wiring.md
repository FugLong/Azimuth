# Wiring — Seeed XIAO ESP32-C3 ↔ BNO08x (SPI)

This document matches the pin assignments used in `src/main.cpp` for the **Azimuth** firmware (SparkFun BNO08x Arduino library, `beginSPI`).

## BNO08x mode

The IMU must be in **SPI** mode: on the breakout, tie **PS0** and **PS1** to **3.3 V** (per your board’s datasheet). Power the sensor from **3.3 V** and connect **GND** to the XIAO ground.

Some breakouts label SPI pins using I²C-style names (e.g. SCL/SDA); use the vendor’s SPI column in their table, not the I²C names.

## Signal map

| BNO08x / function | Seeed XIAO ESP32-C3 | ESP32-C3 GPIO | Notes |
|-------------------|---------------------|---------------|--------|
| **SCK** | **D8** | GPIO8 | Hardware SPI clock (FSPI `SCK` in `pins_arduino.h`) |
| **MISO** (SO / CIPO) | **D9** | GPIO9 | Data from sensor |
| **MOSI** (SI / COPI) | **D10** | GPIO10 | Data to sensor |
| **CS** (chip select) | **D2** | GPIO4 | GPIO CS; library controls it in software |
| **H_INT** (interrupt) | **D3** | GPIO5 | Active low; internal pull-up configured in firmware |
| **RST** (reset) | **D7** | GPIO20 | Active-low reset from library |
| **3V3** | 3V3 | — | Same rail as mode pins if PS0/PS1 go to 3.3 V |
| **GND** | GND | — | Common ground |

## Diagram (logical)

```
BNO08x breakout          XIAO ESP32-C3
─────────────────        ───────────────
SCK              ───── D8  (SCK)
MISO / SO        ───── D9  (MISO)
MOSI / SI        ───── D10 (MOSI)
CS               ───── D2
H_INT            ───── D3
RST              ───── D7
3V3, GND         ───── 3V3, GND
PS0, PS1         ───── 3V3  (SPI mode)
```

## Pinout reference (XIAO ESP32-C3)

Arduino core defines for this board include:

- `SCK` = GPIO8 (D8), `MISO` = GPIO9 (D9), `MOSI` = GPIO10 (D10)
- D2 = GPIO4, D3 = GPIO5, D7 = GPIO20

Firmware uses these GPIO numbers in `kPinCs`, `kPinInt`, and `kPinRst` in `src/main.cpp`.

## D7 / GPIO20 note

On this XIAO variant, **D7** is **GPIO20**, which is also the **UART RX** pin for `Serial` on some schematics. Using **D7 for BNO08x RST** is fine for this firmware. If you later need **hardware UART RX** on that pin at the same time, move RST to another free GPIO and update the three `kPin*` constants in `src/main.cpp`.

## Electrical

- SPI bus: keep **SCK** and **INT** leads short and solid grounds for clean communication at 3 MHz.
- If initialization fails: confirm **PS0/PS1 → 3.3 V**, **INT** and **RST** wired as above, **common ground**, then power-cycle.
