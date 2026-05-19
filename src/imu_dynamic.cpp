#include "imu_dynamic.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace azimuth_imu_dynamic {
namespace {

bool gHasPrev = false;
float gPrevYaw = 0.0f;
float gPrevPitch = 0.0f;
float gPrevRoll = 0.0f;
uint32_t gPrevMs = 0;
/** Drives the period curve (deg/s); twitches are filtered in the low band. */
float gActivityDegPerSec = 0.0f;
float gSmoothedPeriodMs = 40.0f;
/** Low-pass of sub-snap motion so involuntary spikes do not peg peak. */
float gLowBandOmega = 0.0f;
/** Last capped instantaneous rate (telemetry: raw_deg_s). */
float gLastOmegaRaw = 0.0f;

/** Raw inst. speed is capped for mapping. */
constexpr float kOmegaMapCeilDegPerSec = 1500.0f;
/** Above this (after cap), bypass low-pass into snap follower — keep high to ignore euler glitches. */
constexpr float kSnapOmegaDegPerSec = 480.0f;
/** EMA alpha for motion below snap threshold (0..1 per update). */
constexpr float kLowBandAlpha = 0.22f;

/** Time constant for activity decay when falling (ms). */
constexpr float kActivityTauMs = 200.0f;
/** Below this maps to slowest report interval (deg/s). */
constexpr float kActivityStill = 1.5f;
/** Envelope must reach this (deg/s) for full peak — was too easy at ~175 with max() envelope. */
constexpr float kActivityHot = 300.0f;
/** Max rise of envelope per report (deg/s) — kills one-frame spikes while allowing ~few frames to full hot. */
constexpr float kMaxActivityRisePerSample = 88.0f;

/** When target wants faster reports (smaller ms), snap almost immediately. */
constexpr float kPeriodAttack = 0.94f;
constexpr float kPeriodDecay = 0.2f;

/** Slowest report interval in dynamic mode (ms): 40 ms = 25 Hz. */
constexpr float kSlowPeriodCapMs = 40.0f;

float wrapDeg180(float d) {
  while (d > 180.0f) {
    d -= 360.0f;
  }
  while (d < -180.0f) {
    d += 360.0f;
  }
  return d;
}

/** Slow end for variable rate: 40 ms = 25 Hz (same for all peak settings below 40 ms). */
float slowPeriodCapMs(uint16_t peakMs) {
  const uint16_t peak = std::min<uint16_t>(40, std::max<uint16_t>(5, peakMs));
  if (peak >= 40) {
    return 40.0f;
  }
  return kSlowPeriodCapMs;
}

float quantizePeriodMs(float p, uint16_t peakMs) {
  const float lo = static_cast<float>(std::max<uint16_t>(5, peakMs));
  const float hi = slowPeriodCapMs(peakMs);
  p = std::min(hi, std::max(lo, p));
  const float q = std::round(p / 5.0f) * 5.0f;
  return std::min(hi, std::max(lo, q));
}

}  // namespace

void resetState() {
  gHasPrev = false;
  gActivityDegPerSec = 0.0f;
  gSmoothedPeriodMs = 40.0f;
  gLowBandOmega = 0.0f;
  gLastOmegaRaw = 0.0f;
}

uint16_t armDynamicForPeak(uint16_t peakPeriodMs) {
  const uint16_t peak = std::min<uint16_t>(40, std::max<uint16_t>(5, peakPeriodMs));
  gHasPrev = false;
  gActivityDegPerSec = 0.0f;
  gLowBandOmega = 0.0f;
  gLastOmegaRaw = 0.0f;
  gPrevMs = 0;
  if (peak >= 40) {
    gSmoothedPeriodMs = 40.0f;
    return 40;
  }
  const float slowMs = slowPeriodCapMs(peak);
  const float mid = (static_cast<float>(peak) + slowMs) * 0.5f;
  gSmoothedPeriodMs = quantizePeriodMs(mid, peak);
  return static_cast<uint16_t>(gSmoothedPeriodMs);
}

