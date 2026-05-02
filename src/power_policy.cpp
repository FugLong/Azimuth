#include "power_policy.h"

namespace azimuth_power {

PowerProfile fromStoredValue(uint8_t value) {
  if (value > static_cast<uint8_t>(PowerProfile::BatterySaver)) {
    return PowerProfile::Balanced;
  }
  return static_cast<PowerProfile>(value);
}

uint8_t toStoredValue(PowerProfile profile) {
  return static_cast<uint8_t>(profile);
}

uint16_t networkServiceIntervalMs(PowerProfile profile) {
  switch (profile) {
    case PowerProfile::PerformanceTracking:
      return 8;
    case PowerProfile::BatterySaver:
      return 30;
    case PowerProfile::Balanced:
    default:
      return 15;
  }
}

uint32_t wifiSleepIdleDelayMs(PowerProfile profile) {
  switch (profile) {
    case PowerProfile::PerformanceTracking:
      return 120000;
    case PowerProfile::BatterySaver:
      return 12000;
    case PowerProfile::Balanced:
    default:
      return 30000;
  }
}

bool runFirmwareUpdateCheck(PowerProfile profile) {
  return profile != PowerProfile::BatterySaver;
}

}  // namespace azimuth_power
