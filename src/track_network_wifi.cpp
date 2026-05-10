#include "track_network_internal.h"

#if !IMU_DEBUG_MODE

#include <ESPmDNS.h>
#include <WiFi.h>

#include "io_buzzer.h"
#include "power_policy.h"
#include "thermal_monitor.h"
#include "track_update.h"

namespace azimuth_net {
namespace {
constexpr uint16_t kApPortalServiceIntervalMs = 80;
constexpr uint32_t kFwCheckMinStaUptimeMs = 300;

void maybePerformFirmwareUpdateCheck(uint32_t now) {
  static uint32_t staReadyMs = 0;
  const bool staOk =
      !gRuntime.offlineApMode && (WiFi.status() == WL_CONNECTED) && !gRuntime.fwUpdateCheckDone;
  if (!staOk) {
    staReadyMs = 0;
    return;
  }
  if (staReadyMs == 0) {
    staReadyMs = now;
  }
  if (now - staReadyMs < kFwCheckMinStaUptimeMs) {
    return;
  }
  // Honour exponential backoff after a failed attempt so we don't pound the
  // release server on a flaky link.
  if (gRuntime.fwUpdateNextCheckMs != 0 && now < gRuntime.fwUpdateNextCheckMs) {
    return;
  }
  // Don't gate on UDP idle — when OpenTrack is streaming at 100 Hz the socket
  // is *never* idle, which used to silently prevent the manifest check from
  // ever running on a working device. The TLS fetch is one-shot per boot and
  // any brief stutter is acceptable. Still avoid overlapping with audio so the
  // buzzer's PWM timing isn't disturbed.
  if (!azimuth_io_buzzer::isActive()) {
    performFirmwareUpdateCheckOnce();
  }
}
}  // namespace

void applyThermalEmergency() {
  if (gRuntime.thermalHoldActive) {
    return;
  }
  gRuntime.stasisActive = false;
  gRuntime.thermalHoldActive = true;
  gRuntime.udpSocketOk = false;
  gRuntime.fwUpdateCheckDone = true;
  MDNS.end();
  if (gRuntime.staWebActive) {
    gRuntime.webSta.stop();
    gRuntime.staWebActive = false;
  }
  if (gRuntime.apPortalActive) {
    gRuntime.dnsCaptive.stop();
    gRuntime.webAp.stop();
    gRuntime.apPortalActive = false;
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  gRuntime.offlineApMode = false;
  gRuntime.wifiSleepEnabled = false;
  azimuth_thermal::notifyThermalHoldEntered();
}

void startOfflineApPortal() {
  gRuntime.offlineApMode = true;
  gRuntime.staWebActive = false;
  gRuntime.dnsCaptive.stop();
  WiFi.disconnect(true);
  delay(100);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  // Offline Mode AP is provisioning/off-grid access: keep TX very low to reduce thermal load.
  WiFi.setTxPower(WIFI_POWER_2dBm);
  (void)WiFi.softAP(kOfflineApSsid, nullptr, 1, 0, 1);
  const IPAddress apIp = WiFi.softAPIP();

  gRuntime.dnsCaptive.setErrorReplyCode(DNSReplyCode::NoError);
  gRuntime.dnsCaptive.setTTL(120);
  (void)gRuntime.dnsCaptive.start(53, "*", apIp);

  registerRoutes(gRuntime.webAp, true);
  gRuntime.webAp.begin();
  gRuntime.apPortalActive = true;
  gRuntime.wifiSleepEnabled = false;
  gRuntime.lastPortalActivityMs = millis();
}

void networkInit() {
  if (!ensurePrefsOpen()) {
    WiFi.persistent(false);
    startOfflineApPortal();
    return;
  }

  refreshRuntimeFromPrefs();
  gRuntime.udpSendEnabled = mergedUdpOn();
  applyOtTarget();

  const String ssid = mergedSsid();
  if (ssid.length() == 0) {
    WiFi.persistent(false);
    startOfflineApPortal();
    return;
  }

  gRuntime.offlineApMode = false;
  WiFi.persistent(false);
  WiFi.setHostname(mergedHostname().c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), mergedPass().c_str());

  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < kWifiConnectTimeoutMs) {
    delay(200);
  }

  if (WiFi.status() != WL_CONNECTED) {
    startOfflineApPortal();
    return;
  }

  delay(100);
  applyOtTarget();
  applyStaWifiTxPower();
  tryOpenUdpSocket();
  gRuntime.lastPortalActivityMs = millis();
  applyAdaptiveWifiSleep();

  if (mergedMdnsOn()) {
    if (MDNS.begin(mergedHostname().c_str())) {
      (void)MDNS.addService("http", "tcp", kWebPortSta);
      (void)MDNS.enableWorkstation(ESP_IF_WIFI_STA);
    }
  }

  registerRoutes(gRuntime.webSta, false);
  gRuntime.webSta.begin();
  gRuntime.staWebActive = true;
}

void networkLoop() {
  if (gRuntime.thermalHoldActive) {
    return;
  }
  const uint32_t now = millis();
  const uint16_t serviceInterval =
      gRuntime.apPortalActive ? kApPortalServiceIntervalMs : azimuth_power::networkServiceIntervalMs();
  if (now - gRuntime.lastNetworkServiceMs >= serviceInterval) {
    gRuntime.lastNetworkServiceMs = now;
    if (gRuntime.apPortalActive) {
      gRuntime.dnsCaptive.processNextRequest();
      gRuntime.webAp.handleClient();
    }
    if (gRuntime.staWebActive) {
      gRuntime.webSta.handleClient();
    }
  }

  // Drive the OTA chunk pump every loop tick (cheap when idle; ~one 4 KB write
  // per call when active). Must run before the background block so a brief
  // adaptive-sleep window doesn't gate progress.
  azimuth_update::tick();

  if (now - gRuntime.lastBackgroundTickMs >= azimuth_power::networkBackgroundPeriodMs()) {
    gRuntime.lastBackgroundTickMs = now;
    if (WiFi.status() == WL_CONNECTED) {
      tryOpenUdpSocket();
      maybeRefreshOtHostname();
    }
    applyAdaptiveWifiSleep();
    maybePerformFirmwareUpdateCheck(now);
  }
}

}  // namespace azimuth_net

#endif  // !IMU_DEBUG_MODE
