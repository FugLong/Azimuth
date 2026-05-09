/**
 * Seeed XIAO ESP32-C3 + BNO08x (SPI) fused orientation.
 *
 * DIY: XIAO + BNO08x breakout (see docs/wiring.md)
 *   FSPI: SCK D8 (GPIO8), MISO D9 (GPIO9), MOSI D10 (GPIO10)
 *   CS D3 (GPIO5), INT D4 (GPIO6), NRST D7 (GPIO20)
 *   PS0 or WAKE D0 (GPIO2) + 10k to 3V3; battery sense D2 (GPIO4) if used; PS1 VDD VDDIO to 3V3
 *
 * Azimuth_Design PCB: same GPIO map as azimuth_hw.h (R14 on PS0 and IO2, divider on IO4, RGB buzzer FUNC).
 *
 * PlatformIO: azimuth_*_diy (XIAO) vs azimuth_*_pcb (custom PCB). Pins: include/azimuth_hw.h
 *   IMU_DEBUG_MODE 1 = USB serial angles only; 0 = Hatire + WiFi or portal + OpenTrack UDP.
 */

#include <Arduino.h>
#include <SPI.h>
#include <cstring>

#include "azimuth_hw.h"
#include "battery_monitor.h"
#include "io_button.h"
#include "io_buzzer.h"
#include "io_led.h"
#include "io_led_policy.h"
#include "thermal_monitor.h"
#include "SparkFun_BNO08x_Arduino_Library.h"

#ifndef IMU_DEBUG_MODE
#define IMU_DEBUG_MODE 1
#endif

#if !IMU_DEBUG_MODE
#include <string.h>
#include "opentrack_pose.h"
#include "track_network.h"
#endif

