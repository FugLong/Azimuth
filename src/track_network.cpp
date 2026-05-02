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

#else

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <cstdio>
#include <cstring>

#include "opentrack_pose.h"
#include "portal_html.h"
#include "board_config.h"
#include "battery_monitor.h"
#include "io_buzzer.h"
#include "io_led.h"
#include "power_policy.h"
#include "secrets.h"

#ifndef OPENTRACK_UDP_PORT
#define OPENTRACK_UDP_PORT 4242
#endif

#ifndef AZIMUTH_FW_VERSION
#define AZIMUTH_FW_VERSION "dev"
#endif

#ifndef AZIMUTH_RELEASE_MANIFEST_URL
#define AZIMUTH_RELEASE_MANIFEST_URL "https://fuglong.github.io/Azimuth/manifest.json"
#endif
#ifndef AZIMUTH_RELEASE_FLASHER_URL
#define AZIMUTH_RELEASE_FLASHER_URL "https://fuglong.github.io/Azimuth/"
#endif

namespace azimuth_net {

OtAxisMapConfig mergedOtAxisMap();

constexpr uint16_t kWebPortSta = 8080;
constexpr uint16_t kWebPortAp = 80;
constexpr const char* kPrefsNs = "azimuth";
constexpr uint32_t kWifiConnectTimeoutMs = 12000;
constexpr const char* kSetupApSsid = "Azimuth-Setup";

WebServer gWebSta(kWebPortSta);
WebServer gWebAp(kWebPortAp);
DNSServer gDnsCaptive;
WiFiUDP gUdp;
IPAddress gOtIp;
/** Trimmed OpenTrack target from NVS / secrets (hostname or IPv4 string). */
String gOtHostTrimmed;
/** True when `gOtHostTrimmed` parsed as IPv4; false when using DNS. */
bool gOtHostIsLiteralIp = false;
uint32_t gLastOtDnsMs = 0;
uint16_t gOtPort = OPENTRACK_UDP_PORT;
bool gUdpSocketOk = false;
bool gUdpSendEnabled = true;
bool gStaWebActive = false;
bool gApPortalActive = false;
/** True while we are serving the portal on SoftAP (provisioning / recovery). */
bool gSetupApMode = false;

static bool gFwUpdateCheckDone = false;
static bool gFwUpdateAvailable = false;
static String gFwLatestVersion;

Preferences gPrefs;
bool gPrefsOpened = false;
uint16_t gImuPeriodMsRuntime = 10;
bool gHatireUsbRuntime = true;
OtAxisMapConfig gOtAxisMapRuntime{};
azimuth_power::PowerProfile gPowerProfileRuntime = azimuth_power::PowerProfile::Balanced;
uint32_t gLastPortalActivityMs = 0;
uint32_t gLastNetworkServiceMs = 0;
uint32_t gLastBackgroundTickMs = 0;
bool gWifiSleepEnabled = false;

bool ensurePrefsOpen() {
  if (gPrefsOpened) {
    return true;
  }
  gPrefsOpened = gPrefs.begin(kPrefsNs, false);
  return gPrefsOpened;
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
  const uint32_t v = gPrefs.getUInt("wifi_tx", 0);
  if (v > 2) {
    return 0;
  }
  return static_cast<uint8_t>(v);
}

azimuth_power::PowerProfile mergedPowerProfile() {
  const uint32_t v = gPrefs.getUInt("power_profile", 1);
  return azimuth_power::fromStoredValue(static_cast<uint8_t>(v));
}

uint16_t mergedImuPeriodMs() {
  const uint32_t v = gPrefs.getUInt("imu_period_ms", 10);
  if (v != 5 && v != 10 && v != 20 && v != 40) {
    return 10;
  }
  return static_cast<uint16_t>(v);
}

bool mergedHatireUsb() { return gPrefs.getBool("hatire_usb", true); }

bool mergedMdnsOn() { return gPrefs.getBool("mdns_on", true); }

String mergedHostname() {
  String h = gPrefs.getString("hostname", "");
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
  if (!gPrefsOpened) {
    otAxisMapSetDefault(&gOtAxisMapRuntime);
    return;
  }
  gImuPeriodMsRuntime = mergedImuPeriodMs();
  gHatireUsbRuntime = mergedHatireUsb();
  gOtAxisMapRuntime = mergedOtAxisMap();
  gPowerProfileRuntime = mergedPowerProfile();
}

void applyStaWifiTxPower() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setTxPower(wifiTxFromProfile(mergedWifiTxProfile()));
  }
}

