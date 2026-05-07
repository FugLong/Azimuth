#include "track_network_internal.h"

#if !IMU_DEBUG_MODE

#include <ESPmDNS.h>
#include <WiFi.h>

#include "io_buzzer.h"
#include "power_policy.h"
#include "thermal_monitor.h"

namespace azimuth_net {

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
  gRuntime.setupApMode = false;
  gRuntime.wifiSleepEnabled = false;
  azimuth_thermal::notifyThermalHoldEntered();
}

void startProvisioningPortal() {
  gRuntime.setupApMode = true;
  gRuntime.staWebActive = false;
  gRuntime.dnsCaptive.stop();
  WiFi.disconnect(true);
  delay(100);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  (void)WiFi.softAP(kSetupApSsid, nullptr, 1, 0, 4);
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
    return;
  }

  refreshRuntimeFromPrefs();
  gRuntime.udpSendEnabled = mergedUdpOn();
  applyOtTarget();

  const String ssid = mergedSsid();
  if (ssid.length() == 0) {
    WiFi.persistent(false);
    startProvisioningPortal();
    return;
  }

  gRuntime.setupApMode = false;
  WiFi.persistent(false);
  WiFi.setHostname(mergedHostname().c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), mergedPass().c_str());

  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < kWifiConnectTimeoutMs) {
    delay(200);
  }

  if (WiFi.status() != WL_CONNECTED) {
    startProvisioningPortal();
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
  const uint16_t serviceInterval = azimuth_power::networkServiceIntervalMs();
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

  if (now - gRuntime.lastBackgroundTickMs >= azimuth_power::networkBackgroundPeriodMs()) {
    gRuntime.lastBackgroundTickMs = now;
    if (WiFi.status() == WL_CONNECTED) {
      tryOpenUdpSocket();
      maybeRefreshOtHostname();
    }
    applyAdaptiveWifiSleep();
  }

  static uint32_t sFwStaReadyMs = 0;
  const bool staOk =
      !gRuntime.setupApMode && (WiFi.status() == WL_CONNECTED) && !gRuntime.fwUpdateCheckDone;
  if (!staOk) {
    sFwStaReadyMs = 0;
  } else if (sFwStaReadyMs == 0) {
    sFwStaReadyMs = now;
  }
  if (staOk && sFwStaReadyMs != 0 && (now - sFwStaReadyMs >= 300)) {
    if (!azimuth_io_buzzer::isActive()) {
      performFirmwareUpdateCheckOnce();
    }
  }
}

}  // namespace azimuth_net

#endif  // !IMU_DEBUG_MODE
