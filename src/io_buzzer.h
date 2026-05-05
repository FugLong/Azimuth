#pragma once

#include <stdint.h>

namespace azimuth_io_buzzer {

void init();
void chirp(uint16_t frequencyHz, uint16_t durationMs);
/** Short non-blocking melody for FUNC button (no-op if buzzer not present). */
void playFuncButtonTune();
/** Distinct “thermal warning” motif (~80 °C edge). */
void playThermalWarnTune();
/** Distinct “thermal emergency” motif (~90 °C); system may cut Wi‑Fi after this. */
void playThermalCriticalTune();
void tick();

/** 0 = mute; 1–100 is UI level (firmware applies a curve to PWM duty). */
void setVolumePercent(uint8_t percent);

}  // namespace azimuth_io_buzzer
