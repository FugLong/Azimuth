#include "io_led.h"

#include <Arduino.h>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_io_led {

void setRgb(uint8_t r, uint8_t g, uint8_t b);

namespace {

bool gHasRgb = false;
RgbPreset gPreset = RgbPreset::Status;
bool gStatusOn = false;
uint8_t gR = 0;
uint8_t gG = 0;
uint8_t gB = 0;

uint32_t gRainbowPhase = 0;
uint32_t gLastRainbowMs = 0;

/** Cumulative weights for inverse-CDF hue mapping (index 0 unused; prefix[i] = sum w[0..i-1]). */
uint32_t gHueWeightPrefix[257];

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

/**
 * Per-hue dwell weights for the rainbow: uniform advance in phase spends equal *time* in each
 * weight bucket. Extra weight on red-primary paths offsets PCB current imbalance (R7 100 Ω red vs
 * R8 68 Ω green, R6 220 Ω blue) so red / orange / yellow are not rushed compared to cyan / green.
 */
uint32_t hueDwellWeight(uint8_t r, uint8_t g, uint8_t b) {
  constexpr uint32_t kBase = 1000;
  uint32_t w = kBase;
  if (r >= g && r >= b && r > 40) {
    w += 520;
  }
  if (r > 95 && g > 95 && b < 110) {
    w += 340;
  }
  if (r > 85 && b > 85 && g < 110) {
    w += 280;
  }
  return w;
}

void buildHueTimingTable() {
  gHueWeightPrefix[0] = 0;
  for (int h = 0; h < 256; h++) {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    wheel(static_cast<uint8_t>(h), r, g, b);
    gHueWeightPrefix[h + 1] = gHueWeightPrefix[h] + hueDwellWeight(r, g, b);
  }
}

uint8_t phaseToWarpedHue(uint32_t phase) {
  const uint32_t sum = gHueWeightPrefix[256];
  if (sum == 0) {
    return static_cast<uint8_t>(phase >> 8);
  }
  const uint64_t target = (static_cast<uint64_t>(phase) * static_cast<uint64_t>(sum)) >> 16;
  uint8_t lo = 0;
  uint8_t hi = 255;
  while (lo < hi) {
    const uint8_t mid = static_cast<uint8_t>((lo + hi + 1) >> 1);
    if (gHueWeightPrefix[mid] <= target) {
      lo = mid;
    } else {
      hi = static_cast<uint8_t>(mid - 1);
    }
  }
  return lo;
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
  gStatusOn = false;
  gRainbowPhase = 0;
  gLastRainbowMs = 0;

  buildHueTimingTable();

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
  if (!gHasRgb) {
    digitalWrite(azimuth_hw::kPinStatusLed, active ? HIGH : LOW);
    return;
  }
  if (rainbowActive()) {
    return;
  }
  if (gPreset == RgbPreset::Manual) {
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
  gPreset = RgbPreset::Manual;
  setRgb(r, g, b);
}

}  // namespace azimuth_io_led
