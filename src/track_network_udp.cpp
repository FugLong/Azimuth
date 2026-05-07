#include "track_network_internal.h"

#if !IMU_DEBUG_MODE

#include <WiFi.h>

namespace azimuth_net {

namespace {
void resolveOtHostnameNow() {
  if (gRuntime.otHostTrimmed.length() == 0 || gRuntime.otHostIsLiteralIp) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    gRuntime.otIp = INADDR_NONE;
    return;
  }
  IPAddress resolved;
  if (WiFi.hostByName(gRuntime.otHostTrimmed.c_str(), resolved)) {
    gRuntime.otIp = resolved;
  } else {
    gRuntime.otIp = INADDR_NONE;
  }
  gRuntime.lastOtDnsMs = millis();
}
}  // namespace

void maybeRefreshOtHostname() {
  if (gRuntime.otHostIsLiteralIp || gRuntime.otHostTrimmed.length() == 0) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  const uint32_t now = millis();
  if (now - gRuntime.lastOtDnsMs < 45000) {
    return;
  }
  resolveOtHostnameNow();
}

void applyOtTarget() {
  gRuntime.otPort = mergedOtPort();
  gRuntime.otHostTrimmed = mergedOtHost();
  gRuntime.otHostTrimmed.trim();

  if (gRuntime.otHostTrimmed.length() == 0) {
    gRuntime.otIp = INADDR_NONE;
    gRuntime.otHostIsLiteralIp = true;
    return;
  }

  IPAddress asIp;
  if (asIp.fromString(gRuntime.otHostTrimmed.c_str())) {
    gRuntime.otIp = asIp;
    gRuntime.otHostIsLiteralIp = true;
    gRuntime.lastOtDnsMs = millis();
    return;
  }

  gRuntime.otHostIsLiteralIp = false;
  gRuntime.otIp = INADDR_NONE;
  resolveOtHostnameNow();
}

void tryOpenUdpSocket() {
  if (gRuntime.udpSocketOk || WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (gRuntime.udp.begin(0)) {
    gRuntime.udpSocketOk = true;
  }
}

bool otTargetOk() { return gRuntime.otIp != INADDR_NONE && gRuntime.otPort != 0; }

void sendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg) {
  if (!gRuntime.udpSocketOk || !gRuntime.udpSendEnabled || gRuntime.stasisActive ||
      WiFi.status() != WL_CONNECTED || !otTargetOk()) {
    return;
  }

  float rot[3];
  opentrackMapEulerToRot(yawDeg, pitchDeg, rollDeg, gRuntime.otAxisMapRuntime, rot);
  const double pose[6] = {
      0.0,
      0.0,
      0.0,
      static_cast<double>(rot[0]),
      static_cast<double>(rot[1]),
      static_cast<double>(rot[2]),
  };

  if (!gRuntime.udp.beginPacket(gRuntime.otIp, gRuntime.otPort)) {
    return;
  }
  gRuntime.udp.write(reinterpret_cast<const uint8_t*>(pose), sizeof(pose));
  gRuntime.udp.endPacket();
}

}  // namespace azimuth_net

#endif  // !IMU_DEBUG_MODE
