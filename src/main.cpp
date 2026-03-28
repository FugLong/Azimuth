/**
 * Seeed XIAO ESP32-C3 + BNO08x (SPI) — fused orientation.
 *
 * Azimuth PCB (ESP32_BNO086): SPI + BNO086 straps
 *   FSPI: SCK=D8 (GPIO8), MISO=D9 (GPIO9), MOSI=D10 (GPIO10)
 *   CS=D3 (GPIO5), H_INT=D4 (GPIO6), NRST=D7 (GPIO20)
 *   PS0/WAKE=D2 (GPIO4); PS1 + VDD + VDDIO → 3V3 on PCB
 *
 * Modes (set at compile time):
 *   IMU_DEBUG_MODE=1 — human-readable yaw/pitch/roll on USB serial
 *   IMU_DEBUG_MODE=0 — Hatire USB (115200) + optional WiFi UDP OpenTrack “UDP over network” (6× double)
 */

#include <Arduino.h>
#include <SPI.h>

#include "SparkFun_BNO08x_Arduino_Library.h"

#ifndef IMU_DEBUG_MODE
#define IMU_DEBUG_MODE 1
#endif

#if !IMU_DEBUG_MODE
#include <WiFi.h>
#include <WiFiUdp.h>
#include <string.h>
#include "secrets.h"
#ifndef OPENTRACK_UDP_PORT
#define OPENTRACK_UDP_PORT 4242
#endif
#endif

namespace {

/** Chip select — any free GPIO; SparkFun lib drives CS in software. */
constexpr uint8_t kPinCs = 5;    // D3
constexpr uint8_t kPinInt = 6;   // D4 (H_INT, active low)
constexpr uint8_t kPinRst = 20;  // D7 (NRST)

/** Rotation vector report period passed to enableRotationVector() — milliseconds (100 Hz). */
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

/** Tracks USB host presence so we can re-sync Hatire after OpenTrack disconnect/reconnect. */
bool gHatireUsbWasConnected = false;

void hatireInitPacket() {
  gHat.Begin = static_cast<int16_t>(0xAAAA);
  gHat.Cpt = 0;
  memset(gHat.gyro, 0, sizeof(gHat.gyro));
  memset(gHat.acc, 0, sizeof(gHat.acc));
  gHat.End = static_cast<int16_t>(0x5555);
}

void sendHatirePacket(float yawDeg, float pitchDeg, float rollDeg) {
  // ESP32 USB CDC: writing with no host (or full TX) can block forever — skip/drop instead.
  if (!Serial) {
    return;
  }
  if (Serial.availableForWrite() < static_cast<int>(sizeof(gHat))) {
    return;
  }

  // Rot[0..2] — Hatire Yaw/Pitch/Roll axis = 0 / 1 / 2 (README). Yaw from fusion; pitch/roll channels
  // swapped vs raw getPitch/getRoll so OpenTrack head axes match expected game view.
  gHat.gyro[0] = yawDeg;
  gHat.gyro[1] = rollDeg;
  gHat.gyro[2] = -pitchDeg;

  Serial.write(reinterpret_cast<const uint8_t*>(&gHat), sizeof(gHat));

  gHat.Cpt++;
  if (gHat.Cpt > 999) {
    gHat.Cpt = 0;
  }
}

/**
 * OpenTrack “UDP over network”: Tx, Ty, Tz, then Yaw, Pitch, Roll (deg), same semantics as Hatire when
 * Yaw/Pitch/Roll axis = 0 / 1 / 2 (see README). Matches sendHatirePacket Rot[] order 1:1.
 */
WiFiUDP gOpentrackUdp;
IPAddress gOpentrackIp;
bool gOpentrackUdpOk = false;

/**
 * SDK default is near max (~19 dBm). Use a lower cap for desk / same-room AP: less PA draw and heat.
 * If UDP becomes unreliable, raise in steps (e.g. WIFI_POWER_11dBm, …) toward WIFI_POWER_19_5dBm.
 */
constexpr wifi_power_t kWifiTxPower = WIFI_POWER_8_5dBm;

void initOpentrackUdp() {
  static const char kSsid[] = WIFI_SSID;
  if (kSsid[0] == '\0') {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  constexpr uint32_t kWifiTimeoutMs = 12000;
  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < kWifiTimeoutMs) {
    delay(200);
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  WiFi.setTxPower(kWifiTxPower);
  if (!gOpentrackUdp.begin(0)) {
    return;
  }
  if (!gOpentrackIp.fromString(OPENTRACK_UDP_HOST)) {
    return;
  }
  gOpentrackUdpOk = true;
  // Modem sleep between beacons — much less RF duty cycle than always-on; main heat win vs tiny UDP payload.
  // If UDP to OpenTrack gets choppy, try commenting this out.
  WiFi.setSleep(true);
}

void sendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg) {
  if (!gOpentrackUdpOk || WiFi.status() != WL_CONNECTED) {
    return;
  }

  const float r0 = yawDeg;
  const float r1 = rollDeg;
  const float r2 = -pitchDeg;
  double pose[6] = {
      0.0,
      0.0,
      0.0,
      static_cast<double>(r0),
      static_cast<double>(r1),
      static_cast<double>(r2),
  };

  if (!gOpentrackUdp.beginPacket(gOpentrackIp, OPENTRACK_UDP_PORT)) {
    return;
  }
  gOpentrackUdp.write(reinterpret_cast<const uint8_t*>(pose), sizeof(pose));
  gOpentrackUdp.endPacket();
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

#if !IMU_DEBUG_MODE
  initOpentrackUdp();
#endif
}

void loop() {
#if !IMU_DEBUG_MODE
  const bool usbConnected = static_cast<bool>(Serial);
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
    yield();  // avoid 100% CPU busy-spin between 100 Hz IMU reports (major heat / power)
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
  sendOpentrackUdp(yawDeg, pitchDeg, rollDeg);
#endif
}
