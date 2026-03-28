#pragma once

#include <stdint.h>

#ifdef __cplusplus

/** Fusion source: Rot slot takes yaw (0), pitch (1), or roll (2), then optional invert. */
struct OtAxisMapConfig {
  uint8_t srcForRot[3];
  bool invertRot[3];
};

/** README default: Rot0=yaw, Rot1=roll, Rot2=-pitch (OpenTrack Yaw/Pitch/Roll → 0/1/2). */
inline void otAxisMapSetDefault(OtAxisMapConfig* c) {
  c->srcForRot[0] = 0;
  c->invertRot[0] = false;
  c->srcForRot[1] = 2;
  c->invertRot[1] = false;
  c->srcForRot[2] = 1;
  c->invertRot[2] = true;
}

inline bool otAxisMapValid(const OtAxisMapConfig& c) {
  bool used[3] = {false, false, false};
  for (int i = 0; i < 3; ++i) {
    const uint8_t s = c.srcForRot[i];
    if (s > 2) {
      return false;
    }
    if (used[s]) {
      return false;
    }
    used[s] = true;
  }
  return true;
}

/**
 * Map fusion Euler angles (degrees) to OpenTrack "UDP over network" / Hatire gyro[0..2]
 * using per-slot source + invert.
 */
inline void opentrackMapEulerToRot(
    float yawDeg, float pitchDeg, float rollDeg, const OtAxisMapConfig& cfg, float rotOut[3]) {
  const float src[3] = {yawDeg, pitchDeg, rollDeg};
  for (int i = 0; i < 3; ++i) {
    uint8_t si = cfg.srcForRot[i];
    if (si > 2) {
      si = 0;
    }
    float v = src[si];
    if (cfg.invertRot[i]) {
      v = -v;
    }
    rotOut[i] = v;
  }
}

#endif
