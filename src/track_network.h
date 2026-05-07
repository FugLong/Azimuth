#pragma once

#include <stdint.h>

#include "opentrack_pose.h"

/** Wi‑Fi, OpenTrack UDP, NVS, HTTP portal — full impl. in `azimuth_main_*`; stubs in `azimuth_debug_*`. */
void trackNetworkLoadTrackingPrefs();
uint16_t trackNetworkImuRotationPeriodMs();
bool trackNetworkHatireUsbEnabled();
const OtAxisMapConfig& trackNetworkOtAxisMap();

void trackNetworkInit();
void trackNetworkLoop();
void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);

/** Cuts Wi‑Fi / portal / UDP for thermal protection (latched until cool reboot). */
void trackNetworkApplyThermalEmergency();
bool trackNetworkThermalHoldActive();

/** Captive **Azimuth-Setup** / recovery AP (not normal STA tracking). */
bool trackNetworkSetupApActive();

/**
 * Pause / stasis (FUNC): runtime gate for OpenTrack UDP and Hatire — does **not** change NVS `udp_enabled`.
 * When active, modem sleep is forced on for low power while STA is up.
 */
void trackNetworkSetStasis(bool active);
bool trackNetworkStasisActive();
