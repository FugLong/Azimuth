#include "battery_monitor.h"

#include <Arduino.h>
#include <climits>
#include <cmath>
#include <cstring>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_battery {
namespace {
Status gStatus;
uint32_t gLastSampleMs = 0;
float gFilteredPackMv = NAN;
float gPercentEstimate = NAN;
int32_t gStablePercent = -1;
float gNoiseEmaMv = NAN;
int8_t gDisplayDirection = 0;  // -1 falling, +1 rising, 0 unknown.

constexpr uint32_t kSamplePeriodMs = 1000;
constexpr uint8_t kSampleCount = 12;
constexpr float kDividerRatio = 2.0f;
constexpr uint16_t kDefaultCapacityMah = 800;
constexpr int32_t kMinCapacityMah = 100;
constexpr int32_t kMaxCapacityMah = 5000;
constexpr int32_t kMinCalOffsetMv = -1200;
constexpr int32_t kMaxCalOffsetMv = 1200;
constexpr uint16_t kCalSampleStepMs = 25;
constexpr int32_t kBatteryAbsentEnterMv = 1900;
constexpr int32_t kBatteryAbsentExitMv = 2300;
constexpr int32_t kChargingHeadroomEnterMv = 4205;
constexpr int32_t kChargingHeadroomExitMv = 4185;
constexpr float kVoltageAlpha = 0.18f;
constexpr float kPercentAlpha = 0.20f;
constexpr int32_t kPercentMax = 110;
constexpr int32_t kFilterRelockStepMv = 130;
constexpr float kNoiseAlpha = 0.25f;
constexpr float kNoiseStableMv = 10.0f;
constexpr float kNoiseUnstableMv = 45.0f;
constexpr float kDeltaStableMv = 4.0f;
constexpr float kDeltaUnstableMv = 35.0f;

int32_t clampOffsetMv(int32_t v) {
  if (v < kMinCalOffsetMv) {
    return kMinCalOffsetMv;
  }
  if (v > kMaxCalOffsetMv) {
    return kMaxCalOffsetMv;
  }
  return v;
}

uint16_t clampCapacityMah(int32_t v) {
  if (v < kMinCapacityMah) {
    return static_cast<uint16_t>(kMinCapacityMah);
  }
  if (v > kMaxCapacityMah) {
    return static_cast<uint16_t>(kMaxCapacityMah);
  }
  return static_cast<uint16_t>(v);
}

float voltageToPercentFloat(int32_t mv) {
  // 1S Li-ion resting-voltage estimate with optional headroom above 100%.
  if (mv <= 3200) {
    return 0.0f;
  }
  if (mv >= 4200) {
    const float over = static_cast<float>(mv - 4200) / 10.0f;
    const float pct = 100.0f + over;
    return (pct > static_cast<float>(kPercentMax)) ? static_cast<float>(kPercentMax) : pct;
  }
  struct Pt {
    int16_t mv;
    int8_t pct;
  };
  static constexpr Pt kCurve[] = {
      {3200, 0}, {3300, 5}, {3400, 10}, {3500, 20}, {3600, 35},
      {3700, 50}, {3800, 65}, {3900, 80}, {4000, 92}, {4100, 97}, {4200, 100},
  };
  for (size_t i = 1; i < sizeof(kCurve) / sizeof(kCurve[0]); ++i) {
    if (mv <= kCurve[i].mv) {
      const int32_t x0 = kCurve[i - 1].mv;
      const int32_t y0 = kCurve[i - 1].pct;
      const int32_t x1 = kCurve[i].mv;
      const int32_t y1 = kCurve[i].pct;
      const int32_t num = (mv - x0) * (y1 - y0);
      return static_cast<float>(y0 + num / (x1 - x0));
    }
  }
  return 100.0f;
}

int32_t clampPercent(int32_t v) {
  if (v < 0) {
    return 0;
  }
  if (v > kPercentMax) {
    return kPercentMax;
  }
  return v;
}

void sortAscending(int32_t* values, int n) {
  for (int i = 1; i < n; ++i) {
    const int32_t key = values[i];
    int j = i - 1;
    while (j >= 0 && values[j] > key) {
      values[j + 1] = values[j];
      --j;
    }
    values[j + 1] = key;
  }
}

float normalizedInverseRange(float value, float low, float high) {
  if (value <= low) {
    return 1.0f;
  }
  if (value >= high) {
    return 0.0f;
  }
  return (high - value) / (high - low);
}

int32_t stabilizedPercentFromEstimate(float targetPercent, float confidence) {
  if (std::isnan(gPercentEstimate)) {
    gPercentEstimate = targetPercent;
  } else {
    gPercentEstimate = (gPercentEstimate * (1.0f - kPercentAlpha)) + (targetPercent * kPercentAlpha);
  }
  const int32_t targetRounded = clampPercent(static_cast<int32_t>(lroundf(gPercentEstimate)));
  if (gStablePercent < 0) {
    gStablePercent = targetRounded;
    return gStablePercent;
  }

  if (confidence < 0.0f) {
    confidence = 0.0f;
  } else if (confidence > 1.0f) {
    confidence = 1.0f;
  }

  const float deadband = (confidence >= 0.75f) ? 1.0f : 2.0f;
  const float delta = static_cast<float>(targetRounded - gStablePercent);
  const float absDelta = fabsf(delta);
  const int8_t dir = (delta > 0.0f) ? 1 : -1;
  const bool reversing = (gDisplayDirection != 0 && dir != gDisplayDirection);
  const float reversePenalty = reversing ? 2.5f : 0.0f;
  if (absDelta < (deadband + reversePenalty)) {
    return gStablePercent;
  }

  // Fast catch-up only for very high-confidence large shifts.
  if (absDelta >= 20.0f && confidence >= 0.90f) {
    gStablePercent = targetRounded;
    gDisplayDirection = dir;
    return gStablePercent;
  }

  // Signal-driven movement (no explicit time gate): stable signal can move faster.
  int32_t maxStep = 1;
  if (confidence >= 0.88f && absDelta >= 10.0f) {
    maxStep = 4;
  } else if (confidence >= 0.80f && absDelta >= 8.0f) {
    maxStep = 3;
  } else if (confidence >= 0.60f && absDelta >= 6.0f) {
    maxStep = 2;
  } else if (confidence < 0.35f && absDelta < 8.0f) {
    return gStablePercent;
  }

  if (delta > 0.0f) {
    gStablePercent += maxStep;
    if (gStablePercent > targetRounded) {
      gStablePercent = targetRounded;
    }
  } else {
    gStablePercent -= maxStep;
    if (gStablePercent < targetRounded) {
      gStablePercent = targetRounded;
    }
  }
  gStablePercent = clampPercent(gStablePercent);
  if (gStablePercent != targetRounded || absDelta >= deadband) {
    gDisplayDirection = dir;
  }
  return gStablePercent;
}

void sampleNow(uint32_t nowMs) {
  if (!gStatus.supported) {
    return;
  }
  if (gLastSampleMs != 0 && (nowMs - gLastSampleMs) < kSamplePeriodMs) {
    return;
  }
  gLastSampleMs = nowMs;

  int32_t minRawMv = INT32_MAX;
  int32_t maxRawMv = INT32_MIN;
  int32_t rawSamples[kSampleCount];
  int32_t sampleCount = 0;
  int valid = 0;
  for (uint8_t i = 0; i < kSampleCount; ++i) {
    const int32_t raw = analogReadMilliVolts(azimuth_hw::kPinBattSenseAdc);
    if (raw > 0) {
      if (raw < minRawMv) {
        minRawMv = raw;
      }
      if (raw > maxRawMv) {
        maxRawMv = raw;
      }
      if (sampleCount < kSampleCount) {
        rawSamples[sampleCount++] = raw;
      }
      valid++;
    }
  }
  if (valid <= 0) {
    gStatus.stub = true;
    gStatus.rawMillivolts = -1;
    gStatus.millivolts = -1;
    gStatus.percent = -1;
    gStatus.remainingMah = -1;
    gStatus.chargeState = "stub";
    return;
  }

  int32_t sumRawMv = 0;
  if (sampleCount >= 6) {
    sortAscending(rawSamples, sampleCount);
    // Average middle 50% to reject edge noise spikes.
    const int start = sampleCount / 4;
    const int end = sampleCount - start;
    valid = 0;
    for (int i = start; i < end; ++i) {
      sumRawMv += rawSamples[i];
      valid++;
    }
  } else {
    valid = sampleCount;
    for (int i = 0; i < sampleCount; ++i) {
      sumRawMv += rawSamples[i];
    }
  }
  const int32_t rawMv = sumRawMv / valid;
  const float scaledMv = static_cast<float>(rawMv) * kDividerRatio +
                         static_cast<float>(gStatus.calibrationOffsetMv);
  const int32_t scaledInstantMv = static_cast<int32_t>(lroundf(scaledMv));

  // Fast absent/present gating with hysteresis on instantaneous scaled voltage.
  const bool wasAbsent = (strcmp(gStatus.chargeState, "absent") == 0);
  const bool nowAbsent = wasAbsent ? (scaledInstantMv < kBatteryAbsentExitMv)
                                   : (scaledInstantMv < kBatteryAbsentEnterMv);
  if (nowAbsent) {
    gStatus.stub = false;
    gStatus.rawMillivolts = rawMv;
    gStatus.millivolts = scaledInstantMv;
    gStatus.percent = -1;
    gStatus.remainingMah = -1;
    gStatus.chargeState = "absent";
    gFilteredPackMv = scaledMv;
    gPercentEstimate = NAN;
    gStablePercent = -1;
    gNoiseEmaMv = NAN;
    gDisplayDirection = 0;
    return;
  }

  // Re-lock quickly when battery appears again.
  if (wasAbsent) {
    gFilteredPackMv = scaledMv;
    gPercentEstimate = NAN;
    gStablePercent = -1;
    gNoiseEmaMv = NAN;
    gDisplayDirection = 0;
  }

  const float prevFilteredMv = gFilteredPackMv;
  if (std::isnan(gFilteredPackMv)) {
    gFilteredPackMv = scaledMv;
  } else if (fabsf(scaledMv - gFilteredPackMv) >= static_cast<float>(kFilterRelockStepMv)) {
    // Real step change (battery swap / bench supply move): relock immediately.
    gFilteredPackMv = scaledMv;
    gPercentEstimate = voltageToPercentFloat(static_cast<int32_t>(lroundf(gFilteredPackMv)));
    gStablePercent = clampPercent(static_cast<int32_t>(lroundf(gPercentEstimate)));
    gNoiseEmaMv = NAN;
    gDisplayDirection = 0;
  } else {
    gFilteredPackMv = (gFilteredPackMv * (1.0f - kVoltageAlpha)) + (scaledMv * kVoltageAlpha);
  }
  const float filteredDeltaMv =
      std::isnan(prevFilteredMv) ? 0.0f : fabsf(gFilteredPackMv - prevFilteredMv);
  const float rawSpreadMv = static_cast<float>(maxRawMv - minRawMv) * kDividerRatio;
  if (std::isnan(gNoiseEmaMv)) {
    gNoiseEmaMv = rawSpreadMv;
  } else {
    gNoiseEmaMv = (gNoiseEmaMv * (1.0f - kNoiseAlpha)) + (rawSpreadMv * kNoiseAlpha);
  }
  const float noiseScore = normalizedInverseRange(gNoiseEmaMv, kNoiseStableMv, kNoiseUnstableMv);
  const float deltaScore = normalizedInverseRange(filteredDeltaMv, kDeltaStableMv, kDeltaUnstableMv);
  const float confidence = (noiseScore * 0.70f) + (deltaScore * 0.30f);

  gStatus.stub = false;
  gStatus.rawMillivolts = rawMv;
  gStatus.millivolts = static_cast<int32_t>(lroundf(gFilteredPackMv));
  gStatus.percent =
      stabilizedPercentFromEstimate(voltageToPercentFloat(gStatus.millivolts), confidence);
  const int32_t percentForMah = (gStatus.percent > 100) ? 100 : gStatus.percent;
  gStatus.remainingMah = static_cast<int32_t>(
      (static_cast<int64_t>(gStatus.capacityMah) * percentForMah) / 100);
  const bool wasCharging = (strcmp(gStatus.chargeState, "charging") == 0);
  const bool isCharging = wasCharging ? (gStatus.millivolts >= kChargingHeadroomExitMv)
                                      : (gStatus.millivolts >= kChargingHeadroomEnterMv);
  gStatus.chargeState = isCharging ? "charging" : "idle";
}
}  // namespace

