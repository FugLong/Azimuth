#include "io_led.h"

#include <Arduino.h>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_io_led {
namespace {
bool gHasRgb = false;
bool gRgbDemo = false;
bool gStatusOn = false;
uint8_t gR = 0;
uint8_t gG = 0;
uint8_t gB = 0;
uint8_t gHue = 0;
uint32_t gLastHueMs = 0;

/** Common-anode RGB: cathode pins — higher logical brightness => more time LOW (sink). */
void writeRgbInverted(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(azimuth_hw::kPinRgbR, 255 - r);
  analogWrite(azimuth_hw::kPinRgbG, 255 - g);
  analogWrite(azimuth_hw::kPinRgbB, 255 - b);
}

void wheel(uint8_t pos, uint8_t& r, uint8_t& g, uint8_t& b) {
  pos = static_cast<uint8_t>(255 - pos);
  if (pos < 85) {
    r = static_cast<uint8_t>(255 - pos * 3);
    g = 0;
    b = static_cast<uint8_t>(pos * 3);
  } else if (pos < 170) {
    pos = static_cast<uint8_t>(pos - 85);
    r = 0;
    g = static_cast<uint8_t>(pos * 3);
    b = static_cast<uint8_t>(255 - pos * 3);
  } else {
    pos = static_cast<uint8_t>(pos - 170);
    r = static_cast<uint8_t>(pos * 3);
    g = static_cast<uint8_t>(255 - pos * 3);
    b = 0;
  }
}
}  // namespace

void init() {
  const auto caps = azimuth_board::capabilities();
  gHasRgb = caps.hasRgb;
  gRgbDemo = gHasRgb;
  gStatusOn = false;

  if (gHasRgb) {
    pinMode(azimuth_hw::kPinRgbR, OUTPUT);
    pinMode(azimuth_hw::kPinRgbG, OUTPUT);
    pinMode(azimuth_hw::kPinRgbB, OUTPUT);
    setRgb(0, 0, 0);
  } else {
    pinMode(azimuth_hw::kPinStatusLed, OUTPUT);
    digitalWrite(azimuth_hw::kPinStatusLed, LOW);
  }
}

void setRgb(uint8_t r, uint8_t g, uint8_t b) {
  gR = r;
  gG = g;
  gB = b;
  if (!gHasRgb) {
    return;
  }
  writeRgbInverted(gR, gG, gB);
}

void setStatus(bool active) {
  gStatusOn = active;
  if (gHasRgb && gRgbDemo) {
    return;
  }
  if (gHasRgb) {
    setRgb(active ? 0 : 8, active ? 24 : 8, active ? 0 : 8);
  } else {
    digitalWrite(azimuth_hw::kPinStatusLed, active ? HIGH : LOW);
  }
}

void tick() {
  if (!gHasRgb || !gRgbDemo) {
    return;
  }
  const uint32_t now = millis();
  if (now - gLastHueMs < 35) {
    return;
  }
  gLastHueMs = now;
  gHue++;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  wheel(gHue, r, g, b);
  setRgb(r, g, b);
}

}  // namespace azimuth_io_led
