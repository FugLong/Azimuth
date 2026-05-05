#include "thermal_monitor.h"

#include <Arduino.h>
#include <cmath>
#include <esp32-hal.h>

#include "io_buzzer.h"
#include "power_policy.h"
#include "track_network.h"

namespace azimuth_thermal {
namespace {

constexpr float kWarnC = 80.0f;
constexpr float kCriticalC = 90.0f;
constexpr float kWarnClearC = 77.0f;
constexpr float kRecoverC = 62.0f;
constexpr uint32_t kHoldSampleMs = 60000;

float gLastTempC = NAN;
uint32_t gLastSampleMs = 0;
bool gWarnLatched = false;
bool gCriticalLatched = false;

uint32_t sampleIntervalMs() {
  if (trackNetworkThermalHoldActive()) {
    return kHoldSampleMs;
  }
  return azimuth_power::thermalSamplePeriodMs(trackNetworkPowerProfile());
}

}  // namespace

void init() {
  gLastTempC = NAN;
  gLastSampleMs = 0;
  gWarnLatched = false;
  gCriticalLatched = false;
}

void notifyThermalHoldEntered() {
  gLastSampleMs = 0;
}

float lastChipTempC() {
  return gLastTempC;
}

const char* stateJsonString() {
  if (trackNetworkThermalHoldActive()) {
    return "thermal_hold";
  }
  if (std::isnan(gLastTempC)) {
    return "ok";
  }
  if (gLastTempC >= kCriticalC) {
    return "emergency";
  }
  if (gLastTempC >= kWarnC) {
    return "warn";
  }
  return "ok";
}

void tick(uint32_t now_ms) {
  if (trackNetworkThermalHoldActive()) {
    if (gLastSampleMs != 0 && (now_ms - gLastSampleMs) < sampleIntervalMs()) {
      return;
    }
    gLastSampleMs = now_ms;
    const float t = temperatureRead();
    if (!std::isnan(t)) {
      gLastTempC = t;
    }
    if (!std::isnan(gLastTempC) && gLastTempC < kRecoverC) {
      delay(80);
      ESP.restart();
    }
    return;
  }

  if (gLastSampleMs != 0 && (now_ms - gLastSampleMs) < sampleIntervalMs()) {
    return;
  }
  gLastSampleMs = now_ms;

  const float t = temperatureRead();
  if (std::isnan(t)) {
    return;
  }
  gLastTempC = t;

  if (t >= kCriticalC) {
    if (!gCriticalLatched) {
      gCriticalLatched = true;
      azimuth_io_buzzer::playThermalCriticalTune();
      trackNetworkApplyThermalEmergency();
    }
    return;
  }

  if (t >= kWarnC) {
    if (!gWarnLatched) {
      gWarnLatched = true;
      azimuth_io_buzzer::playThermalWarnTune();
    }
  } else if (t < kWarnClearC) {
    gWarnLatched = false;
  }
}

}  // namespace azimuth_thermal
