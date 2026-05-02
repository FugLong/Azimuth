#pragma once

#include <stdint.h>

namespace azimuth_power {

enum class PowerProfile : uint8_t {
  PerformanceTracking = 0,
  Balanced = 1,
  BatterySaver = 2,
};

PowerProfile fromStoredValue(uint8_t value);
uint8_t toStoredValue(PowerProfile profile);

uint16_t networkServiceIntervalMs(PowerProfile profile);
uint32_t wifiSleepIdleDelayMs(PowerProfile profile);
bool runFirmwareUpdateCheck(PowerProfile profile);

}  // namespace azimuth_power
