#include "track_config_plan.h"

#include "track_config_validation.h"

namespace azimuth_cfg {
namespace {

void fail(ConfigApplyPlan& plan, const char* code, const char* message) {
  plan.ok = false;
  plan.errorCode = code;
  plan.errorMessage = message;
}

bool hasTypeError(const ConfigApplyPlan& plan) {
  return plan.errorMessage != nullptr;
}

bool otAxesIsPermutation(const std::array<OtAxesEntry, 3>& axes) {
  bool seen[3] = {false, false, false};
  for (const auto& axis : axes) {
    if (axis.src < 0 || axis.src > 2) {
      return false;
    }
    if (seen[axis.src]) {
      return false;
    }
    seen[axis.src] = true;
  }
  return seen[0] && seen[1] && seen[2];
}

}  // namespace

ConfigApplyPlan buildConfigApplyPlan(const ConfigPlanInput& in) {
  ConfigApplyPlan plan;
  plan.ok = false;

  // Type/shape phase: deterministic first error before value/range checks.
  if (in.ssid.present && !in.ssid.typeOk) {
    fail(plan, "type.ssid", "ssid must be string");
    return plan;
  }
  if (in.password.present && !in.password.typeOk) {
    fail(plan, "type.password", "password must be string");
    return plan;
  }
  if (in.otHost.present && !in.otHost.typeOk) {
    fail(plan, "type.ot_host", "ot_host must be string");
    return plan;
  }
  if (in.otPort.present && !in.otPort.typeOk) {
    fail(plan, "type.ot_port", "ot_port must be integer");
    return plan;
  }
  if (in.udpEnabled.present && !in.udpEnabled.typeOk) {
    fail(plan, "type.udp_enabled", "udp_enabled must be boolean");
    return plan;
  }
  if (in.hatireUsb.present && !in.hatireUsb.typeOk) {
    fail(plan, "type.hatire_usb", "hatire_usb must be boolean");
    return plan;
  }
  if (in.mdnsOn.present && !in.mdnsOn.typeOk) {
    fail(plan, "type.mdns_on", "mdns_on must be boolean");
    return plan;
  }
  if (in.imuPeriodMs.present && !in.imuPeriodMs.typeOk) {
    fail(plan, "type.imu_period_ms", "imu_period_ms must be integer");
    return plan;
  }
  if (in.wifiTx.present && !in.wifiTx.typeOk) {
    fail(plan, "type.wifi_tx", "wifi_tx must be integer");
    return plan;
  }
  if (in.otAxes.present) {
    switch (in.otAxes.issue) {
      case OtAxesIssue::kNone:
        break;
      case OtAxesIssue::kNotArray:
        fail(plan, "type.ot_axes", "ot_axes must be a JSON array");
        return plan;
      case OtAxesIssue::kWrongSize:
        fail(plan, "shape.ot_axes.size", "ot_axes must be an array of 3 objects");
        return plan;
      case OtAxesIssue::kItemNotObject:
        fail(plan, "shape.ot_axes.item", "ot_axes[i] must be object");
        return plan;
      case OtAxesIssue::kSrcType:
        fail(plan, "type.ot_axes.src", "ot_axes[].src must be integer 0-2");
        return plan;
      case OtAxesIssue::kSrcRange:
        fail(plan, "range.ot_axes.src", "ot_axes[].src must be 0-2");
        return plan;
      case OtAxesIssue::kInvType:
        fail(plan, "type.ot_axes.inv", "ot_axes[].inv must be boolean");
        return plan;
    }
  }
  if (in.rgbBrightness.present && !in.rgbBrightness.typeOk) {
    fail(plan, "type.rgb_brightness", "rgb_brightness must be integer");
    return plan;
  }
  if (in.ledMode.present && !in.ledMode.typeOk) {
    fail(plan, "type.led_mode", "led_mode must be integer");
    return plan;
  }
  if (in.ledR.present && !in.ledR.typeOk) {
    fail(plan, "type.led_channel", "led_r, led_g, led_b must be integers 0-255");
    return plan;
  }
  if (in.ledG.present && !in.ledG.typeOk) {
    fail(plan, "type.led_channel", "led_r, led_g, led_b must be integers 0-255");
    return plan;
  }
  if (in.ledB.present && !in.ledB.typeOk) {
    fail(plan, "type.led_channel", "led_r, led_g, led_b must be integers 0-255");
    return plan;
  }
  if (in.buzzerVolume.present && !in.buzzerVolume.typeOk) {
    fail(plan, "type.buzzer_volume", "buzzer_volume must be integer");
    return plan;
  }
  if (in.batteryCapacityMah.present && !in.batteryCapacityMah.typeOk) {
    fail(plan, "type.battery_capacity_mah", "battery_capacity_mah must be integer");
    return plan;
  }
  if (in.batteryCalOffsetMv.present && !in.batteryCalOffsetMv.typeOk) {
    fail(plan, "type.battery_cal_offset_mv", "battery_cal_offset_mv must be integer");
    return plan;
  }
  if (in.batteryCalibrate4v2.present && !in.batteryCalibrate4v2.typeOk) {
    fail(plan, "type.battery_calibrate_4v2", "battery_calibrate_4v2 must be boolean");
    return plan;
  }
  if (in.hostname.present && !in.hostname.typeOk) {
    fail(plan, "type.hostname", "hostname must be string");
    return plan;
  }

  if (hasTypeError(plan)) {
    return plan;
  }

  // Range/content phase.
  if (in.otPort.present && (in.otPort.value < 1 || in.otPort.value > 65535)) {
    fail(plan, "range.ot_port", "ot_port invalid");
    return plan;
  }
  if (in.imuPeriodMs.present && !isValidImuPeriodMs(in.imuPeriodMs.value)) {
    fail(plan, "range.imu_period_ms", "imu_period_ms must be 5, 10, 20, or 40");
    return plan;
  }
  if (in.wifiTx.present && !isValidWifiTxProfile(in.wifiTx.value)) {
    fail(plan, "range.wifi_tx", "wifi_tx must be 0, 1, or 2");
    return plan;
  }
  if (in.rgbBrightness.present && !inRangeInclusive(in.rgbBrightness.value, 0, 100)) {
    fail(plan, "range.rgb_brightness", "rgb_brightness must be 0-100");
    return plan;
  }
  if (in.ledMode.present && !isValidLedMode(in.ledMode.value)) {
    fail(plan, "range.led_mode", "led_mode must be 0-3 (RgbPreset)");
    return plan;
  }
  if (in.ledR.present && !inRangeInclusive(in.ledR.value, 0, 255)) {
    fail(plan, "range.led_channel", "led_r, led_g, led_b must be 0-255");
    return plan;
  }
  if (in.ledG.present && !inRangeInclusive(in.ledG.value, 0, 255)) {
    fail(plan, "range.led_channel", "led_r, led_g, led_b must be 0-255");
    return plan;
  }
  if (in.ledB.present && !inRangeInclusive(in.ledB.value, 0, 255)) {
    fail(plan, "range.led_channel", "led_r, led_g, led_b must be 0-255");
    return plan;
  }
  if (in.buzzerVolume.present && !inRangeInclusive(in.buzzerVolume.value, 0, 100)) {
    fail(plan, "range.buzzer_volume", "buzzer_volume must be 0-100");
    return plan;
  }
  if (in.batteryCapacityMah.present &&
      !inRangeInclusive(in.batteryCapacityMah.value, 100, 5000)) {
    fail(plan, "range.battery_capacity_mah", "battery_capacity_mah must be 100..5000");
    return plan;
  }
  if (in.batteryCalOffsetMv.present &&
      !inRangeInclusive(in.batteryCalOffsetMv.value, -1200, 1200)) {
    fail(plan, "range.battery_cal_offset_mv", "battery_cal_offset_mv must be -1200..1200");
    return plan;
  }

  // Cross-field/content phase.
  if (in.otAxes.present && !otAxesIsPermutation(in.otAxes.axes)) {
    fail(plan, "content.ot_axes", "ot_axes must use yaw, pitch, and roll each exactly once");
    return plan;
  }

  std::string normalizedHostname;
  if (in.hostname.present) {
    normalizedHostname = normalizeHostname(in.hostname.value.c_str());
    if (!normalizedHostname.empty() && !isHostnameCharsValid(normalizedHostname.c_str())) {
      fail(plan, "content.hostname",
           "hostname: use lowercase letters, digits, hyphen only (max 24)");
      return plan;
    }
  }

  // Accepted writes and side-effect decisions.
  if (in.ssid.present) {
    plan.writeSsid = true;
    plan.ssidValue = in.ssid.value;
    plan.wifiCredChanged = (in.ssid.value != in.prevSsid);
  }
  if (in.password.present) {
    plan.writePassword = true;
    plan.passwordValue = in.password.value;
    plan.wifiCredChanged = true;
  }
  if (in.otHost.present) {
    plan.writeOtHost = true;
    plan.otHostValue = in.otHost.value;
  }
  if (in.otPort.present) {
    plan.writeOtPort = true;
    plan.otPortValue = static_cast<uint16_t>(in.otPort.value);
  }
  if (in.udpEnabled.present) {
    plan.writeUdpEnabled = true;
    plan.udpEnabledValue = in.udpEnabled.value;
  }
  if (in.hatireUsb.present) {
    plan.writeHatireUsb = true;
    plan.hatireUsbValue = in.hatireUsb.value;
  }
  if (in.mdnsOn.present) {
    plan.writeMdnsOn = true;
    plan.mdnsOnValue = in.mdnsOn.value;
    if (in.mdnsOn.value != in.prevMdnsOn) {
      plan.rebootRequired = true;
    }
  }
  if (in.imuPeriodMs.present) {
    plan.writeImuPeriodMs = true;
    plan.imuPeriodMsValue = static_cast<uint16_t>(in.imuPeriodMs.value);
    if (in.imuPeriodMs.value != static_cast<int>(in.prevImuPeriodMs)) {
      plan.rebootRequired = true;
    }
  }
  if (in.wifiTx.present) {
    plan.writeWifiTx = true;
    plan.wifiTxValue = static_cast<uint8_t>(in.wifiTx.value);
  }
  if (in.otAxes.present) {
    plan.writeOtAxes = true;
    plan.otAxesValue = in.otAxes.axes;
  }
  if (in.rgbBrightness.present) {
    plan.writeRgbBrightness = true;
    plan.rgbBrightnessValue = static_cast<uint8_t>(in.rgbBrightness.value);
  }
  if (in.ledMode.present) {
    plan.writeLedMode = true;
    plan.ledModeValue = static_cast<uint8_t>(in.ledMode.value);
  }
  if (in.ledR.present) {
    plan.writeLedR = true;
    plan.ledRValue = static_cast<uint8_t>(in.ledR.value);
  }
  if (in.ledG.present) {
    plan.writeLedG = true;
    plan.ledGValue = static_cast<uint8_t>(in.ledG.value);
  }
  if (in.ledB.present) {
    plan.writeLedB = true;
    plan.ledBValue = static_cast<uint8_t>(in.ledB.value);
  }
  if (in.buzzerVolume.present) {
    plan.writeBuzzerVolume = true;
    plan.buzzerVolumeValue = static_cast<uint8_t>(in.buzzerVolume.value);
  }
  if (in.batteryCapacityMah.present) {
    plan.writeBatteryCapacityMah = true;
    plan.batteryCapacityMahValue = static_cast<uint16_t>(in.batteryCapacityMah.value);
  }
  if (in.batteryCalOffsetMv.present) {
    plan.writeBatteryCalOffsetMv = true;
    plan.batteryCalOffsetMvValue = static_cast<int32_t>(in.batteryCalOffsetMv.value);
  }
  if (in.batteryCalibrate4v2.present && in.batteryCalibrate4v2.value) {
    plan.batteryCalibrateRequested = true;
    plan.batteryCalibrateAllowed = true;
  }
  if (in.hostname.present) {
    plan.writeHostname = true;
    plan.hostnameValue = normalizedHostname;
    const std::string prevHost = in.prevHostname.empty() ? "azimuth" : in.prevHostname;
    const std::string nextHost = normalizedHostname.empty() ? "azimuth" : normalizedHostname;
    if (nextHost != prevHost) {
      plan.rebootRequired = true;
    }
  }

  plan.ok = true;
  return plan;
}

}  // namespace azimuth_cfg
