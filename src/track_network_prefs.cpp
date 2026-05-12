#include "track_network_internal.h"

#if !IMU_DEBUG_MODE

#include <WiFi.h>
#include <cstdio>
#include <cstring>

#include "battery_monitor.h"
#include "io_buzzer.h"
#include "io_led.h"
#include "power_policy.h"
#include "secrets.h"
#include "track_config_validation.h"

namespace azimuth_net {

bool ensurePrefsOpen() {
  if (gRuntime.prefsOpened) {
    return true;
  }
  gRuntime.prefsOpened = gRuntime.prefs.begin(kPrefsNs, false);
  return gRuntime.prefsOpened;
}

wifi_power_t wifiTxFromProfile(uint8_t profile) {
  switch (profile) {
    case 0:
      return WIFI_POWER_2dBm;
    case 2:
      return WIFI_POWER_19_5dBm;
    default:
      return WIFI_POWER_8_5dBm;
  }
}

uint8_t mergedWifiTxProfile() {
  const uint32_t v = gRuntime.prefs.getUInt("wifi_tx", 1);
  if (v > 2) {
    return 1;
  }
  return static_cast<uint8_t>(v);
}

uint16_t mergedImuPeriodMs() {
  const uint32_t v = gRuntime.prefs.getUInt("imu_period_ms", 10);
  if (v != 5 && v != 10 && v != 20 && v != 40) {
    return 10;
  }
  return static_cast<uint16_t>(v);
}

bool mergedImuDynamic() {
  if (!gRuntime.prefsOpened) {
    return false;
  }
  return gRuntime.prefs.getBool("imu_dyn", false);
}

bool mergedHatireUsb() { return gRuntime.prefs.getBool("hatire_usb", true); }

bool mergedMdnsOn() { return gRuntime.prefs.getBool("mdns_on", true); }

String mergedHostname() {
  String h = gRuntime.prefs.getString("hostname", "");
  h.trim();
  for (int i = 0; i < h.length(); ++i) {
    const char c = h[i];
    if (c >= 'A' && c <= 'Z') {
      h.setCharAt(i, static_cast<char>(c - 'A' + 'a'));
    }
  }
  if (h.length() == 0) {
    return String("azimuth");
  }
  return h;
}

void refreshRuntimeFromPrefs() {
  if (!gRuntime.prefsOpened) {
    otAxisMapSetDefault(&gRuntime.otAxisMapRuntime);
    return;
  }
  gRuntime.imuPeriodMsRuntime = mergedImuPeriodMs();
  gRuntime.imuDynamicRuntime = mergedImuDynamic();
  gRuntime.hatireUsbRuntime = mergedHatireUsb();
  gRuntime.otAxisMapRuntime = mergedOtAxisMap();
  azimuth_battery::setCapacityMah(mergedBatteryCapacityMah());
  azimuth_battery::setCalibrationOffsetMv(mergedBatteryCalOffsetMv());
}

void markImuReportPrefsDirty() { gRuntime.imuReportPrefsDirty = true; }

bool takeImuReportPrefsDirty() {
  if (!gRuntime.imuReportPrefsDirty) {
    return false;
  }
  gRuntime.imuReportPrefsDirty = false;
  return true;
}

void applyStaWifiTxPower() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setTxPower(wifiTxFromProfile(mergedWifiTxProfile()));
  }
}

void markPortalActivity() {
  gRuntime.lastPortalActivityMs = millis();
}

void applyAdaptiveWifiSleep() {
  if (gRuntime.offlineApMode || WiFi.status() != WL_CONNECTED) {
    if (gRuntime.wifiSleepEnabled) {
      WiFi.setSleep(false);
      gRuntime.wifiSleepEnabled = false;
    }
    return;
  }

  if (gRuntime.stasisActive) {
    if (!gRuntime.wifiSleepEnabled) {
      WiFi.setSleep(true);
      gRuntime.wifiSleepEnabled = true;
    }
    return;
  }

  // Portal-idle alone must not enable modem sleep while OpenTrack UDP is
  // streaming — WiFi light sleep adds jitter and burst drops on UDP. Hatire-only
  // (no UDP TX) still allows sleep once portal HTTP goes idle.
  const uint32_t nowMs = millis();
  constexpr uint32_t kUdpStreamWakeWindowMs = 2000;
  const bool udpStreaming =
      gRuntime.udpSendEnabled && (nowMs - gRuntime.lastUdpTxMs < kUdpStreamWakeWindowMs);
  if (udpStreaming) {
    if (gRuntime.wifiSleepEnabled) {
      WiFi.setSleep(false);
      gRuntime.wifiSleepEnabled = false;
    }
    return;
  }

  const uint32_t idleForMs = millis() - gRuntime.lastPortalActivityMs;
  const bool shouldSleep =
      idleForMs >= azimuth_power::wifiSleepIdleDelayMs();
  if (shouldSleep == gRuntime.wifiSleepEnabled) {
    return;
  }
  WiFi.setSleep(shouldSleep);
  gRuntime.wifiSleepEnabled = shouldSleep;
}

bool hostnameCharsValid(const char* s) {
  return azimuth_cfg::isHostnameCharsValid(s);
}

String mergedSsid() {
  String s = gRuntime.prefs.getString("ssid", "");
  if (s.length() == 0) {
    s = WIFI_SSID;
  }
  return s;
}

