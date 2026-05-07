#pragma once

#include <stdint.h>

namespace azimuth_power {

/** Portal HTTP `handleClient` cadence when AP or STA web is active (settings / monitoring — not live dashboards). */
uint16_t networkServiceIntervalMs();
/** DNS retry / UDP bind retry / modem-sleep policy — not on every main-loop iteration. */
uint32_t networkBackgroundPeriodMs();
/** Background on-die temperature sampling when not in thermal hold. */
uint32_t thermalSamplePeriodMs();
/** STA idle time after last portal HTTP activity before enabling modem sleep. */
uint32_t wifiSleepIdleDelayMs();

}  // namespace azimuth_power
