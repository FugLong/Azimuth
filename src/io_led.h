#pragma once

#include <stdint.h>

namespace azimuth_io_led {

/** PCB RGB LED behavior. Rainbow presets use hue timing corrected for R6–R8 (see docs/parts-list.md). */
enum class RgbPreset : uint8_t {
  Rainbow = 0,     // default PCB bring-up: smooth rainbow, equal *perceived* dwell per hue region
  RainbowSlow,     // same mapping, ~2× cycle time
  Status,          // follow setStatus() — tracking / idle tint (no rainbow)
  Manual,          // hold last RGB from setManualRgb(); setStatus does not override
};

void init();
void setRgb(uint8_t r, uint8_t g, uint8_t b);
void setStatus(bool active);
void tick();

void setRgbPreset(RgbPreset preset);
RgbPreset rgbPreset();

/** Switches to Manual and sets color until setRgbPreset selects another mode. */
void setManualRgb(uint8_t r, uint8_t g, uint8_t b);

}  // namespace azimuth_io_led
