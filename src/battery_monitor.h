#pragma once

#include <stdint.h>

namespace azimuth_battery {

struct Status {
  bool supported = false;
  bool stub = true;
  int32_t millivolts = -1;
};

void init();
Status readStatus();

}  // namespace azimuth_battery