void init() {
  gStatus.supported = azimuth_board::capabilities().hasBatterySense;
  gStatus.stub = !gStatus.supported;
  gStatus.rawMillivolts = -1;
  gStatus.millivolts = -1;
  gStatus.percent = -1;
  gStatus.remainingMah = -1;
  gStatus.capacityMah = kDefaultCapacityMah;
  gStatus.calibrationOffsetMv = 0;
  gStatus.chargeState = gStatus.supported ? "unknown" : "unsupported";
  gLastSampleMs = 0;
  gFilteredPackMv = NAN;
  gPercentEstimate = NAN;
  gStablePercent = -1;
  gNoiseEmaMv = NAN;
  gDisplayDirection = 0;
  if (gStatus.supported) {
    analogReadResolution(12);
    analogSetPinAttenuation(azimuth_hw::kPinBattSenseAdc, ADC_11db);
  }
}

void tick(uint32_t nowMs) {
  sampleNow(nowMs);
}

Status readStatus() {
  return gStatus;
}

void setCalibrationOffsetMv(int32_t offsetMv) {
  gStatus.calibrationOffsetMv = clampOffsetMv(offsetMv);
}

void setCapacityMah(uint16_t mah) {
  gStatus.capacityMah = clampCapacityMah(static_cast<int32_t>(mah));
}

