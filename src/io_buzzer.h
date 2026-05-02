#pragma once

#include <stdint.h>

namespace azimuth_io_buzzer {

void init();
void chirp(uint16_t frequencyHz, uint16_t durationMs);
/** Short non-blocking melody for FUNC button (no-op if buzzer not present). */
void playFuncButtonTune();
void tick();

}  // namespace azimuth_io_buzzer
