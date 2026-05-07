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
void trackNetworkSendOpentrackUdp(float, float, float) {}

void trackNetworkApplyThermalEmergency() {}

bool trackNetworkThermalHoldActive() {
  return false;
}

bool trackNetworkSetupApActive() {
  return false;
}

void trackNetworkSetStasis(bool) {}

bool trackNetworkStasisActive() {
  return false;
}

#else

#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFiUdp.h>

#include "track_network_internal.h"

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
  return azimuth_net::gRuntime.setupApMode;
}

void trackNetworkSetStasis(bool active) {
  azimuth_net::setStasis(active);
}

bool trackNetworkStasisActive() {
  return azimuth_net::gRuntime.stasisActive;
}

#endif  // !IMU_DEBUG_MODE
