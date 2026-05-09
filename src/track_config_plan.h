#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace azimuth_cfg {

template <typename T>
struct ConfigField {
  bool present = false;
  bool typeOk = true;
  T value{};
};

struct OtAxesEntry {
  int src = 0;
  bool inv = false;
};

enum class OtAxesIssue : uint8_t {
  kNone = 0,
  kNotArray,
  kWrongSize,
  kItemNotObject,
  kSrcType,
  kSrcRange,
  kInvType,
};

struct OtAxesField {
  bool present = false;
  OtAxesIssue issue = OtAxesIssue::kNone;
  std::array<OtAxesEntry, 3> axes{};
};

struct ConfigPlanInput {
  uint16_t prevImuPeriodMs = 10;
  bool prevMdnsOn = true;
  std::string prevHostname;
  std::string prevSsid;

  ConfigField<std::string> ssid;
  ConfigField<std::string> password;
  ConfigField<std::string> otHost;
  ConfigField<int> otPort;
  ConfigField<bool> udpEnabled;
  ConfigField<bool> hatireUsb;
  ConfigField<bool> mdnsOn;
  ConfigField<int> imuPeriodMs;
  ConfigField<int> wifiTx;
  OtAxesField otAxes;
  ConfigField<int> rgbBrightness;
  ConfigField<int> ledMode;
  ConfigField<int> ledR;
  ConfigField<int> ledG;
  ConfigField<int> ledB;
  ConfigField<int> buzzerVolume;
  ConfigField<int> batteryCapacityMah;
  ConfigField<int> batteryCalOffsetMv;
  ConfigField<bool> batteryCalibrate4v2;
  ConfigField<std::string> hostname;
};

struct ConfigApplyPlan {
  bool ok = false;
  const char* errorCode = nullptr;
  const char* errorMessage = nullptr;

  bool writeSsid = false;
  std::string ssidValue;
  bool writePassword = false;
  std::string passwordValue;
  bool writeOtHost = false;
  std::string otHostValue;
  bool writeOtPort = false;
  uint16_t otPortValue = 0;
  bool writeUdpEnabled = false;
  bool udpEnabledValue = true;
  bool writeHatireUsb = false;
  bool hatireUsbValue = true;
  bool writeMdnsOn = false;
  bool mdnsOnValue = true;
  bool writeImuPeriodMs = false;
  uint16_t imuPeriodMsValue = 10;
  bool writeWifiTx = false;
  uint8_t wifiTxValue = 0;
  bool writeOtAxes = false;
  std::array<OtAxesEntry, 3> otAxesValue{};
  bool writeRgbBrightness = false;
  uint8_t rgbBrightnessValue = 25;
  bool writeLedMode = false;
  uint8_t ledModeValue = 0;
  bool writeLedR = false;
  uint8_t ledRValue = 80;
  bool writeLedG = false;
  uint8_t ledGValue = 140;
  bool writeLedB = false;
  uint8_t ledBValue = 255;
  bool writeBuzzerVolume = false;
  uint8_t buzzerVolumeValue = 25;
  bool writeBatteryCapacityMah = false;
  uint16_t batteryCapacityMahValue = 800;
  bool writeBatteryCalOffsetMv = false;
  int32_t batteryCalOffsetMvValue = 0;
  bool batteryCalibrateRequested = false;
  bool batteryCalibrateAllowed = false;
  bool writeHostname = false;
  std::string hostnameValue;

  bool wifiCredChanged = false;
  bool rebootRequired = false;
};

ConfigApplyPlan buildConfigApplyPlan(const ConfigPlanInput& in);

}  // namespace azimuth_cfg
