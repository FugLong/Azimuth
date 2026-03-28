#pragma once

#include <stdint.h>

#include "opentrack_pose.h"

/** Wi‑Fi, OpenTrack UDP, NVS, HTTP portal — full impl. in `azimuth_main`; stubs in `azimuth_debug`. */
void trackNetworkLoadTrackingPrefs();
uint16_t trackNetworkImuRotationPeriodMs();
bool trackNetworkHatireUsbEnabled();
const OtAxisMapConfig& trackNetworkOtAxisMap();

void trackNetworkInit();
void trackNetworkLoop();
void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);
