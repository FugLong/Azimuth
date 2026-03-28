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
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <cstring>

#include "opentrack_pose.h"
#include "portal_html.h"
#include "secrets.h"

#ifndef OPENTRACK_UDP_PORT
#define OPENTRACK_UDP_PORT 4242
#endif

#ifndef AZIMUTH_FW_VERSION
#define AZIMUTH_FW_VERSION "dev"
#endif

namespace azimuth_net {

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
uint16_t gOtPort = OPENTRACK_UDP_PORT;
bool gUdpSocketOk = false;
bool gUdpSendEnabled = true;
bool gStaWebActive = false;
bool gApPortalActive = false;
/** True while we are serving the portal on SoftAP (provisioning / recovery). */
bool gSetupApMode = false;

Preferences gPrefs;
bool gPrefsOpened = false;
uint16_t gImuPeriodMsRuntime = 10;
bool gHatireUsbRuntime = true;

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
  const uint32_t v = gPrefs.getUInt("wifi_tx", 1);
  if (v > 2) {
    return 1;
  }
  return static_cast<uint8_t>(v);
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
    return;
  }
  gImuPeriodMsRuntime = mergedImuPeriodMs();
  gHatireUsbRuntime = mergedHatireUsb();
}

void applyStaWifiTxPower() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setTxPower(wifiTxFromProfile(mergedWifiTxProfile()));
  }
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

void applyOtTarget() {
  gOtPort = mergedOtPort();
  const String host = mergedOtHost();
  if (!gOtIp.fromString(host.c_str())) {
    gOtIp = INADDR_NONE;
  }
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
  http.sendHeader("Cache-Control", "no-store");
  http.send_P(200, "text/html; charset=utf-8", kPortalIndexHtml);
}

void handleStatus(WebServer& http) {
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
  doc["battery_mv"] = nullptr;
  doc["fw_version"] = AZIMUTH_FW_VERSION;
  doc["imu_period_ms"] = mergedImuPeriodMs();
  doc["hatire_usb"] = mergedHatireUsb();
  doc["hostname"] = mergedHostname();
  doc["mdns_on"] = mergedMdnsOn();
  doc["wifi_tx"] = mergedWifiTxProfile();
  http.sendHeader("Cache-Control", "no-store");
  sendJson(http, 200, doc);
}

void handleScan(WebServer& http) {
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
  http.send(200, "application/json", "{\"ok\":true}");
  http.client().stop();
  delay(200);
  ESP.restart();
}

void handleFactoryResetPost(WebServer& http) {
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

  applyStaWifiTxPower();
  tryOpenUdpSocket();
  // Modem sleep can drop or delay mDNS multicast; keep radio awake so `azimuth.local` resolves reliably.
  WiFi.setSleep(false);

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
  if (gApPortalActive) {
    gDnsCaptive.processNextRequest();
    gWebAp.handleClient();
  }
  if (gStaWebActive) {
    gWebSta.handleClient();
  }
  if (WiFi.status() == WL_CONNECTED) {
    tryOpenUdpSocket();
  }
}

void sendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg) {
  if (!gUdpSocketOk || !gUdpSendEnabled || WiFi.status() != WL_CONNECTED || !otTargetOk()) {
    return;
  }

  float rot[3];
  opentrackMapEulerDegToRot(yawDeg, pitchDeg, rollDeg, rot);
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
}

uint16_t imuPeriodMsValue() { return gImuPeriodMsRuntime; }

bool hatireUsbValue() { return gHatireUsbRuntime; }

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
