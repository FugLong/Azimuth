#pragma once

#include <cstdint>

namespace azimuth_imu_dynamic {

/** Clears motion estimator (e.g. fixed-rate mode). */
void resetState();

/**
 * Re-arm variable rate after boot, sensor reset, or portal save. Returns the first period (ms) to apply.
 */
uint16_t armDynamicForPeak(uint16_t peakPeriodMs);

/**
 * Returns the next rotation-vector report interval (ms) for BNO08x.
 * `peakPeriodMs` is the fastest interval allowed (user "peak" / fixed-rate choice: 5, 10, or 20).
 * When still, relaxes toward a peak-dependent slow cap (typically 20–35 ms), not a hard 40 ms.
 */
uint16_t computeNextPeriodMs(float yawDeg, float pitchDeg, float rollDeg, uint32_t nowMs,
                             uint16_t peakPeriodMs);

}  // namespace azimuth_imu_dynamic
