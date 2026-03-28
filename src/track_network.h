#pragma once

#include <stdint.h>

/** Track build: WiFi, OpenTrack UDP, NVS settings, HTTP settings UI (idle = cheap handleClient only). */
void trackNetworkLoadTrackingPrefs();
uint16_t trackNetworkImuRotationPeriodMs();
bool trackNetworkHatireUsbEnabled();

void trackNetworkInit();
void trackNetworkLoop();
void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);