void markPortalActivity() {
  gLastPortalActivityMs = millis();
}

void applyAdaptiveWifiSleep() {
  if (gSetupApMode || WiFi.status() != WL_CONNECTED) {
    if (gWifiSleepEnabled) {
      WiFi.setSleep(false);
      gWifiSleepEnabled = false;
    }
    return;
  }

  const uint32_t idleForMs = millis() - gLastPortalActivityMs;
  const bool shouldSleep =
      idleForMs >= azimuth_power::wifiSleepIdleDelayMs(gPowerProfileRuntime);
  if (shouldSleep == gWifiSleepEnabled) {
    return;
  }
  WiFi.setSleep(shouldSleep);
  gWifiSleepEnabled = shouldSleep;
}

bool hostnameCharsValid(const char* s) {
  if (!s || !*s) {
    return false;
  }
  const size_t n = strlen(s);
  if (n > 24) {
    return false;
  }
  for (size_t i = 0; i < n; ++i) {
    const unsigned char c = static_cast<unsigned char>(s[i]);
    if (c >= 'a' && c <= 'z') {
      continue;
    }
    if (c >= '0' && c <= '9') {
      continue;
    }
    if (c == '-') {
      continue;
    }
    return false;
  }
  return true;
}

String mergedSsid() {
  String s = gPrefs.getString("ssid", "");
  if (s.length() == 0) {
    s = WIFI_SSID;
  }
  return s;
}

String mergedPass() {
  String p = gPrefs.getString("pass", "");
  if (p.length() == 0) {
    p = WIFI_PASSWORD;
  }
  return p;
}

String mergedOtHost() {
  String h = gPrefs.getString("ot_host", "");
  if (h.length() == 0) {
    h = OPENTRACK_UDP_HOST;
  }
  return h;
}

uint16_t mergedOtPort() {
  uint32_t v = gPrefs.getUInt("ot_port", OPENTRACK_UDP_PORT);
  if (v == 0 || v > 65535) {
    return static_cast<uint16_t>(OPENTRACK_UDP_PORT);
  }
  return static_cast<uint16_t>(v);
}

bool mergedUdpOn() { return gPrefs.getBool("udp_on", true); }

uint8_t mergedRgbBrightness() {
  const uint32_t v = gPrefs.getUInt("rgb_brightness", 25);
  if (v > 100) {
    return 100;
  }
  return static_cast<uint8_t>(v);
}

uint8_t mergedBuzzerVolume() {
  const uint32_t v = gPrefs.getUInt("buzzer_volume", 25);
  if (v > 100) {
    return 100;
  }
  return static_cast<uint8_t>(v);
}

void applyIoLevelsFromPrefs() {
  azimuth_io_led::setBrightnessPercent(mergedRgbBrightness());
  azimuth_io_buzzer::setVolumePercent(mergedBuzzerVolume());
}

OtAxisMapConfig mergedOtAxisMap() {
  OtAxisMapConfig c;
  otAxisMapSetDefault(&c);
  if (!gPrefsOpened) {
    return c;
  }
  for (int i = 0; i < 3; ++i) {
    char ksrc[8];
    snprintf(ksrc, sizeof(ksrc), "ot_s%d", i);
    if (gPrefs.isKey(ksrc)) {
      const uint8_t s = gPrefs.getUChar(ksrc, 255);
      if (s <= 2) {
        c.srcForRot[i] = s;
      }
    }
    char kinv[8];
    snprintf(kinv, sizeof(kinv), "ot_i%d", i);
    if (gPrefs.isKey(kinv)) {
      c.invertRot[i] = gPrefs.getBool(kinv, false);
    }
  }
  if (!otAxisMapValid(c)) {
    otAxisMapSetDefault(&c);
  }
  return c;
}

