#include "power_policy.h"

namespace azimuth_power {

namespace {

// Single policy: tracking path untouched; these only affect portal + light network housekeeping.
// Portal is for settings, updates, and status — not a high‑refresh UI — so relaxed cadence saves CPU + radio.
constexpr uint16_t kNetworkServiceIntervalMs = 25;
// OpenTrack DNS refresh is 45 s; UDP bind is one-shot; modem-sleep idle is 30 s — no need for 200 Hz polls.
constexpr uint32_t kNetworkBackgroundPeriodMs = 500;
constexpr uint32_t kThermalSamplePeriodMs = 15000;
constexpr uint32_t kWifiSleepIdleDelayMs = 30000;

}  // namespace

uint16_t networkServiceIntervalMs() { return kNetworkServiceIntervalMs; }

uint32_t networkBackgroundPeriodMs() { return kNetworkBackgroundPeriodMs; }

uint32_t thermalSamplePeriodMs() { return kThermalSamplePeriodMs; }

uint32_t wifiSleepIdleDelayMs() { return kWifiSleepIdleDelayMs; }

}  // namespace azimuth_power
