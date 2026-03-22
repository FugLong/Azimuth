/**
 * Seeed XIAO ESP32-C3 + BNO08x (SPI) — fused orientation.
 *
 * Wiring (BNO08x breakout: PS0 + PS1 → 3.3 V for SPI):
 *   SPI uses the board default FSPI pins (see variants/XIAO_ESP32C3/pins_arduino.h):
 *     SCK=D8 (GPIO8), MISO=D9 (GPIO9), MOSI=D10 (GPIO10)
 *   CS=D2 (GPIO4), H_INT=D3 (GPIO5), RST=D7 (GPIO20), 3V3, GND
 *
 * Modes (set at compile time):
 *   IMU_DEBUG_MODE=1 — human-readable yaw/pitch/roll on USB serial
 *   IMU_DEBUG_MODE=0 — Hatire binary frames for OpenTrack “Hatire Arduino” input (115200, DTR on)
 */

#include <Arduino.h>
#include <SPI.h>

#if !IMU_DEBUG_MODE
#include <string.h>
#endif

#include "SparkFun_BNO08x_Arduino_Library.h"

#ifndef IMU_DEBUG_MODE
#define IMU_DEBUG_MODE 1
#endif

namespace {

/** Chip select — any free GPIO; SparkFun lib drives CS in software. */
constexpr uint8_t kPinCs = 4;    // D2
constexpr uint8_t kPinInt = 5;   // D3 (H_INT, active low)
constexpr uint8_t kPinRst = 20;  // D7 (NRST)

/** Rotation vector report period passed to enableRotationVector() — milliseconds. */
constexpr uint16_t kRotationVectorPeriodMs = 10;

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

void hatireInitPacket() {
  gHat.Begin = static_cast<int16_t>(0xAAAA);
  gHat.Cpt = 0;
  memset(gHat.gyro, 0, sizeof(gHat.gyro));
  memset(gHat.acc, 0, sizeof(gHat.acc));
  gHat.End = static_cast<int16_t>(0x5555);
}

void sendHatirePacket(float yawDeg, float pitchDeg, float rollDeg) {
  // Same axis/sign convention as Nano33_PC_Head_Tracker Rev2 + Fusion NWU output
  gHat.gyro[0] = yawDeg;
  gHat.gyro[1] = -pitchDeg;
  gHat.gyro[2] = rollDeg;

  Serial.write(reinterpret_cast<const uint8_t*>(&gHat), sizeof(gHat));

  gHat.Cpt++;
  if (gHat.Cpt > 999) {
    gHat.Cpt = 0;
  }
}
#endif  // !IMU_DEBUG_MODE

void enableReports() {
  if (!imu.enableRotationVector(kRotationVectorPeriodMs)) {
#if IMU_DEBUG_MODE
    Serial.println(F("enableRotationVector failed"));
#endif
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);

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
  // Hatire: wait for USB host (OpenTrack) to open the port; DTR should be enabled in the plugin.
  while (!Serial) {
    delay(10);
  }
#endif

#if !IMU_DEBUG_MODE
  hatireInitPacket();
#endif

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
}

void loop() {
  if (imu.wasReset()) {
#if IMU_DEBUG_MODE
    Serial.println(F("# sensor reset; re-enabling reports"));
#endif
    enableReports();
  }

  if (!imu.getSensorEvent()) {
    return;
  }

  if (imu.getSensorEventID() != SENSOR_REPORTID_ROTATION_VECTOR) {
    return;
  }

  const float yawDeg = imu.getYaw() * kRadToDeg;
  const float pitchDeg = imu.getPitch() * kRadToDeg;
  const float rollDeg = imu.getRoll() * kRadToDeg;

#if IMU_DEBUG_MODE
  const uint32_t now = millis();
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
  sendHatirePacket(yawDeg, pitchDeg, rollDeg);
#endif
}
