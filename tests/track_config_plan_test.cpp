#include <array>
#include <cassert>
#include <string>
#include <vector>

#include "../src/track_config_plan.h"

using azimuth_cfg::ConfigApplyPlan;
using azimuth_cfg::ConfigPlanInput;
using azimuth_cfg::OtAxesIssue;

namespace {

ConfigPlanInput makeBaseInput() {
  ConfigPlanInput in;
  in.offlineApMode = false;
  in.prevImuPeriodMs = 10;
  in.prevMdnsOn = true;
  in.prevHostname = "azimuth";
  in.prevSsid = "home-net";
  return in;
}

struct PlanCase {
  const char* name;
  ConfigPlanInput input;
  bool expectOk;
  const char* expectErr;
  bool expectRestart;
  bool expectWifiChanged;
  bool expectBatteryCalReq;
};

void assertPlan(const PlanCase& tc) {
  const ConfigApplyPlan plan = azimuth_cfg::buildConfigApplyPlan(tc.input);
  assert(plan.ok == tc.expectOk);
  if (!tc.expectOk) {
    assert(plan.errorCode != nullptr);
    assert(std::string(plan.errorCode) == tc.expectErr);
    return;
  }
  assert(plan.errorCode == nullptr);
  assert(plan.rebootRequired == tc.expectRestart);
  assert(plan.wifiCredChanged == tc.expectWifiChanged);
  assert(plan.batteryCalibrateRequested == tc.expectBatteryCalReq);
}

}  // namespace

int main() {
  std::vector<PlanCase> cases;

  {
    auto in = makeBaseInput();
    cases.push_back({"valid noop", in, true, "", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.imuPeriodMs.present = true;
    in.imuPeriodMs.typeOk = false;
    cases.push_back({"type precedence before range", in, false, "type.imu_period_ms", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.imuPeriodMs.present = true;
    in.imuPeriodMs.value = 15;
    cases.push_back({"imu invalid range", in, false, "range.imu_period_ms", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.wifiTx.present = true;
    in.wifiTx.value = 3;
    cases.push_back({"wifi tx invalid", in, false, "range.wifi_tx", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.ledMode.present = true;
    in.ledMode.value = 9;
    cases.push_back({"led mode invalid", in, false, "range.led_mode", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.ledR.present = true;
    in.ledR.typeOk = false;
    cases.push_back({"led channel type", in, false, "type.led_channel", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.ledB.present = true;
    in.ledB.value = 300;
    cases.push_back({"led channel range", in, false, "range.led_channel", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.hostname.present = true;
    in.hostname.value = "  AZI-Host  ";
    cases.push_back({"hostname normalized and reboot", in, true, "", true, false, false});
  }
  {
    auto in = makeBaseInput();
    in.hostname.present = true;
    in.hostname.value = "Bad_Host";
    cases.push_back({"hostname invalid chars", in, false, "content.hostname", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.ssid.present = true;
    in.ssid.value = "home-net";
    in.password.present = true;
    in.password.value = "newpass";
    cases.push_back({"password change sets wifi changed", in, true, "", false, true, false});
  }
  {
    auto in = makeBaseInput();
    in.password.present = true;
    in.password.value = "";
    cases.push_back({"explicit empty password is accepted", in, true, "", false, true, false});
  }
  {
    auto in = makeBaseInput();
    in.mdnsOn.present = true;
    in.mdnsOn.value = false;
    cases.push_back({"mdns toggle reboot", in, true, "", true, false, false});
  }
  {
    auto in = makeBaseInput();
    in.imuPeriodMs.present = true;
    in.imuPeriodMs.value = 20;
    cases.push_back({"imu period change reboot", in, true, "", true, false, false});
  }
  {
    auto in = makeBaseInput();
    in.otAxes.present = true;
    in.otAxes.issue = OtAxesIssue::kNotArray;
    cases.push_back({"ot axes wrong type", in, false, "type.ot_axes", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.otAxes.present = true;
    in.otAxes.issue = OtAxesIssue::kWrongSize;
    cases.push_back({"ot axes wrong size", in, false, "shape.ot_axes.size", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.otAxes.present = true;
    in.otAxes.axes = {{{0, false}, {0, false}, {2, false}}};
    cases.push_back({"ot axes non permutation", in, false, "content.ot_axes", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.batteryCapacityMah.present = true;
    in.batteryCapacityMah.value = 50;
    cases.push_back({"battery capacity range", in, false, "range.battery_capacity_mah", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.batteryCalOffsetMv.present = true;
    in.batteryCalOffsetMv.value = 1500;
    cases.push_back({"battery offset range", in, false, "range.battery_cal_offset_mv", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.batteryCalibrate4v2.present = true;
    in.batteryCalibrate4v2.value = true;
    cases.push_back({"battery calibrate requested", in, true, "", false, false, true});
  }
  {
    auto in = makeBaseInput();
    in.offlineApMode = true;
    in.prevSsid.clear();
    in.ssid.present = true;
    in.ssid.value.clear();
    cases.push_back({"setup ap requires non-empty ssid", in, false, "constraint.setup_ap.ssid", false, false, false});
  }
  {
    auto in = makeBaseInput();
    in.otPort.present = true;
    in.otPort.typeOk = false;
    in.imuPeriodMs.present = true;
    in.imuPeriodMs.value = 15;
    cases.push_back({"multiple invalid deterministic first error", in, false, "type.ot_port", false, false, false});
  }

  for (const auto& tc : cases) {
    assertPlan(tc);
  }

  // Spot-check accepted values in a valid plan.
  {
    auto in = makeBaseInput();
    in.otPort.present = true;
    in.otPort.value = 4242;
    in.ledG.present = true;
    in.ledG.value = 111;
    in.otAxes.present = true;
    in.otAxes.axes = {{{2, false}, {0, true}, {1, false}}};
    const ConfigApplyPlan plan = azimuth_cfg::buildConfigApplyPlan(in);
    assert(plan.ok);
    assert(plan.writeOtPort && plan.otPortValue == 4242);
    assert(plan.writeLedG && plan.ledGValue == 111);
    assert(plan.writeOtAxes);
    assert(plan.otAxesValue[0].src == 2);
    assert(plan.otAxesValue[1].inv);
  }
  {
    auto in = makeBaseInput();
    in.ssid.present = true;
    in.ssid.value = "";
    in.password.present = true;
    in.password.value = "";
    const ConfigApplyPlan plan = azimuth_cfg::buildConfigApplyPlan(in);
    assert(plan.ok);
    assert(plan.writeSsid && plan.ssidValue.empty());
    assert(plan.writePassword && plan.passwordValue.empty());
    assert(plan.wifiCredChanged);
  }

  return 0;
}