namespace {

/** Chip select — any free GPIO; SparkFun lib drives CS in software. */
constexpr uint8_t kPinCs = azimuth_hw::kPinImuCs;
constexpr uint8_t kPinInt = azimuth_hw::kPinImuInt;
constexpr uint8_t kPinRst = azimuth_hw::kPinImuRst;

#if IMU_DEBUG_MODE
/** Rotation vector report period passed to enableRotationVector() — milliseconds (100 Hz). */
constexpr uint16_t kRotationVectorPeriodMs = 10;
#endif

#if IMU_DEBUG_MODE
/** Limit how often we print in debug mode (sensor still runs at full rate). */
constexpr uint32_t kPrintIntervalMs = 100;
#endif

/** SPI clock (Hz). BNO08x SPI is limited to 3 MHz; library clamps if higher. */
constexpr uint32_t kSpiHz = 3000000;

constexpr float kRadToDeg = 180.0f / 3.14159265f;

BNO08x imu;

#if IMU_DEBUG_MODE
uint32_t gLastPrintMs = 0;
#endif

int32_t gPrevBatteryPercent = -1;
bool gBatteryWarn25Latched = false;
bool gBatteryWarn15Latched = false;
bool gBatteryWarn10Latched = false;
bool gBatteryWarn5Latched = false;
bool gBatteryWarn4Latched = false;
bool gBatteryWarn3Latched = false;
bool gBatteryWarn2Latched = false;
bool gBatteryWarn1Latched = false;
bool gBatteryEmergencyWifiCut = false;
uint32_t gBatteryEmergencyLastPulseMs = 0;

void onFuncButtonSingleTap() {
#if !IMU_DEBUG_MODE
  if (trackNetworkThermalHoldActive()) {
    return;
  }
  const auto batt = azimuth_battery::readStatus();
  if (batt.supported && !batt.stub && batt.percent >= 0 && batt.percent <= 1 &&
      strcmp(batt.chargeState, "absent") != 0) {
    return;
  }
  const bool next = !trackNetworkStasisActive();
  trackNetworkSetStasis(next);
  if (next) {
    azimuth_io_buzzer::playStasisEnterTune();
  } else {
    azimuth_io_buzzer::playStasisExitTune();
  }
#endif
}

void onFuncButtonDoubleTap() {}

#if !IMU_DEBUG_MODE
/**
 * Hatire / OpenTrack USB packet (same layout as Nano33_PC_Head_Tracker IO.h).
 * The float fields are Euler angles in degrees; acc[] was unused there (zeros).
 */
struct Hat {
  int16_t Begin;
  uint16_t Cpt;
  float gyro[3];
  float acc[3];
  int16_t End;
} __attribute__((packed));

static_assert(sizeof(Hat) == 30, "Hatire struct must be 30 bytes for OpenTrack compatibility");

Hat gHat;

/** Tracks USB host presence so we can re-sync Hatire after OpenTrack disconnect/reconnect. */
bool gHatireUsbWasConnected = false;

void hatireInitPacket() {
  gHat.Begin = static_cast<int16_t>(0xAAAA);
  gHat.Cpt = 0;
  memset(gHat.gyro, 0, sizeof(gHat.gyro));
  memset(gHat.acc, 0, sizeof(gHat.acc));
  gHat.End = static_cast<int16_t>(0x5555);
}

bool hatireUsbHostActive() {
  if (!Serial) {
    return false;
  }
  // Keep this portable across HWCDC variants (not all expose DTR state).
  if (Serial.availableForWrite() < static_cast<int>(sizeof(gHat))) {
    return false;
  }
  return true;
}

bool sendHatirePacket(float yawDeg, float pitchDeg, float rollDeg) {
  // ESP32 USB CDC: writing with no host (or full TX) can block forever — skip/drop instead.
  if (!hatireUsbHostActive()) {
    return false;
  }

  opentrackMapEulerToRot(yawDeg, pitchDeg, rollDeg, trackNetworkOtAxisMap(), gHat.gyro);

  Serial.write(reinterpret_cast<const uint8_t*>(&gHat), sizeof(gHat));

  gHat.Cpt++;
  if (gHat.Cpt > 999) {
    gHat.Cpt = 0;
  }
  return true;
}

#endif  // !IMU_DEBUG_MODE

void enableReports() {
#if IMU_DEBUG_MODE
  const uint16_t periodMs = kRotationVectorPeriodMs;
#else
  const uint16_t periodMs = trackNetworkImuRotationPeriodMs();
#endif
  if (!imu.enableRotationVector(periodMs)) {
#if IMU_DEBUG_MODE
    Serial.println(F("enableRotationVector failed"));
#endif
  }
}

void maybePlayBatteryThresholdAlert(bool& latched, int32_t prevPct, int32_t nowPct,
                                    int32_t thresholdPct, bool severe) {
  if (!latched && prevPct > thresholdPct && nowPct <= thresholdPct) {
    if (severe) {
      azimuth_io_buzzer::playBatteryPanicPulse();
    } else {
      azimuth_io_buzzer::playBatteryLowTune();
    }
    latched = true;
    return;
  }
  // Rearm with slight hysteresis to prevent chatter.
  if (latched && nowPct > (thresholdPct + 1)) {
    latched = false;
  }
}

void tickBatteryAlerts(uint32_t nowMs) {
  const auto batt = azimuth_battery::readStatus();
  if (!batt.supported || batt.stub || batt.percent < 0 ||
      strcmp(batt.chargeState, "absent") == 0) {
    gPrevBatteryPercent = -1;
    gBatteryWarn25Latched = false;
    gBatteryWarn15Latched = false;
    gBatteryWarn10Latched = false;
    gBatteryWarn5Latched = false;
    gBatteryWarn4Latched = false;
    gBatteryWarn3Latched = false;
    gBatteryWarn2Latched = false;
    gBatteryWarn1Latched = false;
    gBatteryEmergencyWifiCut = false;
    gBatteryEmergencyLastPulseMs = 0;
    return;
  }

  const int32_t pct = batt.percent;
  int32_t prevPct = gPrevBatteryPercent;
  if (prevPct < 0) {
    prevPct = pct + 1;
  }

  maybePlayBatteryThresholdAlert(gBatteryWarn25Latched, prevPct, pct, 25, false);
  maybePlayBatteryThresholdAlert(gBatteryWarn15Latched, prevPct, pct, 15, false);
  if (!gBatteryWarn10Latched && prevPct > 10 && pct <= 10) {
    azimuth_io_buzzer::playBatteryCriticalTune();
    gBatteryWarn10Latched = true;
  } else if (gBatteryWarn10Latched && pct > 11) {
    gBatteryWarn10Latched = false;
  }
  maybePlayBatteryThresholdAlert(gBatteryWarn5Latched, prevPct, pct, 5, true);
  maybePlayBatteryThresholdAlert(gBatteryWarn4Latched, prevPct, pct, 4, true);
  maybePlayBatteryThresholdAlert(gBatteryWarn3Latched, prevPct, pct, 3, true);
  maybePlayBatteryThresholdAlert(gBatteryWarn2Latched, prevPct, pct, 2, true);
  maybePlayBatteryThresholdAlert(gBatteryWarn1Latched, prevPct, pct, 1, true);

  if (pct <= 1) {
    if (nowMs - gBatteryEmergencyLastPulseMs >= 550) {
      azimuth_io_buzzer::playBatteryPanicPulse();
      gBatteryEmergencyLastPulseMs = nowMs;
    }
#if !IMU_DEBUG_MODE
    if (!gBatteryEmergencyWifiCut) {
      trackNetworkApplyThermalEmergency();
      gBatteryEmergencyWifiCut = true;
    }
#endif
  } else {
    gBatteryEmergencyLastPulseMs = 0;
  }

  gPrevBatteryPercent = pct;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  azimuth_io_led::init();
  azimuth_io_buzzer::init();
  azimuth_io_button::init();
  azimuth_io_button::setTapCallbacks(onFuncButtonSingleTap, onFuncButtonDoubleTap);
  azimuth_battery::init();
  azimuth_thermal::init();
  azimuth_io_led::setStatus(true);

#if IMU_DEBUG_MODE
  const uint32_t usbWaitMs = 4000;
  const uint32_t t0 = millis();
  while (!Serial && (millis() - t0 < usbWaitMs)) {
    delay(10);
  }

  Serial.println();
  Serial.println(F("BNO08x (SPI) — DEBUG: yaw / pitch / roll (deg)"));
  Serial.println(F("Init..."));
#else
  hatireInitPacket();
  trackNetworkLoadTrackingPrefs();
#endif

  // SparkFun BNO08x beginSPI() calls SPI.begin() with no pins — board defaults apply.
  // XIAO variant maps SPI to GPIO 8/9/10; esp32-c3-devkitc-02 uses variant esp32c3 (4/5/6).
  // Azimuth_Design wires FSPI to 8/9/10, so always bind the bus explicitly.
  SPI.begin(azimuth_hw::kPinSpiSck, azimuth_hw::kPinSpiMiso, azimuth_hw::kPinSpiMosi, -1);

  if (!imu.beginSPI(kPinCs, kPinInt, kPinRst, kSpiHz, SPI)) {
#if IMU_DEBUG_MODE
    Serial.println(F("ERROR: BNO08x not detected. Check SPI wiring, PS0/PS1=3V3, INT, RST."));
#endif
    while (true) {
      delay(500);
    }
  }

#if IMU_DEBUG_MODE
  Serial.println(F("BNO08x OK"));
#endif

  enableReports();
  delay(150);

#if !IMU_DEBUG_MODE
  trackNetworkInit();
#endif
  azimuth_io_buzzer::playFuncButtonTune();
}

void loop() {
  const uint32_t nowMs = millis();
  azimuth_battery::tick(nowMs);
  tickBatteryAlerts(nowMs);
  azimuth_thermal::tick(nowMs);
  azimuth_io_button::tick();
  azimuth_io_buzzer::tick();
#if !IMU_DEBUG_MODE
  const bool usbConnected = hatireUsbHostActive();
  if (usbConnected && !gHatireUsbWasConnected) {
    hatireInitPacket();
  }
  gHatireUsbWasConnected = usbConnected;
#endif

  if (imu.wasReset()) {
#if IMU_DEBUG_MODE
    Serial.println(F("# sensor reset; re-enabling reports"));
#endif
    enableReports();
  }

  if (!imu.getSensorEvent()) {
    azimuth_io_led_policy::tick(nowMs, false);
#if !IMU_DEBUG_MODE
    trackNetworkLoop();
#endif
    yield();  // avoid 100% CPU busy-spin between IMU reports (major heat / power)
    return;
  }

  if (imu.getSensorEventID() != SENSOR_REPORTID_ROTATION_VECTOR) {
    azimuth_io_led_policy::tick(nowMs, false);
#if !IMU_DEBUG_MODE
    trackNetworkLoop();
#endif
    yield();
    return;
  }

  const float yawDeg = imu.getYaw() * kRadToDeg;
  const float pitchDeg = imu.getPitch() * kRadToDeg;
  const float rollDeg = imu.getRoll() * kRadToDeg;

#if IMU_DEBUG_MODE
  const uint32_t now = millis();
  azimuth_io_led_policy::tick(nowMs, true);
  if (now - gLastPrintMs < kPrintIntervalMs) {
    return;
  }
  gLastPrintMs = now;

  Serial.print(F("Yaw "));
  Serial.print(yawDeg, 1);
  Serial.print(F("°   Pitch "));
  Serial.print(pitchDeg, 1);
  Serial.print(F("°   Roll "));
  Serial.print(rollDeg, 1);
  Serial.println(F("°"));
#else
  // Pose path before `trackNetworkLoop()` so USB/UDP leave the device with minimal delay
  // after a rotation-vector report (network work is internally time-sliced).
  trackNetworkPublishPoseSample(yawDeg, pitchDeg, rollDeg);
  bool hatireSentThisFrame = false;
  if (trackNetworkHatireUsbEnabled() && !trackNetworkStasisActive()) {
    hatireSentThisFrame = sendHatirePacket(yawDeg, pitchDeg, rollDeg);
  }
  if (!hatireSentThisFrame) {
    trackNetworkSendOpentrackUdp(yawDeg, pitchDeg, rollDeg);
  }
  trackNetworkLoop();
  azimuth_io_led_policy::tick(nowMs, true);
#endif
}
