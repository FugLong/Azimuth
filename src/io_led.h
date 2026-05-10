#pragma once

#include <stdint.h>

namespace azimuth_io_led {

enum class RgbPreset : uint8_t {
  Rainbow = 0,
  RainbowSlow,
  Status,
  Manual,
};

/** Safety / mode overlay; base ambient only when `None` (see `io_led_policy`). */
enum class PolicyOverride : uint8_t {
  None = 0,
  ThermalHold,
  ThermalWarn,
  BatteryCritical,
  SetupAp,
  Stasis,
  Update,
};

void init();
void setRgb(uint8_t r, uint8_t g, uint8_t b);
void setStatus(bool active);
void tick();

void setRgbPreset(RgbPreset preset);
RgbPreset rgbPreset();

void setManualRgb(uint8_t r, uint8_t g, uint8_t b);
void clearManualRgb();

/** Scale RGB output 0–100% (default 25). No-op on boards without RGB. */
void setBrightnessPercent(uint8_t percent);

void setPolicyOverride(PolicyOverride layer);

}  // namespace azimuth_io_led
