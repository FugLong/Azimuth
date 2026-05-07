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
PolicyOverride gPolicy = PolicyOverride::None;
uint8_t gR = 0;
uint8_t gG = 0;
uint8_t gB = 0;
uint8_t gBrightnessPct = 25;

uint32_t gRainbowPhase = 0;
uint32_t gLastRainbowMs = 0;

uint8_t scaleCap255(uint16_t v) {
  return static_cast<uint8_t>(v > 255 ? 255 : v);
}

uint8_t trianglePulse(uint32_t t, uint32_t periodMs) {
  if (periodMs < 4) {
    return 255;
  }
  const uint32_t p = t % periodMs;
  const uint32_t half = periodMs / 2;
  if (half == 0) {
    return 255;
  }
  if (p < half) {
    return static_cast<uint8_t>((p * 255U) / half);
  }
  const uint32_t down = periodMs - p;
  const uint32_t span = periodMs - half;
  return span > 0 ? static_cast<uint8_t>((down * 255U) / span) : 0;
}

void writeRgbBallastInverted(uint8_t r, uint8_t g, uint8_t b) {
  const uint8_t pr0 = r;
  const uint8_t pg0 = scaleCap255((static_cast<uint16_t>(g) * 68U) / 100U);
  const uint8_t pb0 = scaleCap255((static_cast<uint16_t>(b) * 220U) / 100U);
  const uint8_t pr = scaleCap255((static_cast<uint16_t>(pr0) * gBrightnessPct) / 100U);
  const uint8_t pg = scaleCap255((static_cast<uint16_t>(pg0) * gBrightnessPct) / 100U);
  const uint8_t pb = scaleCap255((static_cast<uint16_t>(pb0) * gBrightnessPct) / 100U);
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

bool applyPolicyVisual() {
  if (gPolicy == PolicyOverride::None) {
    return false;
  }
  const uint32_t t = millis();
  if (!gHasRgb) {
    uint32_t period = 500;
    switch (gPolicy) {
      case PolicyOverride::ThermalHold:
        period = 600;
        break;
      case PolicyOverride::ThermalWarn:
        period = 300;
        break;
      case PolicyOverride::BatteryCritical:
        period = 120;
        break;
      case PolicyOverride::SetupAp:
        period = 400;
        break;
      case PolicyOverride::Stasis:
        period = 700;
        break;
      default:
        break;
    }
    const bool on = (t / period) % 2U == 0U;
    digitalWrite(azimuth_hw::kPinStatusLed, on ? HIGH : LOW);
    return true;
  }

  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  switch (gPolicy) {
    case PolicyOverride::ThermalHold: {
      const uint8_t v = trianglePulse(t, 1000U);
      r = v;
      g = 0;
      b = 0;
      break;
    }
    case PolicyOverride::ThermalWarn: {
      const uint8_t v = trianglePulse(t, 500U);
      r = v;
      g = static_cast<uint8_t>(v / 4U);
      b = 0;
      break;
    }
    case PolicyOverride::BatteryCritical: {
      const uint8_t v = trianglePulse(t, 260U);
      r = v;
      g = 0;
      b = static_cast<uint8_t>(v / 8U);
      break;
    }
    case PolicyOverride::SetupAp: {
      const uint8_t v = trianglePulse(t, 1400U);
      r = 0;
      g = static_cast<uint8_t>((static_cast<uint16_t>(v) * 3U) / 4U);
      b = v;
      break;
    }
    case PolicyOverride::Stasis: {
      const uint8_t v = trianglePulse(t, 1100U);
      r = 0;
      g = static_cast<uint8_t>(v / 3U);
      b = static_cast<uint8_t>((static_cast<uint16_t>(v) * 2U) / 3U);
      break;
    }
    default:
      break;
  }
  writeRgbBallastInverted(r, g, b);
  return true;
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
  gPolicy = PolicyOverride::None;
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
  if (gPolicy != PolicyOverride::None) {
    return;
  }
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
  if (applyPolicyVisual()) {
    return;
  }
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

void setBrightnessPercent(uint8_t percent) {
  if (percent > 100) {
    percent = 100;
  }
  gBrightnessPct = percent;
  if (!gHasRgb) {
    return;
  }
  writeRgbBallastInverted(gR, gG, gB);
}

void setPolicyOverride(PolicyOverride layer) {
  gPolicy = layer;
}

}  // namespace azimuth_io_led
