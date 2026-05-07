#include "track_config_validation.h"

#include <cstddef>

namespace azimuth_cfg {

bool isHostnameCharsValid(const char* s) {
  if (!s || !*s) {
    return false;
  }
  size_t n = 0;
  for (; s[n] != '\0'; ++n) {
    const unsigned char c = static_cast<unsigned char>(s[n]);
    if (c >= 'a' && c <= 'z') {
      continue;
    }
    if (c >= '0' && c <= '9') {
      continue;
    }
    if (c == '-') {
      continue;
    }
    return false;
  }
  return n <= 24;
}

std::string normalizeHostname(const char* s) {
  std::string out;
  if (!s) {
    return out;
  }
  out = s;
  const auto first = out.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    out.clear();
    return out;
  }
  const auto last = out.find_last_not_of(" \t\r\n");
  out = out.substr(first, last - first + 1);
  for (char& c : out) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return out;
}

bool isValidImuPeriodMs(int p) {
  return p == 5 || p == 10 || p == 20 || p == 40;
}

bool isValidWifiTxProfile(int tx) {
  return tx >= 0 && tx <= 2;
}

bool isValidLedMode(int m) {
  return m >= 0 && m <= 3;
}

bool inRangeInclusive(int v, int lo, int hi) {
  return v >= lo && v <= hi;
}

}  // namespace azimuth_cfg
