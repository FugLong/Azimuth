#pragma once

#include <stdint.h>

namespace azimuth_io_led_policy {

/** One owner per loop for ambient vs safety overlays (`docs/io-led-buzzer-plan.md`). */
void tick(uint32_t nowMs, bool imuPoseStreaming);

}  // namespace azimuth_io_led_policy
