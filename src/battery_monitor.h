#pragma once

#include <stdint.h>

namespace azimuth_battery {

struct Status {
  bool supported = false;
  bool stub = true;
  int32_t rawMillivolts = -1;      // ADC pin voltage before divider scaling
  int32_t millivolts = -1;
  int32_t percent = -1;            // 0..110 when known (100+ while charging headroom)
  int32_t remainingMah = -1;       // derived from percent and configured pack size
  uint16_t capacityMah = 800;      // user-configured nominal pack size
  int32_t calibrationOffsetMv = 0; // user-configured offset after scaling
  const char* chargeState = "unknown";  // unknown|charging|full|discharging|idle|absent|unsupported|stub
};

void init();
void tick(uint32_t nowMs);
Status readStatus();
void setCalibrationOffsetMv(int32_t offsetMv);
void setCapacityMah(uint16_t mah);
bool calibrateAgainstKnownPackMv(int32_t knownPackMv, uint32_t sampleWindowMs,
                                 int32_t* outOffsetMv, int32_t* outAvgRawMv);

}  // namespace azimuth_battery