bool calibrateAgainstKnownPackMv(int32_t knownPackMv, uint32_t sampleWindowMs,
                                 int32_t* outOffsetMv, int32_t* outAvgRawMv) {
  if (!gStatus.supported || knownPackMv <= 0) {
    return false;
  }
  if (sampleWindowMs < 250) {
    sampleWindowMs = 250;
  }

  const uint32_t t0 = millis();
  int64_t sumRaw = 0;
  int32_t valid = 0;
  while (millis() - t0 < sampleWindowMs) {
    const int32_t raw = analogReadMilliVolts(azimuth_hw::kPinBattSenseAdc);
    if (raw > 0) {
      sumRaw += raw;
      valid++;
    }
    delay(kCalSampleStepMs);
    yield();
  }
  if (valid <= 0) {
    return false;
  }

  const int32_t avgRaw = static_cast<int32_t>(sumRaw / valid);
  if (static_cast<int32_t>(avgRaw * kDividerRatio) < kBatteryAbsentEnterMv) {
    return false;
  }
  const int32_t newOffset = clampOffsetMv(knownPackMv - static_cast<int32_t>(avgRaw * kDividerRatio));
  gStatus.calibrationOffsetMv = newOffset;

  // Reset filter so UI converges quickly to the new calibrated level.
  gFilteredPackMv = static_cast<float>(avgRaw) * kDividerRatio + static_cast<float>(newOffset);
  gStatus.rawMillivolts = avgRaw;
  gStatus.millivolts = static_cast<int32_t>(lroundf(gFilteredPackMv));
  gPercentEstimate = voltageToPercentFloat(gStatus.millivolts);
  gStablePercent = clampPercent(static_cast<int32_t>(lroundf(gPercentEstimate)));
  gNoiseEmaMv = NAN;
  gDisplayDirection = 0;
  gStatus.percent = gStablePercent;
  const int32_t percentForMah = (gStatus.percent > 100) ? 100 : gStatus.percent;
  gStatus.remainingMah = static_cast<int32_t>(
      (static_cast<int64_t>(gStatus.capacityMah) * percentForMah) / 100);
  gStatus.stub = false;
  gStatus.chargeState = (gStatus.millivolts >= kChargingHeadroomEnterMv) ? "charging" : "idle";

  if (outOffsetMv) {
    *outOffsetMv = newOffset;
  }
  if (outAvgRawMv) {
    *outAvgRawMv = avgRaw;
  }
  return true;
}

}  // namespace azimuth_battery
