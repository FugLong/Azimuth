#pragma once

#include <stdint.h>

namespace azimuth_thermal {

void init();
/** Call from main loop; samples on a power-profile-based interval, independent of the web UI. */
void tick(uint32_t now_ms);
/** Last on-die temperature (°C), or NaN if not yet sampled / invalid. */
float lastChipTempC();
/** `ok` · `warn` (≥80 °C) · `emergency` (before Wi‑Fi cut) · `thermal_hold` (radio off). */
const char* stateJsonString();
/** Called when firmware disables Wi‑Fi for overheating — resets sample timing for recovery checks. */
void notifyThermalHoldEntered();

}  // namespace azimuth_thermal
