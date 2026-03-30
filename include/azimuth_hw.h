/**
 * Pin map for BNO08x (SPI) and board I/O.
 *
 * Two hardware paths share the same GPIO numbers so one firmware build profile
 * matches both: Seeed XIAO ESP32-C3 (DIY + ESP32_BNO086-style PCB) and a
 * discrete ESP32-C3 module (e.g. WROOM-02 on the Azimuth integrated PCB).
 * Route these GPIOs on the PCB; names like D3/D8 are XIAO silk only.
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

// PS0/WAKE — strapped on PCB; not driven as a kPin* in current firmware
constexpr uint8_t kPinImuPs0Wake = 4;

// Planned board I/O (wiring.md — not all used in firmware yet)
constexpr uint8_t kPinStatusLed = 3;
constexpr uint8_t kPinButtonFunc = 7;
constexpr uint8_t kPinBuzzer = 21;
constexpr uint8_t kPinBattSenseAdc = 2;  // ADC1

}  // namespace azimuth_hw
