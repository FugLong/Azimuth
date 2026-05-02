#pragma once

#include <stdint.h>

namespace azimuth_io_led {

enum class RgbPreset : uint8_t {
  Rainbow = 0,
  RainbowSlow,
  Status,
  Manual,
};

void init();
void setRgb(uint8_t r, uint8_t g, uint8_t b);
void setStatus(bool active);
void tick();

void setRgbPreset(RgbPreset preset);
RgbPreset rgbPreset();

void setManualRgb(uint8_t r, uint8_t g, uint8_t b);
void clearManualRgb();

}  // namespace azimuth_io_led
