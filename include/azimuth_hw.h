/**
 * Pin map for BNO08x (SPI) and board I/O.
 *
 * One GPIO map for both hardware paths (DIY XIAO + breakout and Azimuth_Design PCB):
 *   - DIY: Seeed XIAO ESP32-C3 + BNO08x breakout — wire the breakout to the same GPIOs as **`Azimuth.kicad_sch`** (XIAO **D** labels in docs/wiring.md).
 *   - Azimuth_Design: ESP32-C3-WROOM-02 on `kicad/Azimuth_Design` — RGB, buzzer, FUNC where populated.
 *
 * See docs/hardware-profiles.md and docs/wiring.md.
 */
#pragma once

#include <cstdint>

namespace azimuth_hw {

// BNO08x — SPI (FSPI on ESP32-C3: SCK/MISO/MOSI)
constexpr uint8_t kPinSpiSck = 8;
constexpr uint8_t kPinSpiMiso = 9;
constexpr uint8_t kPinSpiMosi = 10;

constexpr uint8_t kPinImuCs = 5;    // H_CSN
constexpr uint8_t kPinImuInt = 6;   // H_INTN (active low)
constexpr uint8_t kPinImuRst = 20;  // NRST (same GPIO as UART RX on XIAO)

// PS0/WAKE — GPIO2 (XIAO D0); use 10 kΩ to 3V3 on breadboard to match Azimuth_Design **R14**
constexpr uint8_t kPinImuPs0Wake = 2;

// RGB (Azimuth_Design): common anode, inverted PWM. Resistor nets R6/R7/R8 — logical R/G/B must match
// the *physical* cathode for each die (TZ-P4-1615 pad order vs schematic labels). Verified on V0.1:
// IO1+R7 (100 Ω) = red, IO0+R8 (68 Ω) = green, IO3+R6 (220 Ω) = blue (not IO0=red as older docs said).
constexpr uint8_t kPinRgbR = 1;
constexpr uint8_t kPinRgbG = 0;
constexpr uint8_t kPinRgbB = 3;
// DIY single-LED path uses GPIO3 as a simple status line (no Azimuth RGB layout).
constexpr uint8_t kPinStatusLed = 3;
constexpr uint8_t kPinButtonFunc = 7;
constexpr uint8_t kPinBuzzer = 21;
constexpr uint8_t kPinBattSenseAdc = 4;  // ADC1 CH4 (XIAO D2); divider tap per Azimuth_Design

}  // namespace azimuth_hw
