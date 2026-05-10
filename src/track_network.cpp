#include "track_network.h"

#ifndef IMU_DEBUG_MODE
#define IMU_DEBUG_MODE 1
#endif

#if IMU_DEBUG_MODE

void trackNetworkLoadTrackingPrefs() {}
uint16_t trackNetworkImuRotationPeriodMs() { return 10; }
bool trackNetworkHatireUsbEnabled() { return true; }

void trackNetworkInit() {}
void trackNetworkLoop() {}
void trackNetworkPublishPoseSample(float, float, float) {}
void trackNetworkSendOpentrackUdp(float, float, float) {}

void trackNetworkApplyThermalEmergency() {}

bool trackNetworkThermalHoldActive() {
  return false;
}

bool trackNetworkSetupApActive() {
  return trackNetworkOfflineApActive();
}

bool trackNetworkOfflineApActive() {
  return false;
}

void trackNetworkSetStasis(bool) {}

bool trackNetworkStasisActive() {
  return false;
}

const char* trackNetworkBeginFirmwareUpdate() {
  return "chip_busy";
}

#else

#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFiUdp.h>

#include "track_network_internal.h"
#include "track_update.h"

namespace azimuth_net {

NetworkRuntime::NetworkRuntime()
    : webSta(kWebPortSta),
      webAp(kWebPortAp) {}

NetworkRuntime gRuntime;

}  // namespace azimuth_net

void trackNetworkLoadTrackingPrefs() {
  azimuth_net::loadTrackingPrefs();
}

uint16_t trackNetworkImuRotationPeriodMs() {
  return azimuth_net::imuPeriodMsValue();
}

bool trackNetworkHatireUsbEnabled() {
  return azimuth_net::hatireUsbValue();
}

const OtAxisMapConfig& trackNetworkOtAxisMap() {
  return azimuth_net::otAxisMapValue();
}

void trackNetworkInit() {
  azimuth_net::networkInit();
}

void trackNetworkLoop() {
  azimuth_net::networkLoop();
}

void trackNetworkPublishPoseSample(float yawDeg, float pitchDeg, float rollDeg) {
  if (azimuth_net::gRuntime.stasisActive) {
    azimuth_net::gRuntime.poseValid = false;
    return;
  }
  azimuth_net::gRuntime.poseYawDeg = yawDeg;
  azimuth_net::gRuntime.posePitchDeg = pitchDeg;
  azimuth_net::gRuntime.poseRollDeg = rollDeg;
  azimuth_net::gRuntime.poseLastMs = millis();
  azimuth_net::gRuntime.poseValid = true;
}

void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg) {
  azimuth_net::sendOpentrackUdp(yawDeg, pitchDeg, rollDeg);
}

void trackNetworkApplyThermalEmergency() {
  azimuth_net::applyThermalEmergency();
}

bool trackNetworkThermalHoldActive() {
  return azimuth_net::gRuntime.thermalHoldActive;
}

bool trackNetworkSetupApActive() {
  return trackNetworkOfflineApActive();
}

bool trackNetworkOfflineApActive() {
  return azimuth_net::gRuntime.offlineApMode;
}

void trackNetworkSetStasis(bool active) {
  azimuth_net::setStasis(active);
}

bool trackNetworkStasisActive() {
  return azimuth_net::gRuntime.stasisActive;
}

const char* trackNetworkBeginFirmwareUpdate() {
  return azimuth_update::beginResultString(azimuth_update::beginUpdate());
}

#endif  // !IMU_DEBUG_MODE