String mergedPass() {
  String p = gRuntime.prefs.getString("pass", "");
  if (p.length() == 0) {
    p = WIFI_PASSWORD;
  }
  return p;
}

String mergedOtHost() {
  String h = gRuntime.prefs.getString("ot_host", "");
  if (h.length() == 0) {
    h = OPENTRACK_UDP_HOST;
  }
  return h;
}

uint16_t mergedOtPort() {
  uint32_t v = gRuntime.prefs.getUInt("ot_port", OPENTRACK_UDP_PORT);
  if (v == 0 || v > 65535) {
    return static_cast<uint16_t>(OPENTRACK_UDP_PORT);
  }
  return static_cast<uint16_t>(v);
}

bool mergedUdpOn() { return gRuntime.prefs.getBool("udp_on", true); }

uint8_t mergedRgbBrightness() {
  const uint32_t v = gRuntime.prefs.getUInt("rgb_brightness", 25);
  if (v > 100) {
    return 100;
  }
  return static_cast<uint8_t>(v);
}

uint8_t mergedLedMode() {
  const uint32_t v = gRuntime.prefs.getUInt("led_mode", 0);
  if (v > 3) {
    return 0;
  }
  return static_cast<uint8_t>(v);
}

uint8_t mergedLedR() {
  if (!gRuntime.prefsOpened) {
    return 80;
  }
  return gRuntime.prefs.getUChar(kPrefsKeyLedR, 80);
}

uint8_t mergedLedG() {
  if (!gRuntime.prefsOpened) {
    return 140;
  }
  return gRuntime.prefs.getUChar(kPrefsKeyLedG, 140);
}

uint8_t mergedLedB() {
  if (!gRuntime.prefsOpened) {
    return 255;
  }
  return gRuntime.prefs.getUChar(kPrefsKeyLedB, 255);
}

uint8_t mergedBuzzerVolume() {
  const uint32_t v = gRuntime.prefs.getUInt("buzzer_volume", 25);
  if (v > 100) {
    return 100;
  }
  return static_cast<uint8_t>(v);
}

uint16_t mergedBatteryCapacityMah() {
  const uint32_t v = gRuntime.prefs.getUInt(kPrefsKeyBatteryCapacityMah, 800);
  if (v < 100 || v > 5000) {
    return 800;
  }
  return static_cast<uint16_t>(v);
}

int32_t mergedBatteryCalOffsetMv() {
  const int32_t v = gRuntime.prefs.getInt(kPrefsKeyBatteryCalOffsetMv, 0);
  if (v < -1200 || v > 1200) {
    return 0;
  }
  return v;
}

void applyIoLevelsFromPrefs() {
  azimuth_io_led::setBrightnessPercent(mergedRgbBrightness());
  const uint8_t mode = mergedLedMode();
  if (mode == 3) {
    azimuth_io_led::setManualRgb(mergedLedR(), mergedLedG(), mergedLedB());
  } else {
    azimuth_io_led::setRgbPreset(static_cast<azimuth_io_led::RgbPreset>(mode));
  }
  azimuth_io_buzzer::setVolumePercent(mergedBuzzerVolume());
}

OtAxisMapConfig mergedOtAxisMap() {
  OtAxisMapConfig c;
  otAxisMapSetDefault(&c);
  if (!gRuntime.prefsOpened) {
    return c;
  }
  for (int i = 0; i < 3; ++i) {
    char ksrc[8];
    snprintf(ksrc, sizeof(ksrc), "ot_s%d", i);
    if (gRuntime.prefs.isKey(ksrc)) {
      const uint8_t s = gRuntime.prefs.getUChar(ksrc, 255);
      if (s <= 2) {
        c.srcForRot[i] = s;
      }
    }
    char kinv[8];
    snprintf(kinv, sizeof(kinv), "ot_i%d", i);
    if (gRuntime.prefs.isKey(kinv)) {
      c.invertRot[i] = gRuntime.prefs.getBool(kinv, false);
    }
  }
  if (!otAxisMapValid(c)) {
    otAxisMapSetDefault(&c);
  }
  return c;
}

void appendOtAxesToJson(JsonDocument& doc) {
  const OtAxisMapConfig c = mergedOtAxisMap();
  JsonArray ax = doc["ot_axes"].to<JsonArray>();
  for (int i = 0; i < 3; ++i) {
    JsonObject o = ax.add<JsonObject>();
    o["src"] = c.srcForRot[i];
    o["inv"] = c.invertRot[i];
  }
}

void loadTrackingPrefs() {
  if (!ensurePrefsOpen()) {
    return;
  }
  refreshRuntimeFromPrefs();
  applyIoLevelsFromPrefs();
}

uint16_t imuPeriodMsValue() { return gRuntime.imuPeriodMsRuntime; }

bool hatireUsbValue() { return gRuntime.hatireUsbRuntime; }

const OtAxisMapConfig& otAxisMapValue() { return gRuntime.otAxisMapRuntime; }

void setStasis(bool active) {
  if (gRuntime.thermalHoldActive) {
    return;
  }
  gRuntime.stasisActive = active;
  applyAdaptiveWifiSleep();
}

}  // namespace azimuth_net

#endif  // !IMU_DEBUG_MODE