/** Parse ot_axes: [{src:0..2, inv:bool}, x3]. Returns false on error. */
bool applyOtAxesFromJson(const JsonArray& arr, const char** errOut) {
  if (arr.size() != 3) {
    *errOut = "ot_axes must be an array of 3 objects";
    return false;
  }
  OtAxisMapConfig c;
  otAxisMapSetDefault(&c);
  for (size_t i = 0; i < 3; ++i) {
    JsonObjectConst o = arr[i].as<JsonObjectConst>();
    if (o.isNull()) {
      *errOut = "ot_axes[i] must be object";
      return false;
    }
    if (!o["src"].is<int>()) {
      *errOut = "ot_axes[].src must be integer 0–2";
      return false;
    }
    const int src = o["src"].as<int>();
    if (src < 0 || src > 2) {
      *errOut = "ot_axes[].src must be 0–2";
      return false;
    }
    c.srcForRot[i] = static_cast<uint8_t>(src);
    if (!o["inv"].isNull() && !o["inv"].is<bool>()) {
      *errOut = "ot_axes[].inv must be boolean";
      return false;
    }
    c.invertRot[i] = o["inv"].is<bool>() ? o["inv"].as<bool>() : false;
  }
  if (!otAxisMapValid(c)) {
    *errOut = "ot_axes must use yaw, pitch, and roll each exactly once";
    return false;
  }
  for (int i = 0; i < 3; ++i) {
    char ksrc[8];
    char kinv[8];
    snprintf(ksrc, sizeof(ksrc), "ot_s%d", i);
    snprintf(kinv, sizeof(kinv), "ot_i%d", i);
    (void)gPrefs.putUChar(ksrc, c.srcForRot[i]);
    (void)gPrefs.putBool(kinv, c.invertRot[i]);
  }
  return true;
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

void resolveOtHostnameNow() {
  if (gOtHostTrimmed.length() == 0 || gOtHostIsLiteralIp) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    gOtIp = INADDR_NONE;
    return;
  }
  IPAddress resolved;
  if (WiFi.hostByName(gOtHostTrimmed.c_str(), resolved)) {
    gOtIp = resolved;
  } else {
    gOtIp = INADDR_NONE;
  }
  gLastOtDnsMs = millis();
}

void maybeRefreshOtHostname() {
  if (gOtHostIsLiteralIp || gOtHostTrimmed.length() == 0) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  const uint32_t now = millis();
  if (now - gLastOtDnsMs < 45000) {
    return;
  }
  resolveOtHostnameNow();
}

void applyOtTarget() {
  gOtPort = mergedOtPort();
  gOtHostTrimmed = mergedOtHost();
  gOtHostTrimmed.trim();

  if (gOtHostTrimmed.length() == 0) {
    gOtIp = INADDR_NONE;
    gOtHostIsLiteralIp = true;
    return;
  }

  IPAddress asIp;
  if (asIp.fromString(gOtHostTrimmed.c_str())) {
    gOtIp = asIp;
    gOtHostIsLiteralIp = true;
    gLastOtDnsMs = millis();
    return;
  }

  gOtHostIsLiteralIp = false;
  gOtIp = INADDR_NONE;
  resolveOtHostnameNow();
}

