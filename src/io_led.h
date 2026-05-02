#pragma once

#include <stdint.h>

namespace azimuth_io_led {

void init();
void setRgb(uint8_t r, uint8_t g, uint8_t b);
void setStatus(bool active);
void tick();

}  // namespace azimuth_io_led
