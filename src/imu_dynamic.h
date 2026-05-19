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
 * When still, relaxes toward 40 ms (25 Hz) for variable rate when peak is below 40 ms.
 * Motion drive is capped (see `kOmegaMapCeilDegPerSec` in .cpp) and twitch-filtered below a snap threshold.
 */
uint16_t computeNextPeriodMs(float yawDeg, float pitchDeg, float rollDeg, uint32_t nowMs,
                             uint16_t peakPeriodMs);

/**
 * After `computeNextPeriodMs`, read internal controller state for `/api/status` telemetry.
 * `rawOmegaDegPerSec` is capped instantaneous max-axis rate (can spike one frame); `activity` is the
 * slew-shaped envelope that actually drives the report-rate curve.
 */
void readDynamicTelemetry(float* activityDegPerSec, float* smoothedPeriodMs,
                          float* rawOmegaDegPerSec);

}  // namespace azimuth_imu_dynamic
