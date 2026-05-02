#include "battery_monitor.h"

#include "board_config.h"

namespace azimuth_battery {
namespace {
Status gStatus;
}  // namespace

void init() {
  gStatus.supported = azimuth_board::capabilities().hasBatterySense;
  gStatus.stub = true;
  gStatus.millivolts = -1;
}

Status readStatus() {
  return gStatus;
}

}  // namespace azimuth_battery
