#pragma once

/**
 * Map fusion Euler angles (degrees) to OpenTrack "UDP over network" / Hatire Rot[0..2].
 * Writes [yaw, roll, -pitch] so OpenTrack **Input** axes Yaw/Pitch/Roll → Rot 0/1/2 match the README.
 */
inline void opentrackMapEulerDegToRot(float yawDeg, float pitchDeg, float rollDeg, float rotOut[3]) {
  rotOut[0] = yawDeg;
  rotOut[1] = rollDeg;
  rotOut[2] = -pitchDeg;
}
