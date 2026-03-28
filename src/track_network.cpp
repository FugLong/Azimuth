#include "track_network.h"

#ifndef IMU_DEBUG_MODE
#define IMU_DEBUG_MODE 1
#endif

#if IMU_DEBUG_MODE

void trackNetworkInit() {}
void trackNetworkLoop() {}
void trackNetworkSendOpentrackUdp(float, float, float) {}

#else

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <cstring>

#include "secrets.h"

#ifndef OPENTRACK_UDP_PORT
#define OPENTRACK_UDP_PORT 4242
#endif

namespace {

constexpr uint16_t kWebPort = 8080;
constexpr const char* kPrefsNs = "azimuth";
constexpr wifi_power_t kWifiTxPower = WIFI_POWER_8_5dBm;
constexpr uint32_t kWifiConnectTimeoutMs = 12000;
constexpr const char* kSetupApSsid = "Azimuth-Setup";

WebServer gServer(kWebPort);
WiFiUDP gUdp;
IPAddress gOtIp;
uint16_t gOtPort = OPENTRACK_UDP_PORT;
bool gUdpSocketOk = false;
bool gUdpSendEnabled = true;
bool gWebStarted = false;
/** True while we are serving the portal on SoftAP only (no home Wi‑Fi configured). */
bool gSetupApMode = false;

Preferences gPrefs;

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

static const char kIndexHtml[] PROGMEM = R"AZPORTAL(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Azimuth</title>
<style>
:root{--bg:#0f1419;--card:#1a2332;--bd:#2d3a4d;--tx:#e8eef5;--muted:#8b9cb3;--acc:#3d9ee5;--ok:#4ade80;--warn:#fbbf24;}
*{box-sizing:border-box}
body{margin:0;font:15px/1.45 system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--tx);min-height:100vh;padding:1rem}
h1{font-size:1.35rem;font-weight:600;margin:0 0 .25rem}
.sub{color:var(--muted);font-size:.85rem;margin-bottom:1.25rem}
.card{background:var(--card);border:1px solid var(--bd);border-radius:12px;padding:1rem 1.1rem;margin-bottom:1rem;max-width:520px}
label{display:block;font-size:.8rem;color:var(--muted);margin:.65rem 0 .25rem}
input[type=text],input[type=password],input[type=number],select{width:100%;padding:.55rem .65rem;border-radius:8px;border:1px solid var(--bd);background:#0d1218;color:var(--tx)}
.row{display:flex;gap:.5rem;align-items:center;flex-wrap:wrap}
.btn{cursor:pointer;border:none;border-radius:8px;padding:.55rem 1rem;font-weight:600;font-size:.9rem}
.btn-primary{background:var(--acc);color:#061018}
.btn-sec{background:var(--bd);color:var(--tx)}
.btn-dang{background:#7f1d1d;color:#fecaca}
.toggle{width:3rem;height:1.6rem;border-radius:999px;background:var(--bd);position:relative;border:none;cursor:pointer;padding:0}
.toggle.on{background:#1e5a8a}
.toggle::after{content:'';position:absolute;width:1.25rem;height:1.25rem;border-radius:50%;background:#fff;top:.175rem;left:.2rem;transition:left .15s}
.toggle.on::after{left:1.45rem}
#msg{margin-top:.75rem;font-size:.85rem;color:var(--muted)}
#msg.err{color:#f87171}
#msg.ok{color:var(--ok)}
.scan-list{max-height:10rem;overflow:auto;border:1px solid var(--bd);border-radius:8px;margin-top:.5rem;display:none}
.scan-list div{padding:.45rem .65rem;border-bottom:1px solid var(--bd);cursor:pointer;display:flex;justify-content:space-between}
.scan-list div:hover{background:#0d1218}
.scan-list div:last-child{border-bottom:none}
pre.stats{font-size:.75rem;color:var(--muted);white-space:pre-wrap;margin:.5rem 0 0}
</style>
</head>
<body>
<h1>Azimuth</h1>
<p class="sub" id="subLine">Settings · idle cost: none until you open this page</p>
<p id="setupBanner" style="display:none;background:var(--bd);border:1px solid var(--acc);border-radius:8px;padding:.65rem .85rem;font-size:.85rem;margin:0 0 1rem;max-width:520px">Join Wi‑Fi <strong>Azimuth‑Setup</strong> on your phone or PC, then open <strong>http://192.168.4.1:8080</strong>. Save your home network below; the device reboots and this AP turns off.</p>

<div class="card">
  <strong>Wi‑Fi</strong>
  <label>SSID</label>
  <input type="text" id="ssid" autocomplete="off"/>
  <label>Password <span style="color:var(--muted)">(leave blank to keep)</span></label>
  <input type="password" id="pass" autocomplete="off"/>
  <div class="row" style="margin-top:.75rem">
    <button type="button" class="btn btn-sec" id="btnScan">Scan networks</button>
  </div>
  <div class="scan-list" id="scanList"></div>
</div>

<div class="card">
  <strong>OpenTrack UDP</strong>
  <div class="row" style="margin:.5rem 0">
    <span>Send UDP packets</span>
    <button type="button" class="toggle" id="udpToggle" aria-label="UDP"></button>
  </div>
  <label>Host (IP or hostname)</label>
  <input type="text" id="otHost"/>
  <label>Port</label>
  <input type="number" id="otPort" min="1" max="65535"/>
</div>

<div class="card">
  <strong>Device</strong>
  <p class="sub" style="margin:0 0 .5rem">Battery: not connected — placeholder for later.</p>
  <div class="row">
    <button type="button" class="btn btn-primary" id="btnSave">Save settings</button>
    <button type="button" class="btn btn-dang" id="btnReboot">Reboot</button>
  </div>
  <div id="msg"></div>
  <pre class="stats" id="stats"></pre>
</div>

<script>
const $=id=>document.getElementById(id);
function setMsg(t,cls){const m=$('msg');m.textContent=t||'';m.className=cls||''}
function setUdpUi(on){$('udpToggle').classList.toggle('on',on)}
async function load(){
  const r=await fetch('/api/status');const j=await r.json();
  const ap=!!j.setup_ap;
  $('setupBanner').style.display=ap?'block':'none';
  $('subLine').textContent=ap?'First-time Wi‑Fi setup (access point)':'Settings · idle cost: none until you open this page';
  $('ssid').value=j.ssid||'';
  $('pass').value='';
  $('otHost').value=j.ot_host||'';
  $('otPort').value=j.ot_port||4242;
  setUdpUi(!!j.udp_enabled);
  const st=(ap?'Setup AP · ':'')+'IP '+(j.ip||'—')+(ap?'':' · RSSI '+(j.rssi??'—'))+' · heap '+j.heap_free+' · up '+(Math.round(j.uptime_ms/1000))+'s\nUDP target '+(j.ot_target_ok?'OK':'bad host')+' · STA '+(j.wifi_connected?'up':'down');
  $('stats').textContent=st;
}
$('udpToggle').onclick=()=>{setUdpUi(!$('udpToggle').classList.contains('on'))};
$('btnScan').onclick=async()=>{
  setMsg('Scanning… (tracking may hitch briefly)','');
  $('scanList').style.display='none';
  try{
    const r=await fetch('/api/scan');const j=await r.json();
    const box=$('scanList');box.innerHTML='';
    (j.networks||[]).forEach(n=>{
      const d=document.createElement('div');
      d.innerHTML='<span>'+(n.ssid||'(hidden)')+'</span><span style="color:var(--muted)">'+n.rssi+' dBm</span>';
      d.onclick=()=>{$('ssid').value=n.ssid||'';box.style.display='none'};
      box.appendChild(d);
    });
    box.style.display='block';
    setMsg('','');
  }catch(e){setMsg('Scan failed','err')}
};
$('btnSave').onclick=async()=>{
  setMsg('Saving…','');
  const body={
    ssid:$('ssid').value.trim(),
    ot_host:$('otHost').value.trim(),
    ot_port:parseInt($('otPort').value,10)||4242,
    udp_enabled:$('udpToggle').classList.contains('on')
  };
  const pw=$('pass').value;
  if(pw.length)body.password=pw;
  try{
    const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
    const j=await r.json();
    if(!r.ok){setMsg(j.error||'Save failed','err');return}
    setMsg(j.restarting?'Saved — rebooting…':'Saved.','ok');
    if(!j.restarting)load();
  }catch(e){setMsg('Network error','err')}
};
$('btnReboot').onclick=async()=>{
  if(!confirm('Reboot device?'))return;
  await fetch('/api/reboot',{method:'POST'});
  setMsg('Reboot sent…','ok');
};
load();
setInterval(load,8000);
</script>
</body>
</html>
)AZPORTAL";

void sendJson(int code, const JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  gServer.send(code, "application/json", out);
}

void handleRoot() { gServer.send_P(200, "text/html; charset=utf-8", kIndexHtml); }

void handleStatus() {
  JsonDocument doc;
  doc["setup_ap"] = gSetupApMode;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  if (gSetupApMode) {
    doc["ip"] = WiFi.softAPIP().toString();
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
  gServer.sendHeader("Cache-Control", "no-store");
  sendJson(200, doc);
}

void handleScan() {
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
  gServer.sendHeader("Cache-Control", "no-store");
  sendJson(200, doc);
}

void handleConfigPost() {
  if (!gServer.hasArg("plain")) {
    JsonDocument err;
    err["error"] = "expected JSON body";
    sendJson(400, err);
    return;
  }

  JsonDocument body;
  DeserializationError e = deserializeJson(body, gServer.arg("plain"));
  if (e) {
    JsonDocument err;
    err["error"] = "invalid JSON";
    sendJson(400, err);
    return;
  }

  bool wifiCredChanged = false;
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

  if (errMsg) {
    JsonDocument err;
    err["error"] = errMsg;
    sendJson(400, err);
    return;
  }

  if (gSetupApMode && mergedSsid().length() == 0) {
    JsonDocument err;
    err["error"] = "Enter your home Wi-Fi network name (SSID), then save.";
    sendJson(400, err);
    return;
  }

  gUdpSendEnabled = mergedUdpOn();
  applyOtTarget();

  JsonDocument ok;
  ok["ok"] = true;
  ok["restarting"] = wifiCredChanged;

  if (wifiCredChanged) {
    String out;
    serializeJson(ok, out);
    gServer.send(200, "application/json", out);
    gServer.client().stop();
    delay(400);
    ESP.restart();
  } else {
    sendJson(200, ok);
  }
}

void handleRebootPost() {
  gServer.send(200, "application/json", "{\"ok\":true}");
  gServer.client().stop();
  delay(200);
  ESP.restart();
}

void registerRoutes() {
  gServer.on("/", HTTP_GET, handleRoot);
  gServer.on("/api/status", HTTP_GET, handleStatus);
  gServer.on("/api/scan", HTTP_GET, handleScan);
  gServer.on(
      "/api/config", HTTP_POST,
      []() {
        gServer.sendHeader("Access-Control-Allow-Origin", "*");
        handleConfigPost();
      });
  gServer.on(
      "/api/reboot", HTTP_POST,
      []() {
        gServer.sendHeader("Access-Control-Allow-Origin", "*");
        handleRebootPost();
      });
  gServer.onNotFound([]() { gServer.send(404, "text/plain", "not found"); });
}

}  // namespace

void trackNetworkInit() {
  if (!gPrefs.begin(kPrefsNs, false)) {
    return;
  }

  gUdpSendEnabled = mergedUdpOn();
  applyOtTarget();

  const String ssid = mergedSsid();
  if (ssid.length() == 0) {
    // No SSID in NVS or secrets.h: open provisioning AP so the user can configure Wi‑Fi once.
    gSetupApMode = true;
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP_STA);
    (void)WiFi.softAP(kSetupApSsid, nullptr, 1, 0, 4);
    registerRoutes();
    gServer.begin();
    gWebStarted = true;
    return;
  }

  gSetupApMode = false;
  WiFi.persistent(false);
  WiFi.setHostname("azimuth");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), mergedPass().c_str());

  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < kWifiConnectTimeoutMs) {
    delay(200);
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  // Let the STA netif settle before mDNS (avoids flaky hostname registration on some routers).
  delay(100);

  WiFi.setTxPower(kWifiTxPower);
  tryOpenUdpSocket();
  // Modem sleep can drop or delay mDNS multicast; keep radio awake so `azimuth.local` resolves reliably.
  WiFi.setSleep(false);

  if (MDNS.begin("azimuth")) {
    (void)MDNS.addService("http", "tcp", kWebPort);
    (void)MDNS.enableWorkstation(ESP_IF_WIFI_STA);
  }

  registerRoutes();
  gServer.begin();
  gWebStarted = true;
}

void trackNetworkLoop() {
  if (gWebStarted) {
    gServer.handleClient();
  }
  if (WiFi.status() == WL_CONNECTED) {
    tryOpenUdpSocket();
  }
}

void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg) {
  if (!gUdpSocketOk || !gUdpSendEnabled || WiFi.status() != WL_CONNECTED || !otTargetOk()) {
    return;
  }

  const float r0 = yawDeg;
  const float r1 = rollDeg;
  const float r2 = -pitchDeg;
  const double pose[6] = {
      0.0,
      0.0,
      0.0,
      static_cast<double>(r0),
      static_cast<double>(r1),
      static_cast<double>(r2),
  };

  if (!gUdp.beginPacket(gOtIp, gOtPort)) {
    return;
  }
  gUdp.write(reinterpret_cast<const uint8_t*>(pose), sizeof(pose));
  gUdp.endPacket();
}

#endif  // !IMU_DEBUG_MODE
