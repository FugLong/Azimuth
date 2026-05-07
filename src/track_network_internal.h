#pragma once

#ifndef IMU_DEBUG_MODE
#define IMU_DEBUG_MODE 1
#endif

#if !IMU_DEBUG_MODE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFiUdp.h>

#include "opentrack_pose.h"

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

constexpr uint16_t kWebPortSta = 8080;
constexpr uint16_t kWebPortAp = 80;
constexpr const char* kPrefsNs = "azimuth";
constexpr const char* kPrefsKeyBatteryCapacityMah = "bat_cap";
constexpr const char* kPrefsKeyBatteryCalOffsetMv = "bat_cal";
constexpr const char* kPrefsKeyLedR = "led_r";
constexpr const char* kPrefsKeyLedG = "led_g";
constexpr const char* kPrefsKeyLedB = "led_b";
constexpr uint32_t kWifiConnectTimeoutMs = 12000;
constexpr const char* kSetupApSsid = "Azimuth-Setup";

struct NetworkRuntime {
  WebServer webSta;
  WebServer webAp;
  DNSServer dnsCaptive;
  WiFiUDP udp;
  IPAddress otIp;
  String otHostTrimmed;
  bool otHostIsLiteralIp = false;
  uint32_t lastOtDnsMs = 0;
  uint16_t otPort = OPENTRACK_UDP_PORT;
  bool udpSocketOk = false;
  bool udpSendEnabled = true;
  bool staWebActive = false;
  bool apPortalActive = false;
  bool setupApMode = false;
  bool fwUpdateCheckDone = false;
  bool fwUpdateAvailable = false;
  String fwLatestVersion;
  Preferences prefs;
  bool prefsOpened = false;
  uint16_t imuPeriodMsRuntime = 10;
  bool hatireUsbRuntime = true;
  OtAxisMapConfig otAxisMapRuntime{};
  uint32_t lastPortalActivityMs = 0;
  uint32_t lastNetworkServiceMs = 0;
  uint32_t lastBackgroundTickMs = 0;
  bool wifiSleepEnabled = false;
  bool thermalHoldActive = false;
  bool stasisActive = false;

  NetworkRuntime();
};

extern NetworkRuntime gRuntime;

bool ensurePrefsOpen();
wifi_power_t wifiTxFromProfile(uint8_t profile);
uint8_t mergedWifiTxProfile();
uint16_t mergedImuPeriodMs();
bool mergedHatireUsb();
bool mergedMdnsOn();
String mergedHostname();
void refreshRuntimeFromPrefs();
void applyStaWifiTxPower();
void markPortalActivity();
void applyAdaptiveWifiSleep();
bool hostnameCharsValid(const char* s);
String mergedSsid();
String mergedPass();
String mergedOtHost();
uint16_t mergedOtPort();
bool mergedUdpOn();
uint8_t mergedRgbBrightness();
uint8_t mergedLedMode();
uint8_t mergedLedR();
uint8_t mergedLedG();
uint8_t mergedLedB();
uint8_t mergedBuzzerVolume();
uint16_t mergedBatteryCapacityMah();
int32_t mergedBatteryCalOffsetMv();
void applyIoLevelsFromPrefs();
OtAxisMapConfig mergedOtAxisMap();
bool applyOtAxesFromJson(const JsonArray& arr, const char** errOut);
void appendOtAxesToJson(JsonDocument& doc);

void maybeRefreshOtHostname();
void applyOtTarget();
void tryOpenUdpSocket();
bool otTargetOk();
void sendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);

String portalHttpUrl();
void sendJson(WebServer& http, int code, const JsonDocument& doc);
void registerRoutes(WebServer& http, bool captiveProbeRedirect);
void performFirmwareUpdateCheckOnce();

void applyThermalEmergency();
void startProvisioningPortal();
void networkInit();
void networkLoop();
void loadTrackingPrefs();
uint16_t imuPeriodMsValue();
bool hatireUsbValue();
const OtAxisMapConfig& otAxisMapValue();
void setStasis(bool active);

}  // namespace azimuth_net

#endif  // !IMU_DEBUG_MODE
