#include <cassert>
#include <string>

#include "../src/track_config_validation.h"

using azimuth_cfg::inRangeInclusive;
using azimuth_cfg::isHostnameCharsValid;
using azimuth_cfg::isValidImuPeriodMs;
using azimuth_cfg::isValidLedMode;
using azimuth_cfg::isValidWifiTxProfile;
using azimuth_cfg::normalizeHostname;

int main() {
  assert(isHostnameCharsValid("azimuth"));
  assert(isHostnameCharsValid("abc-123"));
  assert(!isHostnameCharsValid(""));
  assert(!isHostnameCharsValid("bad_host"));
  assert(!isHostnameCharsValid("UPPER"));
  assert(!isHostnameCharsValid("this-hostname-is-way-too-long-12345"));

  assert(normalizeHostname("  AZIMUTH  ") == "azimuth");
  assert(normalizeHostname("\nA-bC-9\t") == "a-bc-9");
  assert(normalizeHostname("   ") == "");

  assert(isValidImuPeriodMs(5));
  assert(isValidImuPeriodMs(10));
  assert(isValidImuPeriodMs(20));
  assert(isValidImuPeriodMs(40));
  assert(!isValidImuPeriodMs(0));
  assert(!isValidImuPeriodMs(15));

  assert(isValidWifiTxProfile(0));
  assert(isValidWifiTxProfile(1));
  assert(isValidWifiTxProfile(2));
  assert(!isValidWifiTxProfile(-1));
  assert(!isValidWifiTxProfile(3));

  assert(isValidLedMode(0));
  assert(isValidLedMode(3));
  assert(!isValidLedMode(-1));
  assert(!isValidLedMode(4));

  assert(inRangeInclusive(0, 0, 100));
  assert(inRangeInclusive(100, 0, 100));
  assert(!inRangeInclusive(-1, 0, 100));
  assert(!inRangeInclusive(101, 0, 100));

  return 0;
}