uint16_t computeNextPeriodMs(float yawDeg, float pitchDeg, float rollDeg, uint32_t nowMs,
                             uint16_t peakPeriodMs) {
  const uint16_t peak = std::min<uint16_t>(40, std::max<uint16_t>(5, peakPeriodMs));
  if (peak >= 40) {
    return 40;
  }

  const float slowMs = slowPeriodCapMs(peak);

  if (!gHasPrev) {
    gPrevYaw = yawDeg;
    gPrevPitch = pitchDeg;
    gPrevRoll = rollDeg;
    gPrevMs = nowMs;
    gHasPrev = true;
    gActivityDegPerSec = 0.0f;
    gLowBandOmega = 0.0f;
    gLastOmegaRaw = 0.0f;
    const float peakF = static_cast<float>(peak);
    gSmoothedPeriodMs = quantizePeriodMs((peakF + slowMs) * 0.5f, peak);
    return static_cast<uint16_t>(gSmoothedPeriodMs);
  }

  uint32_t dtMs = nowMs - gPrevMs;
  if (dtMs == 0) {
    dtMs = 1;
  }
  if (dtMs > 500) {
    gPrevYaw = yawDeg;
    gPrevPitch = pitchDeg;
    gPrevRoll = rollDeg;
    gPrevMs = nowMs;
    gActivityDegPerSec *= 0.5f;
    gLowBandOmega *= 0.5f;
    gLastOmegaRaw *= 0.5f;
    return static_cast<uint16_t>(quantizePeriodMs(gSmoothedPeriodMs, peak));
  }

  const float dy = wrapDeg180(yawDeg - gPrevYaw);
  const float dp = wrapDeg180(pitchDeg - gPrevPitch);
  const float dr = wrapDeg180(rollDeg - gPrevRoll);
  // Max axis delta per frame tracks single-axis head turns better than Euclidean norm of Euler deltas.
  const float stepDeg = std::max({std::fabs(dy), std::fabs(dp), std::fabs(dr)});
  const float omegaInst = stepDeg * (1000.0f / static_cast<float>(dtMs));

  gPrevYaw = yawDeg;
  gPrevPitch = pitchDeg;
  gPrevRoll = rollDeg;
  gPrevMs = nowMs;

  const float raw = std::min(omegaInst, kOmegaMapCeilDegPerSec);
  gLastOmegaRaw = raw;
  float instForEnv = raw;
  if (raw < kSnapOmegaDegPerSec) {
    gLowBandOmega += kLowBandAlpha * (raw - gLowBandOmega);
    instForEnv = gLowBandOmega;
  } else {
    gLowBandOmega = raw;
    instForEnv = raw;
  }

  const float dt = static_cast<float>(dtMs);
  const float decay = std::exp(-dt / kActivityTauMs);
  // Old design used max(inst, decay*old) so a single noisy frame could pin activity at full hot.
  // Slew-limited rise lets sustained motion still reach kActivityHot in a few frames (~30–80 ms).
  if (instForEnv > gActivityDegPerSec) {
    const float rise = instForEnv - gActivityDegPerSec;
    gActivityDegPerSec += std::min(rise, kMaxActivityRisePerSample);
  } else {
    gActivityDegPerSec = std::max(instForEnv, gActivityDegPerSec * decay);
  }

  float u = 0.0f;
  if (gActivityDegPerSec >= kActivityHot) {
    u = 1.0f;
  } else if (gActivityDegPerSec <= kActivityStill) {
    u = 0.0f;
  } else {
    u = (gActivityDegPerSec - kActivityStill) / (kActivityHot - kActivityStill);
    u = std::min(1.0f, std::max(0.0f, u));
    // Gentle ease in the mid band only — keeps deliberate motion from feeling mushy.
    u = u * u * (3.0f - 2.0f * u);
  }

  const float peakF = static_cast<float>(peak);
  const float targetMs = slowMs + (peakF - slowMs) * u;

  const float alpha = (targetMs < gSmoothedPeriodMs) ? kPeriodAttack : kPeriodDecay;
  gSmoothedPeriodMs += alpha * (targetMs - gSmoothedPeriodMs);

  return static_cast<uint16_t>(quantizePeriodMs(gSmoothedPeriodMs, peak));
}

void readDynamicTelemetry(float* activityDegPerSec, float* smoothedPeriodMs, float* rawOmegaDegPerSec) {
  if (activityDegPerSec) {
    *activityDegPerSec = gActivityDegPerSec;
  }
  if (smoothedPeriodMs) {
    *smoothedPeriodMs = gSmoothedPeriodMs;
  }
  if (rawOmegaDegPerSec) {
    *rawOmegaDegPerSec = gLastOmegaRaw;
  }
}

}  // namespace azimuth_imu_dynamic