void tryOpenUdpSocket() {
  if (gUdpSocketOk || WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (gUdp.begin(0)) {
    gUdpSocketOk = true;
  }
}

bool otTargetOk() { return gOtIp != INADDR_NONE && gOtPort != 0; }

String portalHttpUrl() { return String("http://") + WiFi.softAPIP().toString() + "/"; }

struct SemVer {
  int ma = 0;
  int mi = 0;
  int pa = 0;
};

static bool parseSemVer(const char* s, SemVer& o) {
  o = {};
  if (!s || !*s) {
    return false;
  }
  if (s[0] == 'v' || s[0] == 'V') {
    ++s;
  }
  const int n = sscanf(s, "%d.%d.%d", &o.ma, &o.mi, &o.pa);
  if (n == 1) {
    o.mi = 0;
    o.pa = 0;
  } else if (n == 2) {
    o.pa = 0;
  }
  return n >= 1;
}

static bool semverLess(const SemVer& a, const SemVer& b) {
  if (a.ma != b.ma) {
    return a.ma < b.ma;
  }
  if (a.mi != b.mi) {
    return a.mi < b.mi;
  }
  return a.pa < b.pa;
}

/** Once per STA session, after Wi‑Fi is up — compares VERSION-derived build to hosted manifest.
 *  Short timeouts: this runs synchronously inside loop(); long stalls would delay IMU + tracking. */
static void performFirmwareUpdateCheckOnce() {
  gFwUpdateCheckDone = true;
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
  SemVer cur{};
  SemVer rem{};
  if (!parseSemVer(AZIMUTH_FW_VERSION, cur) || !parseSemVer(remoteVer, rem)) {
    return;
  }
  if (semverLess(cur, rem)) {
    gFwUpdateAvailable = true;
    gFwLatestVersion = remoteVer;
  }
}

void sendCaptiveRedirect(WebServer& http) {
  const String url = portalHttpUrl();
  http.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  http.sendHeader("Location", url);
  http.send(302, "text/plain", "");
}

void sendJson(WebServer& http, int code, const JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  http.send(code, "application/json", out);
}

void handleRoot(WebServer& http) {
  markPortalActivity();
  http.sendHeader("Cache-Control", "no-store");
  http.send_P(200, "text/html; charset=utf-8", kPortalIndexHtml);
}

void handleStatus(WebServer& http) {
  markPortalActivity();
  JsonDocument doc;
  doc["setup_ap"] = gSetupApMode;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  if (gSetupApMode) {
    doc["ip"] = WiFi.softAPIP().toString();
    doc["portal_url"] = portalHttpUrl();
    doc["rssi"] = 0;
  } else {
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
  }
  doc["heap_free"] = ESP.getFreeHeap();
  doc["uptime_ms"] = millis();
  doc["ssid"] = mergedSsid();
  doc["password_set"] = (mergedPass().length() > 0);
  doc["ot_host"] = mergedOtHost();
  doc["ot_port"] = mergedOtPort();
  doc["udp_enabled"] = gUdpSendEnabled;
  doc["ot_target_ok"] = otTargetOk();
  doc["http_client_ip"] = http.client().remoteIP().toString();
  if (otTargetOk()) {
    doc["ot_resolved_ip"] = gOtIp.toString();
  } else {
    doc["ot_resolved_ip"] = nullptr;
  }
  doc["ot_using_dns"] = (!gOtHostIsLiteralIp && gOtHostTrimmed.length() > 0);
  const auto batt = azimuth_battery::readStatus();
  if (batt.millivolts >= 0) {
    doc["battery_mv"] = batt.millivolts;
  } else {
    doc["battery_mv"] = nullptr;
  }
  doc["battery_state"] = batt.stub ? "stub" : "active";
  doc["board"] = azimuth_board::boardName();
  doc["fw_version"] = AZIMUTH_FW_VERSION;
  doc["fw_update_available"] = gFwUpdateAvailable;
  if (gFwUpdateAvailable) {
    doc["fw_latest_version"] = gFwLatestVersion;
  } else {
    doc["fw_latest_version"] = nullptr;
  }
  doc["fw_flasher_url"] = AZIMUTH_RELEASE_FLASHER_URL;
  doc["imu_period_ms"] = mergedImuPeriodMs();
  doc["hatire_usb"] = mergedHatireUsb();
  doc["hostname"] = mergedHostname();
  doc["mdns_on"] = mergedMdnsOn();
  doc["wifi_tx"] = mergedWifiTxProfile();
  doc["power_profile"] = azimuth_power::toStoredValue(gPowerProfileRuntime);
  appendOtAxesToJson(doc);
  doc["rgb_brightness"] = mergedRgbBrightness();
  doc["buzzer_volume"] = mergedBuzzerVolume();
  {
    const auto caps = azimuth_board::capabilities();
    doc["has_rgb"] = caps.hasRgb;
    doc["has_buzzer"] = caps.hasBuzzer;
  }
  http.sendHeader("Cache-Control", "no-store");
  sendJson(http, 200, doc);
}

void handleScan(WebServer& http) {
  markPortalActivity();
  JsonDocument doc;
  JsonArray arr = doc["networks"].to<JsonArray>();
  const int n = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/true);
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

void handleConfigPost(WebServer& http) {
  markPortalActivity();
  if (!ensurePrefsOpen()) {
    JsonDocument err;
    err["error"] = "storage unavailable";
    sendJson(http, 500, err);
    return;
  }

  if (!http.hasArg("plain")) {
    JsonDocument err;
    err["error"] = "expected JSON body";
    sendJson(http, 400, err);
    return;
  }

  JsonDocument body;
  DeserializationError e = deserializeJson(body, http.arg("plain"));
  if (e) {
    JsonDocument err;
    err["error"] = "invalid JSON";
    sendJson(http, 400, err);
    return;
  }

  const uint16_t prevImuMs = mergedImuPeriodMs();
  const String prevHostname = mergedHostname();
  const bool prevMdns = mergedMdnsOn();

  bool wifiCredChanged = false;
  bool needReboot = false;
  const char* errMsg = nullptr;

  if (!body["ssid"].isNull()) {
    if (!body["ssid"].is<const char*>()) {
      errMsg = "ssid must be string";
    } else {
      const char* s = body["ssid"].as<const char*>();
      if (s && s[0]) {
        if (strcmp(s, mergedSsid().c_str()) != 0) {
          wifiCredChanged = true;
        }
        gPrefs.putString("ssid", s);
      }
    }
  }

  if (!errMsg && body["password"].is<const char*>()) {
    const char* p = body["password"].as<const char*>();
    if (p && p[0]) {
      wifiCredChanged = true;
      gPrefs.putString("pass", p);
    }
  }

  if (!errMsg && !body["ot_host"].isNull()) {
    if (!body["ot_host"].is<const char*>()) {
      errMsg = "ot_host must be string";
    } else {
      gPrefs.putString("ot_host", body["ot_host"].as<const char*>());
    }
  }

  if (!errMsg && !body["ot_port"].isNull()) {
    const int port = body["ot_port"].as<int>();
    if (port < 1 || port > 65535) {
      errMsg = "ot_port invalid";
    } else {
      gPrefs.putUInt("ot_port", static_cast<uint32_t>(port));
    }
  }

  if (!errMsg && !body["udp_enabled"].isNull()) {
    if (!body["udp_enabled"].is<bool>()) {
      errMsg = "udp_enabled must be boolean";
    } else {
      gPrefs.putBool("udp_on", body["udp_enabled"].as<bool>());
    }
  }

  if (!errMsg && !body["hatire_usb"].isNull()) {
    if (!body["hatire_usb"].is<bool>()) {
      errMsg = "hatire_usb must be boolean";
    } else {
      gPrefs.putBool("hatire_usb", body["hatire_usb"].as<bool>());
    }
  }

  if (!errMsg && !body["imu_period_ms"].isNull()) {
    const int p = body["imu_period_ms"].as<int>();
    if (p != 5 && p != 10 && p != 20 && p != 40) {
      errMsg = "imu_period_ms must be 5, 10, 20, or 40";
    } else {
      gPrefs.putUInt("imu_period_ms", static_cast<uint32_t>(p));
      if (static_cast<uint16_t>(p) != prevImuMs) {
        needReboot = true;
      }
    }
  }

  if (!errMsg && !body["wifi_tx"].isNull()) {
    const int tx = body["wifi_tx"].as<int>();
    if (tx < 0 || tx > 2) {
      errMsg = "wifi_tx must be 0, 1, or 2";
    } else {
      gPrefs.putUInt("wifi_tx", static_cast<uint32_t>(tx));
    }
  }

  if (!errMsg && !body["power_profile"].isNull()) {
    const int p = body["power_profile"].as<int>();
    if (p < 0 || p > 2) {
      errMsg = "power_profile must be 0, 1, or 2";
    } else {
      gPrefs.putUInt("power_profile", static_cast<uint32_t>(p));
    }
  }

  if (!errMsg && !body["rgb_brightness"].isNull()) {
    const int b = body["rgb_brightness"].as<int>();
    if (b < 0 || b > 100) {
      errMsg = "rgb_brightness must be 0–100";
    } else {
      gPrefs.putUInt("rgb_brightness", static_cast<uint32_t>(b));
    }
  }

  if (!errMsg && !body["buzzer_volume"].isNull()) {
    const int b = body["buzzer_volume"].as<int>();
    if (b < 0 || b > 100) {
      errMsg = "buzzer_volume must be 0–100";
    } else {
      gPrefs.putUInt("buzzer_volume", static_cast<uint32_t>(b));
    }
  }

  if (!errMsg && !body["mdns_on"].isNull()) {
    if (!body["mdns_on"].is<bool>()) {
      errMsg = "mdns_on must be boolean";
    } else {
      gPrefs.putBool("mdns_on", body["mdns_on"].as<bool>());
      if (body["mdns_on"].as<bool>() != prevMdns) {
        needReboot = true;
      }
    }
  }

  if (!errMsg && !body["ot_axes"].isNull()) {
    const char* axErr = nullptr;
    if (!body["ot_axes"].is<JsonArray>()) {
      errMsg = "ot_axes must be a JSON array";
    } else {
      JsonArray arr = body["ot_axes"].as<JsonArray>();
      if (!applyOtAxesFromJson(arr, &axErr)) {
        errMsg = axErr;
      }
    }
  }

  if (!errMsg && !body["hostname"].isNull()) {
    if (!body["hostname"].is<const char*>()) {
      errMsg = "hostname must be string";
    } else {
      const char* raw = body["hostname"].as<const char*>();
      if (!raw) {
        errMsg = "hostname invalid";
      } else {
        String h(raw);
        h.trim();
        for (int i = 0; i < h.length(); ++i) {
          const char c = h[i];
          if (c >= 'A' && c <= 'Z') {
            h.setCharAt(i, static_cast<char>(c - 'A' + 'a'));
          }
        }
        if (h.length() == 0) {
          gPrefs.putString("hostname", "");
          if (prevHostname != String("azimuth")) {
            needReboot = true;
          }
        } else if (!hostnameCharsValid(h.c_str())) {
          errMsg = "hostname: use lowercase letters, digits, hyphen only (max 24)";
        } else {
          gPrefs.putString("hostname", h.c_str());
          if (h != prevHostname) {
            needReboot = true;
          }
        }
      }
    }
  }

  if (errMsg) {
    JsonDocument err;
    err["error"] = errMsg;
    sendJson(http, 400, err);
    return;
  }

  if (gSetupApMode && mergedSsid().length() == 0) {
    JsonDocument err;
    err["error"] = "Enter your home Wi-Fi network name (SSID), then save.";
    sendJson(http, 400, err);
    return;
  }

  gUdpSendEnabled = mergedUdpOn();
  applyOtTarget();
  refreshRuntimeFromPrefs();
  applyIoLevelsFromPrefs();
  applyStaWifiTxPower();

  const bool restarting = wifiCredChanged || needReboot;

  JsonDocument ok;
  ok["ok"] = true;
  ok["restarting"] = restarting;

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
  gPrefs.clear();
  gPrefs.end();
  gPrefsOpened = false;
  ESP.restart();
}

void registerRoutes(WebServer& http, bool captiveProbeRedirect) {
  http.on("/", HTTP_GET, [&http]() { handleRoot(http); });
  http.on("/api/status", HTTP_GET, [&http]() { handleStatus(http); });
  http.on("/api/scan", HTTP_GET, [&http]() { handleScan(http); });
  http.on(
      "/api/config", HTTP_POST,
      [&http]() {
        http.sendHeader("Access-Control-Allow-Origin", "*");
        handleConfigPost(http);
      });
  http.on(
      "/api/reboot", HTTP_POST,
      [&http]() {
        http.sendHeader("Access-Control-Allow-Origin", "*");
        handleRebootPost(http);
      });
  http.on(
      "/api/factory_reset", HTTP_POST,
      [&http]() {
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

/** Open **Azimuth-Setup**, DNS captive hijack, HTTP :80 (OS sign-in sheet), same UI as STA :8080. */
void startProvisioningPortal() {
  gSetupApMode = true;
  gStaWebActive = false;
  gDnsCaptive.stop();
  WiFi.disconnect(true);
  delay(100);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  (void)WiFi.softAP(kSetupApSsid, nullptr, 1, 0, 4);
  const IPAddress apIp = WiFi.softAPIP();

  gDnsCaptive.setErrorReplyCode(DNSReplyCode::NoError);
  gDnsCaptive.setTTL(120);
  (void)gDnsCaptive.start(53, "*", apIp);

  registerRoutes(gWebAp, true);
  gWebAp.begin();
  gApPortalActive = true;
  gWifiSleepEnabled = false;
  gLastPortalActivityMs = millis();
}

void networkInit() {
  if (!ensurePrefsOpen()) {
    return;
  }

  refreshRuntimeFromPrefs();
  gUdpSendEnabled = mergedUdpOn();
  applyOtTarget();

  const String ssid = mergedSsid();
  if (ssid.length() == 0) {
    WiFi.persistent(false);
    startProvisioningPortal();
    return;
  }

  gSetupApMode = false;
  WiFi.persistent(false);
  WiFi.setHostname(mergedHostname().c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), mergedPass().c_str());

  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < kWifiConnectTimeoutMs) {
    delay(200);
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Wrong password, AP down, etc.: same recovery path as first boot — join Azimuth-Setup and fix Wi‑Fi.
    startProvisioningPortal();
    return;
  }

  // Let the STA netif settle before mDNS (avoids flaky hostname registration on some routers).
  delay(100);

  applyOtTarget();

  applyStaWifiTxPower();
  tryOpenUdpSocket();
  gLastPortalActivityMs = millis();
  applyAdaptiveWifiSleep();

  if (mergedMdnsOn()) {
    if (MDNS.begin(mergedHostname().c_str())) {
      (void)MDNS.addService("http", "tcp", kWebPortSta);
      (void)MDNS.enableWorkstation(ESP_IF_WIFI_STA);
    }
  }

  registerRoutes(gWebSta, false);
  gWebSta.begin();
  gStaWebActive = true;
}

void networkLoop() {
  const uint32_t now = millis();
  const uint16_t serviceInterval = azimuth_power::networkServiceIntervalMs(gPowerProfileRuntime);
  if (now - gLastNetworkServiceMs >= serviceInterval) {
    gLastNetworkServiceMs = now;
    if (gApPortalActive) {
      gDnsCaptive.processNextRequest();
      gWebAp.handleClient();
    }
    if (gStaWebActive) {
      gWebSta.handleClient();
    }
  }

  if (now - gLastBackgroundTickMs >= 200) {
    gLastBackgroundTickMs = now;
    if (WiFi.status() == WL_CONNECTED) {
      tryOpenUdpSocket();
      maybeRefreshOtHostname();
    }
    applyAdaptiveWifiSleep();
  }

  if (gStaWebActive && WiFi.status() == WL_CONNECTED && !gFwUpdateCheckDone &&
      azimuth_power::runFirmwareUpdateCheck(gPowerProfileRuntime)) {
    static uint32_t sStaStableMs = 0;
    if (sStaStableMs == 0) {
      sStaStableMs = now;
    }
    if (now - sStaStableMs >= 8000) {
      performFirmwareUpdateCheckOnce();
    }
  }
}

void sendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg) {
  if (!gUdpSocketOk || !gUdpSendEnabled || WiFi.status() != WL_CONNECTED || !otTargetOk()) {
    return;
  }

  float rot[3];
  opentrackMapEulerToRot(yawDeg, pitchDeg, rollDeg, gOtAxisMapRuntime, rot);
  const double pose[6] = {
      0.0,
      0.0,
      0.0,
      static_cast<double>(rot[0]),
      static_cast<double>(rot[1]),
      static_cast<double>(rot[2]),
  };

  if (!gUdp.beginPacket(gOtIp, gOtPort)) {
    return;
  }
  gUdp.write(reinterpret_cast<const uint8_t*>(pose), sizeof(pose));
  gUdp.endPacket();
}

void loadTrackingPrefs() {
  if (!ensurePrefsOpen()) {
    return;
  }
  refreshRuntimeFromPrefs();
  applyIoLevelsFromPrefs();
}

uint16_t imuPeriodMsValue() { return gImuPeriodMsRuntime; }

bool hatireUsbValue() { return gHatireUsbRuntime; }

const OtAxisMapConfig& otAxisMapValue() { return gOtAxisMapRuntime; }

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

#endif  // !IMU_DEBUG_MODE
