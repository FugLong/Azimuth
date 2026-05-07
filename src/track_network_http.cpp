#include "track_network_internal.h"

#if !IMU_DEBUG_MODE

#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "battery_monitor.h"
#include "board_config.h"
#include "portal_html.h"
#include "secrets.h"
#include "thermal_monitor.h"
#include "track_config_plan.h"
#include "track_config_validation.h"
#include "track_version.h"

namespace azimuth_net {

String portalHttpUrl() { return String("http://") + WiFi.softAPIP().toString() + "/"; }

namespace {
void sendCaptiveRedirect(WebServer& http) {
  const String url = portalHttpUrl();
  http.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  http.sendHeader("Location", url);
  http.send(302, "text/plain", "");
}

void handleRoot(WebServer& http) {
  markPortalActivity();
  http.sendHeader("Cache-Control", "no-store");
  http.send_P(200, "text/html; charset=utf-8", kPortalIndexHtml);
}

void appendStatusWifi(JsonDocument& doc, WebServer& http) {
  doc["setup_ap"] = gRuntime.setupApMode;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  if (gRuntime.setupApMode) {
    doc["ip"] = WiFi.softAPIP().toString();
    doc["portal_url"] = portalHttpUrl();
    doc["rssi"] = 0;
  } else {
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
  }
  doc["http_client_ip"] = http.client().remoteIP().toString();
}

void appendStatusTracking(JsonDocument& doc) {
  doc["uptime_ms"] = millis();
  doc["heap_free"] = ESP.getFreeHeap();
  doc["ssid"] = mergedSsid();
  doc["password_set"] = (mergedPass().length() > 0);
  doc["ot_host"] = mergedOtHost();
  doc["ot_port"] = mergedOtPort();
  doc["udp_enabled"] = gRuntime.udpSendEnabled;
  doc["ot_target_ok"] = otTargetOk();
  if (otTargetOk()) {
    doc["ot_resolved_ip"] = gRuntime.otIp.toString();
  } else {
    doc["ot_resolved_ip"] = nullptr;
  }
  doc["ot_using_dns"] = (!gRuntime.otHostIsLiteralIp && gRuntime.otHostTrimmed.length() > 0);
  doc["imu_period_ms"] = mergedImuPeriodMs();
  doc["hatire_usb"] = mergedHatireUsb();
  doc["hostname"] = mergedHostname();
  doc["mdns_on"] = mergedMdnsOn();
  doc["wifi_tx"] = mergedWifiTxProfile();
  appendOtAxesToJson(doc);
}

void appendStatusBattery(JsonDocument& doc) {
  const auto batt = azimuth_battery::readStatus();
  if (batt.millivolts >= 0) {
    doc["battery_mv"] = batt.millivolts;
  } else {
    doc["battery_mv"] = nullptr;
  }
  if (batt.rawMillivolts >= 0) {
    doc["battery_raw_mv"] = batt.rawMillivolts;
  } else {
    doc["battery_raw_mv"] = nullptr;
  }
  if (batt.percent >= 0) {
    doc["battery_percent"] = batt.percent;
  } else {
    doc["battery_percent"] = nullptr;
  }
  if (batt.remainingMah >= 0) {
    doc["battery_remaining_mah"] = batt.remainingMah;
  } else {
    doc["battery_remaining_mah"] = nullptr;
  }
  doc["battery_capacity_mah"] = mergedBatteryCapacityMah();
  doc["battery_cal_offset_mv"] = mergedBatteryCalOffsetMv();
  doc["battery_charge_state"] = batt.chargeState;
  if (!batt.supported) {
    doc["battery_state"] = "unsupported";
  } else if (strcmp(batt.chargeState, "absent") == 0) {
    doc["battery_state"] = "absent";
  } else if (batt.stub) {
    doc["battery_state"] = "stub";
  } else {
    doc["battery_state"] = "active";
  }
}

void appendStatusDevice(JsonDocument& doc) {
  doc["board"] = azimuth_board::boardName();
  doc["fw_version"] = AZIMUTH_FW_VERSION;
  doc["fw_update_available"] = gRuntime.fwUpdateAvailable;
  if (gRuntime.fwUpdateAvailable) {
    doc["fw_latest_version"] = gRuntime.fwLatestVersion;
  } else {
    doc["fw_latest_version"] = nullptr;
  }
  doc["fw_flasher_url"] = AZIMUTH_RELEASE_FLASHER_URL;
  doc["rgb_brightness"] = mergedRgbBrightness();
  doc["buzzer_volume"] = mergedBuzzerVolume();
  doc["led_mode"] = mergedLedMode();
  doc["led_r"] = mergedLedR();
  doc["led_g"] = mergedLedG();
  doc["led_b"] = mergedLedB();
  {
    const auto caps = azimuth_board::capabilities();
    doc["has_rgb"] = caps.hasRgb;
    doc["has_buzzer"] = caps.hasBuzzer;
  }
  {
    const float tc = azimuth_thermal::lastChipTempC();
    if (!std::isnan(tc)) {
      doc["chip_temp_c"] = tc;
    } else {
      doc["chip_temp_c"] = nullptr;
    }
  }
  doc["thermal_state"] = azimuth_thermal::stateJsonString();
  doc["thermal_hold"] = gRuntime.thermalHoldActive;
  doc["stasis"] = gRuntime.stasisActive;
}

void handleStatus(WebServer& http) {
  JsonDocument doc;
  appendStatusWifi(doc, http);
  appendStatusTracking(doc);
  appendStatusBattery(doc);
  appendStatusDevice(doc);
  http.sendHeader("Cache-Control", "no-store");
  sendJson(http, 200, doc);
}

void handleScan(WebServer& http) {
  markPortalActivity();
  JsonDocument doc;
  JsonArray arr = doc["networks"].to<JsonArray>();
  const int n = WiFi.scanNetworks(false, true);
  for (int i = 0; i < n; ++i) {
    JsonObject o = arr.add<JsonObject>();
    o["ssid"] = WiFi.SSID(i);
    o["rssi"] = WiFi.RSSI(i);
    o["enc"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? 0 : 1;
  }
  WiFi.scanDelete();
  http.sendHeader("Cache-Control", "no-store");
  sendJson(http, 200, doc);
}

template <typename T>
void parseField(const JsonDocument& body, const char* key, azimuth_cfg::ConfigField<T>& out);

template <>
void parseField<std::string>(const JsonDocument& body, const char* key,
                             azimuth_cfg::ConfigField<std::string>& out) {
  const JsonVariantConst v = body[key];
  if (v.isNull()) {
    return;
  }
  out.present = true;
  if (!v.is<const char*>()) {
    out.typeOk = false;
    return;
  }
  const char* s = v.as<const char*>();
  out.value = s ? s : "";
}

template <>
void parseField<int>(const JsonDocument& body, const char* key, azimuth_cfg::ConfigField<int>& out) {
  const JsonVariantConst v = body[key];
  if (v.isNull()) {
    return;
  }
  out.present = true;
  if (!v.is<int>()) {
    out.typeOk = false;
    return;
  }
  out.value = v.as<int>();
}

template <>
void parseField<bool>(const JsonDocument& body, const char* key,
                      azimuth_cfg::ConfigField<bool>& out) {
  const JsonVariantConst v = body[key];
  if (v.isNull()) {
    return;
  }
  out.present = true;
  if (!v.is<bool>()) {
    out.typeOk = false;
    return;
  }
  out.value = v.as<bool>();
}

void parseOtAxesField(const JsonDocument& body, azimuth_cfg::OtAxesField& out) {
  const JsonVariantConst v = body["ot_axes"];
  if (v.isNull()) {
    return;
  }
  out.present = true;
  if (!v.is<JsonArrayConst>()) {
    out.issue = azimuth_cfg::OtAxesIssue::kNotArray;
    return;
  }
  JsonArrayConst arr = v.as<JsonArrayConst>();
  if (arr.size() != 3) {
    out.issue = azimuth_cfg::OtAxesIssue::kWrongSize;
    return;
  }
  for (size_t i = 0; i < 3; ++i) {
    JsonObjectConst o = arr[i].as<JsonObjectConst>();
    if (o.isNull()) {
      out.issue = azimuth_cfg::OtAxesIssue::kItemNotObject;
      return;
    }
    if (!o["src"].is<int>()) {
      out.issue = azimuth_cfg::OtAxesIssue::kSrcType;
      return;
    }
    const int src = o["src"].as<int>();
    if (src < 0 || src > 2) {
      out.issue = azimuth_cfg::OtAxesIssue::kSrcRange;
      return;
    }
    if (!o["inv"].isNull() && !o["inv"].is<bool>()) {
      out.issue = azimuth_cfg::OtAxesIssue::kInvType;
      return;
    }
    out.axes[i].src = src;
    out.axes[i].inv = o["inv"].is<bool>() ? o["inv"].as<bool>() : false;
  }
}

azimuth_cfg::ConfigPlanInput parseConfigPlanInput(const JsonDocument& body) {
  azimuth_cfg::ConfigPlanInput in;
  in.setupApMode = gRuntime.setupApMode;
  in.prevImuPeriodMs = mergedImuPeriodMs();
  in.prevMdnsOn = mergedMdnsOn();
  in.prevHostname = mergedHostname().c_str();
  in.prevSsid = mergedSsid().c_str();

  parseField<std::string>(body, "ssid", in.ssid);
  parseField<std::string>(body, "password", in.password);
  parseField<std::string>(body, "ot_host", in.otHost);
  parseField<int>(body, "ot_port", in.otPort);
  parseField<bool>(body, "udp_enabled", in.udpEnabled);
  parseField<bool>(body, "hatire_usb", in.hatireUsb);
  parseField<bool>(body, "mdns_on", in.mdnsOn);
  parseField<int>(body, "imu_period_ms", in.imuPeriodMs);
  parseField<int>(body, "wifi_tx", in.wifiTx);
  parseOtAxesField(body, in.otAxes);
  parseField<int>(body, "rgb_brightness", in.rgbBrightness);
  parseField<int>(body, "led_mode", in.ledMode);
  parseField<int>(body, "led_r", in.ledR);
  parseField<int>(body, "led_g", in.ledG);
  parseField<int>(body, "led_b", in.ledB);
  parseField<int>(body, "buzzer_volume", in.buzzerVolume);
  parseField<int>(body, "battery_capacity_mah", in.batteryCapacityMah);
  parseField<int>(body, "battery_cal_offset_mv", in.batteryCalOffsetMv);
  parseField<bool>(body, "battery_calibrate_4v2", in.batteryCalibrate4v2);
  parseField<std::string>(body, "hostname", in.hostname);
  return in;
}

void applyOtAxesPrefs(const std::array<azimuth_cfg::OtAxesEntry, 3>& axes) {
  for (int i = 0; i < 3; ++i) {
    char ksrc[8];
    char kinv[8];
    snprintf(ksrc, sizeof(ksrc), "ot_s%d", i);
    snprintf(kinv, sizeof(kinv), "ot_i%d", i);
    (void)gRuntime.prefs.putUChar(ksrc, static_cast<uint8_t>(axes[i].src));
    (void)gRuntime.prefs.putBool(kinv, axes[i].inv);
  }
}

void applyPlanToPrefs(const azimuth_cfg::ConfigApplyPlan& plan) {
  if (plan.writeSsid) gRuntime.prefs.putString("ssid", plan.ssidValue.c_str());
  if (plan.writePassword) gRuntime.prefs.putString("pass", plan.passwordValue.c_str());
  if (plan.writeOtHost) gRuntime.prefs.putString("ot_host", plan.otHostValue.c_str());
  if (plan.writeOtPort) gRuntime.prefs.putUInt("ot_port", static_cast<uint32_t>(plan.otPortValue));
  if (plan.writeUdpEnabled) gRuntime.prefs.putBool("udp_on", plan.udpEnabledValue);
  if (plan.writeHatireUsb) gRuntime.prefs.putBool("hatire_usb", plan.hatireUsbValue);
  if (plan.writeMdnsOn) gRuntime.prefs.putBool("mdns_on", plan.mdnsOnValue);
  if (plan.writeImuPeriodMs) {
    gRuntime.prefs.putUInt("imu_period_ms", static_cast<uint32_t>(plan.imuPeriodMsValue));
  }
  if (plan.writeWifiTx) gRuntime.prefs.putUInt("wifi_tx", static_cast<uint32_t>(plan.wifiTxValue));
  if (plan.writeOtAxes) applyOtAxesPrefs(plan.otAxesValue);
  if (plan.writeRgbBrightness) {
    gRuntime.prefs.putUInt("rgb_brightness", static_cast<uint32_t>(plan.rgbBrightnessValue));
  }
  if (plan.writeLedMode) gRuntime.prefs.putUInt("led_mode", static_cast<uint32_t>(plan.ledModeValue));
  if (plan.writeLedR) gRuntime.prefs.putUChar(kPrefsKeyLedR, plan.ledRValue);
  if (plan.writeLedG) gRuntime.prefs.putUChar(kPrefsKeyLedG, plan.ledGValue);
  if (plan.writeLedB) gRuntime.prefs.putUChar(kPrefsKeyLedB, plan.ledBValue);
  if (plan.writeBuzzerVolume) {
    gRuntime.prefs.putUInt("buzzer_volume", static_cast<uint32_t>(plan.buzzerVolumeValue));
  }
  if (plan.writeBatteryCapacityMah) {
    gRuntime.prefs.putUInt(kPrefsKeyBatteryCapacityMah,
                           static_cast<uint32_t>(plan.batteryCapacityMahValue));
  }
  if (plan.writeBatteryCalOffsetMv) {
    gRuntime.prefs.putInt(kPrefsKeyBatteryCalOffsetMv, plan.batteryCalOffsetMvValue);
  }
  if (plan.writeHostname) gRuntime.prefs.putString("hostname", plan.hostnameValue.c_str());
}

void sendJsonError(WebServer& http, int code, const char* msg) {
  JsonDocument err;
  err["error"] = msg;
  sendJson(http, code, err);
}

void handleConfigPost(WebServer& http) {
  markPortalActivity();
  if (!ensurePrefsOpen()) {
    sendJsonError(http, 500, "storage unavailable");
    return;
  }
  if (!http.hasArg("plain")) {
    sendJsonError(http, 400, "expected JSON body");
    return;
  }

  JsonDocument body;
  DeserializationError e = deserializeJson(body, http.arg("plain"));
  if (e) {
    sendJsonError(http, 400, "invalid JSON");
    return;
  }

  const azimuth_cfg::ConfigPlanInput in = parseConfigPlanInput(body);
  azimuth_cfg::ConfigApplyPlan plan = azimuth_cfg::buildConfigApplyPlan(in);
  if (!plan.ok) {
    sendJsonError(http, 400, plan.errorMessage);
    return;
  }

  bool didBatteryCal = false;
  int32_t batteryCalOffsetMv = mergedBatteryCalOffsetMv();
  int32_t batteryCalRawMv = 0;
  if (plan.batteryCalibrateRequested && plan.batteryCalibrateAllowed) {
    int32_t off = 0;
    int32_t avgRaw = 0;
    if (!azimuth_battery::calibrateAgainstKnownPackMv(4200, 3000, &off, &avgRaw)) {
      sendJsonError(http, 400, "battery_calibrate_4v2 failed (no stable battery sample)");
      return;
    }
    didBatteryCal = true;
    batteryCalOffsetMv = off;
    batteryCalRawMv = avgRaw;
    plan.writeBatteryCalOffsetMv = true;
    plan.batteryCalOffsetMvValue = off;
  }

  applyPlanToPrefs(plan);

  uint16_t batteryCapacityMah = mergedBatteryCapacityMah();
  int32_t batteryOffsetMv = mergedBatteryCalOffsetMv();
  if (plan.writeBatteryCapacityMah) {
    batteryCapacityMah = plan.batteryCapacityMahValue;
  }
  if (plan.writeBatteryCalOffsetMv) {
    batteryOffsetMv = plan.batteryCalOffsetMvValue;
  }

  gRuntime.udpSendEnabled = mergedUdpOn();
  applyOtTarget();
  refreshRuntimeFromPrefs();
  azimuth_battery::setCapacityMah(batteryCapacityMah);
  azimuth_battery::setCalibrationOffsetMv(batteryOffsetMv);
  applyIoLevelsFromPrefs();
  applyStaWifiTxPower();

  const bool restarting = plan.wifiCredChanged || plan.rebootRequired;
  JsonDocument ok;
  ok["ok"] = true;
  ok["restarting"] = restarting;
  if (didBatteryCal) {
    ok["battery_calibrated"] = true;
    ok["battery_cal_offset_mv"] = batteryCalOffsetMv;
    ok["battery_cal_raw_mv"] = batteryCalRawMv;
  }
  if (restarting) {
    String out;
    serializeJson(ok, out);
    http.send(200, "application/json", out);
    http.client().stop();
    delay(400);
    ESP.restart();
  } else {
    sendJson(http, 200, ok);
  }
}

void handleRebootPost(WebServer& http) {
  markPortalActivity();
  http.send(200, "application/json", "{\"ok\":true}");
  http.client().stop();
  delay(200);
  ESP.restart();
}

void handleFactoryResetPost(WebServer& http) {
  markPortalActivity();
  if (!ensurePrefsOpen()) {
    JsonDocument err;
    err["error"] = "prefs";
    sendJson(http, 500, err);
    return;
  }
  http.send(200, "application/json", "{\"ok\":true}");
  http.client().stop();
  delay(400);
  gRuntime.prefs.clear();
  gRuntime.prefs.end();
  gRuntime.prefsOpened = false;
  ESP.restart();
}
}  // namespace

void sendJson(WebServer& http, int code, const JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  http.send(code, "application/json", out);
}

void performFirmwareUpdateCheckOnce() {
  gRuntime.fwUpdateCheckDone = true;
  WiFiClientSecure client;
  client.setTimeout(2500);
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(2500);
  if (!http.begin(client, AZIMUTH_RELEASE_MANIFEST_URL)) {
    return;
  }
  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    return;
  }
  const String payload = http.getString();
  http.end();
  JsonDocument doc;
  const DeserializationError jerr = deserializeJson(doc, payload);
  if (jerr) {
    return;
  }
  const char* remoteVer = doc["version"].as<const char*>();
  if (!remoteVer || !remoteVer[0]) {
    return;
  }
  azimuth_version::SemVer cur{};
  azimuth_version::SemVer rem{};
  if (!azimuth_version::parseSemVer(AZIMUTH_FW_VERSION, cur) ||
      !azimuth_version::parseSemVer(remoteVer, rem)) {
    return;
  }
  if (azimuth_version::semverLess(cur, rem)) {
    gRuntime.fwUpdateAvailable = true;
    gRuntime.fwLatestVersion = remoteVer;
  }
}

void registerRoutes(WebServer& http, bool captiveProbeRedirect) {
  http.on("/", HTTP_GET, [&http]() { handleRoot(http); });
  http.on("/api/status", HTTP_GET, [&http]() { handleStatus(http); });
  http.on("/api/scan", HTTP_GET, [&http]() { handleScan(http); });
  http.on("/api/config", HTTP_POST, [&http]() {
    http.sendHeader("Access-Control-Allow-Origin", "*");
    handleConfigPost(http);
  });
  http.on("/api/reboot", HTTP_POST, [&http]() {
    http.sendHeader("Access-Control-Allow-Origin", "*");
    handleRebootPost(http);
  });
  http.on("/api/factory_reset", HTTP_POST, [&http]() {
    http.sendHeader("Access-Control-Allow-Origin", "*");
    handleFactoryResetPost(http);
  });
  if (captiveProbeRedirect) {
    http.onNotFound([&http]() {
      const HTTPMethod m = http.method();
      if (m != HTTP_GET && m != HTTP_HEAD) {
        http.send(404, "text/plain", "not found");
        return;
      }
      sendCaptiveRedirect(http);
    });
  } else {
    http.onNotFound([&http]() { http.send(404, "text/plain", "not found"); });
  }
}

}  // namespace azimuth_net

#endif  // !IMU_DEBUG_MODE
