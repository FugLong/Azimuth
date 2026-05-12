#pragma once

#include <stdint.h>

#include "opentrack_pose.h"

/** Wi‑Fi, OpenTrack UDP, NVS, HTTP portal — full impl. in `azimuth_main_*`; stubs in `azimuth_debug_*`. */
void trackNetworkLoadTrackingPrefs();
uint16_t trackNetworkImuRotationPeriodMs();
/** NVS: slows IMU/UDP when head is still to save battery; peak rate follows `trackNetworkImuRotationPeriodMs()`. */
bool trackNetworkImuDynamicEnabled();
/** Portal saved IMU prefs without reboot — main applies rotation-vector interval once. */
bool trackNetworkTakeImuPrefsDirty();
bool trackNetworkHatireUsbEnabled();
const OtAxisMapConfig& trackNetworkOtAxisMap();

void trackNetworkInit();
void trackNetworkLoop();
void trackNetworkPublishPoseSample(float yawDeg, float pitchDeg, float rollDeg);
void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);

/** Cuts Wi‑Fi / portal / UDP for thermal protection (latched until cool reboot). */
void trackNetworkApplyThermalEmergency();
bool trackNetworkThermalHoldActive();

/** Captive **Azimuth-Tracker** / Offline Mode AP (first-time setup or off-grid use). */
bool trackNetworkSetupApActive();
/** Preferred naming (same behavior as `trackNetworkSetupApActive`). */
bool trackNetworkOfflineApActive();

/**
 * Pause / stasis (FUNC): runtime gate for OpenTrack UDP and Hatire — does **not** change NVS `udp_enabled`.
 * When active, modem sleep is forced on for low power while STA is up.
 */
void trackNetworkSetStasis(bool active);
bool trackNetworkStasisActive();

/**
 * Trigger a wireless firmware update from the trusted release URL.
 * Returns one of `azimuth_update::beginResultString(...)` values. Safe to call
 * from IMU_DEBUG_MODE builds (returns "chip_busy").
 */
const char* trackNetworkBeginFirmwareUpdate();
