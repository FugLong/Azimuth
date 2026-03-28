#pragma once

/** Track build: WiFi, OpenTrack UDP, NVS settings, HTTP settings UI (idle = cheap handleClient only). */
void trackNetworkInit();
void trackNetworkLoop();
void trackNetworkSendOpentrackUdp(float yawDeg, float pitchDeg, float rollDeg);
