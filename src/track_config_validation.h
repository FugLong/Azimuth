#pragma once

#include <string>

namespace azimuth_cfg {

bool isHostnameCharsValid(const char* s);
std::string normalizeHostname(const char* s);

bool isValidImuPeriodMs(int p);
bool isValidWifiTxProfile(int tx);
bool isValidLedMode(int m);

bool inRangeInclusive(int v, int lo, int hi);

}  // namespace azimuth_cfg
