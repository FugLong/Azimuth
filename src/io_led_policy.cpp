#include "io_led_policy.h"

#include "io_led.h"

#if IMU_DEBUG_MODE

namespace azimuth_io_led_policy {

void tick(uint32_t nowMs, bool imuPoseStreaming) {
  (void)nowMs;
  azimuth_io_led::setPolicyOverride(azimuth_io_led::PolicyOverride::None);
  azimuth_io_led::setStatus(imuPoseStreaming);
  azimuth_io_led::tick();
}

}  // namespace azimuth_io_led_policy

#else

#include "battery_monitor.h"
#include "thermal_monitor.h"
#include "track_network.h"

#include <cstring>

namespace azimuth_io_led_policy {

void tick(uint32_t nowMs, bool imuPoseStreaming) {
  (void)nowMs;
  using PO = azimuth_io_led::PolicyOverride;

  if (trackNetworkThermalHoldActive()) {
    azimuth_io_led::setPolicyOverride(PO::ThermalHold);
    azimuth_io_led::tick();
    return;
  }

  const char* ts = azimuth_thermal::stateJsonString();
  if (strcmp(ts, "warn") == 0) {
    azimuth_io_led::setPolicyOverride(PO::ThermalWarn);
    azimuth_io_led::tick();
    return;
  }

  const auto batt = azimuth_battery::readStatus();
  if (batt.supported && !batt.stub && batt.percent >= 0 && batt.percent <= 10 &&
      strcmp(batt.chargeState, "absent") != 0) {
    azimuth_io_led::setPolicyOverride(PO::BatteryCritical);
    azimuth_io_led::tick();
    return;
  }

  if (trackNetworkSetupApActive()) {
    azimuth_io_led::setPolicyOverride(PO::SetupAp);
    azimuth_io_led::tick();
    return;
  }

  if (trackNetworkStasisActive()) {
    azimuth_io_led::setPolicyOverride(PO::Stasis);
    azimuth_io_led::tick();
    return;
  }

  azimuth_io_led::setPolicyOverride(PO::None);
  azimuth_io_led::setStatus(imuPoseStreaming);
  azimuth_io_led::tick();
}

}  // namespace azimuth_io_led_policy

#endif
