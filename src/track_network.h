#pragma once

#include <stdint.h>

#include "opentrack_pose.h"
#include "power_policy.h"

/** Wi‑Fi, OpenTrack UDP, NVS, HTTP portal — full impl. in `azimuth_main_*`; stubs in `azimuth_debug_*`. */
void trackNetworkLoadTrackingPrefs();
uint16_t trackNetworkImuRotationPeriodMs();
bool trackNetworkHatireUsbEnabled();
const OtAxisMapConfig& trackNetworkOtAxisMap();

void trackNetworkInit();
void trackNetworkLoop();
void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);

/** Active power profile (thermal sampling interval, etc.). */
azimuth_power::PowerProfile trackNetworkPowerProfile();
/** Cuts Wi‑Fi / portal / UDP for thermal protection (latched until cool reboot). */
void trackNetworkApplyThermalEmergency();
bool trackNetworkThermalHoldActive();
