#pragma once

#include <stdint.h>

#include "opentrack_pose.h"

/** Track build: WiFi, OpenTrack UDP, NVS settings, HTTP settings UI (idle = cheap handleClient only). */
void trackNetworkLoadTrackingPrefs();
uint16_t trackNetworkImuRotationPeriodMs();
bool trackNetworkHatireUsbEnabled();
const OtAxisMapConfig& trackNetworkOtAxisMap();

void trackNetworkInit();
void trackNetworkLoop();
void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);
