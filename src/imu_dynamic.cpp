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
/** Smoothed magnitude of head motion (deg/s), decaying after bursts — avoids snapping to "still" on one quiet frame. */
float gActivityDegPerSec = 0.0f;
float gSmoothedPeriodMs = 20.0f;

/** Time constant for activity decay (ms); larger = longer tail at higher report rate after motion. */
constexpr float kActivityTauMs = 280.0f;
/** Map activity to period: below this feels "still" for rate purposes (deg/s). */
constexpr float kActivityStill = 1.2f;
/** Above this we sit at peak report rate (deg/s). */
constexpr float kActivityHot = 32.0f;

constexpr float kPeriodAttack = 0.42f;
constexpr float kPeriodDecay = 0.11f;

float wrapDeg180(float d) {
  while (d > 180.0f) {
    d -= 360.0f;
  }
  while (d < -180.0f) {
    d += 360.0f;
  }
  return d;
}

/** Slowest report interval we allow in dynamic mode (ms). 20 ms = 50 Hz — much smoother than 25 Hz for slow pans. */
float slowPeriodCapMs(uint16_t peakMs) {
  const uint16_t peak = std::min<uint16_t>(40, std::max<uint16_t>(5, peakMs));
  if (peak >= 40) {
    return 40.0f;
  }
  const float p = static_cast<float>(peak);
  // Headroom above peak: at least 20 ms floor, stretch toward ~35 ms for high peak so we still save power.
  return std::min(35.0f, std::max(20.0f, p + 12.0f));
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
  gSmoothedPeriodMs = 20.0f;
}

uint16_t armDynamicForPeak(uint16_t peakPeriodMs) {
  const uint16_t peak = std::min<uint16_t>(40, std::max<uint16_t>(5, peakPeriodMs));
  gHasPrev = false;
  gActivityDegPerSec = 0.0f;
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

  const float dt = static_cast<float>(dtMs);
  const float decay = std::exp(-dt / kActivityTauMs);
  gActivityDegPerSec = std::max(omegaInst, gActivityDegPerSec * decay);

  float u = 0.0f;
  if (gActivityDegPerSec >= kActivityHot) {
    u = 1.0f;
  } else if (gActivityDegPerSec <= kActivityStill) {
    u = 0.0f;
  } else {
    u = (gActivityDegPerSec - kActivityStill) / (kActivityHot - kActivityStill);
    u = std::min(1.0f, std::max(0.0f, u));
    // Ease so mid-speed motion stays closer to peak (less "mush" when slowing).
    u = u * u;
  }

  const float peakF = static_cast<float>(peak);
  const float targetMs = slowMs + (peakF - slowMs) * u;

  const float alpha = (targetMs < gSmoothedPeriodMs) ? kPeriodAttack : kPeriodDecay;
  gSmoothedPeriodMs += alpha * (targetMs - gSmoothedPeriodMs);

  return static_cast<uint16_t>(quantizePeriodMs(gSmoothedPeriodMs, peak));
}

}  // namespace azimuth_imu_dynamic
