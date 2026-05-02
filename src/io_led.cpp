#include "io_led.h"

#include <Arduino.h>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_io_led {

void setRgb(uint8_t r, uint8_t g, uint8_t b);

namespace {

bool gHasRgb = false;
RgbPreset gPreset = RgbPreset::Status;
bool gManualHold = false;
bool gStatusOn = false;
uint8_t gR = 0;
uint8_t gG = 0;
uint8_t gB = 0;

uint32_t gRainbowPhase = 0;
uint32_t gLastRainbowMs = 0;

uint8_t scaleCap255(uint16_t v) {
  return static_cast<uint8_t>(v > 255 ? 255 : v);
}

void writeRgbBallastInverted(uint8_t r, uint8_t g, uint8_t b) {
  const uint8_t pr = r;
  const uint8_t pg = scaleCap255((static_cast<uint16_t>(g) * 68U) / 100U);
  const uint8_t pb = scaleCap255((static_cast<uint16_t>(b) * 220U) / 100U);
  analogWrite(azimuth_hw::kPinRgbR, 255 - pr);
  analogWrite(azimuth_hw::kPinRgbG, 255 - pg);
  analogWrite(azimuth_hw::kPinRgbB, 255 - pb);
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

/** Phase → hue: 43% warm (0–84), 30% green/cyan (85–169), rest blue/magenta (170–255). */
uint8_t phaseToWarpedHue(uint32_t phase) {
  constexpr uint32_t kMod = 65536U;
  constexpr unsigned kPctWarm = 43U;
  constexpr unsigned kPctCyan = 30U;
  constexpr uint32_t kWarmEnd = (kMod * kPctWarm) / 100U;
  constexpr uint32_t kCyanEnd = kWarmEnd + (kMod * kPctCyan) / 100U;

  const uint32_t p = phase & 0xFFFFU;

  if (p < kWarmEnd) {
    const uint32_t span = kWarmEnd > 1U ? kWarmEnd - 1U : 1U;
    return static_cast<uint8_t>((static_cast<uint64_t>(p) * 84U) / span);
  }
  if (p < kCyanEnd) {
    const uint32_t x = p - kWarmEnd;
    const uint32_t span = kCyanEnd - kWarmEnd;
    const uint32_t denom = span > 1U ? span - 1U : 1U;
    return static_cast<uint8_t>(85U + (static_cast<uint64_t>(x) * 84U) / denom);
  }
  const uint32_t x = p - kCyanEnd;
  const uint32_t span = kMod - kCyanEnd;
  const uint32_t denom = span > 1U ? span - 1U : 1U;
  return static_cast<uint8_t>(170U + (static_cast<uint64_t>(x) * 85U) / denom);
}

void applyRainbow(bool slow) {
  const uint32_t now = millis();
  if (now - gLastRainbowMs < 35) {
    return;
  }
  gLastRainbowMs = now;
  const uint32_t inc = slow ? 128U : 256U;
  gRainbowPhase = (gRainbowPhase + inc) & 0xFFFFU;
  const uint8_t hue = phaseToWarpedHue(gRainbowPhase);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  wheel(hue, r, g, b);
  setRgb(r, g, b);
}

bool rainbowActive() {
  return gPreset == RgbPreset::Rainbow || gPreset == RgbPreset::RainbowSlow;
}

}  // namespace

void init() {
  const auto caps = azimuth_board::capabilities();
  gHasRgb = caps.hasRgb;
  gPreset = gHasRgb ? RgbPreset::Rainbow : RgbPreset::Status;
  gManualHold = false;
  gStatusOn = false;
  gRainbowPhase = 0;
  gLastRainbowMs = 0;

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
  writeRgbBallastInverted(gR, gG, gB);
}

void setStatus(bool active) {
  gStatusOn = active;
  if (!gHasRgb) {
    digitalWrite(azimuth_hw::kPinStatusLed, active ? HIGH : LOW);
    return;
  }
  if (rainbowActive()) {
    return;
  }
  if (gManualHold) {
    return;
  }
  setRgb(active ? 0 : 8, active ? 24 : 8, active ? 0 : 8);
}

void tick() {
  if (!gHasRgb) {
    return;
  }
  if (gPreset == RgbPreset::Rainbow) {
    applyRainbow(false);
    return;
  }
  if (gPreset == RgbPreset::RainbowSlow) {
    applyRainbow(true);
    return;
  }
}

void setRgbPreset(RgbPreset preset) {
  gPreset = preset;
  if (preset != RgbPreset::Manual) {
    gManualHold = false;
  }
  if (!gHasRgb) {
    return;
  }
  if (rainbowActive()) {
    gRainbowPhase = 0;
    gLastRainbowMs = 0;
    return;
  }
  if (gPreset == RgbPreset::Manual) {
    setRgb(gR, gG, gB);
    return;
  }
  setStatus(gStatusOn);
}

RgbPreset rgbPreset() { return gPreset; }

void setManualRgb(uint8_t r, uint8_t g, uint8_t b) {
  gManualHold = true;
  gPreset = RgbPreset::Manual;
  setRgb(r, g, b);
}

void clearManualRgb() {
  gManualHold = false;
  setRgbPreset(RgbPreset::Status);
}

}  // namespace azimuth_io_led
