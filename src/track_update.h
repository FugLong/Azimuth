#pragma once

#include <stdint.h>

/**
 * Wireless firmware update (OTA) — pulls `firmware/<board>/firmware.bin` from the same
 * GitHub Pages release the manifest check already trusts, into the standby OTA app
 * partition, then reboots into it on success. The default Arduino-ESP32 partition
 * table already includes `ota_0`/`ota_1`/`otadata`, so no platformio.ini changes
 * are needed (see docs/io-led-buzzer-plan.md → Wireless updates).
 *
 * Design notes:
 * - Chunked / cooperative: each `azimuth_update::tick()` reads up to ~4 KB and writes
 *   to `Update`, so the WebServer + IMU loop stay responsive throughout.
 * - Uses the low-level `Update` class directly (not `HTTPUpdate`) to avoid the
 *   Arduino-ESP32 HTTPUpdate MD5-mismatch regression on ESP32-C3 (esp-web-tools /
 *   esphome issue #13255, 2025+).
 * - Refuses to start in offline AP mode, while the chip is in thermal hold, or when
 *   the battery is critical and not charging — flash-writing while browning out can
 *   brick the device.
 */
namespace azimuth_update {

enum class Phase : uint8_t {
  Idle = 0,
  Connecting,
  Downloading,
  Finalizing,
  Success,
  Failed,
};

struct Status {
  Phase phase = Phase::Idle;
  uint32_t totalBytes = 0;
  uint32_t writtenBytes = 0;
  uint8_t progressPercent = 0;
  /** Last error message (empty when phase != Failed). Pointer to static string. */
  const char* errorMessage = "";
};

/**
 * Reasons `beginUpdate` can refuse to start. Mirrored as JSON strings by the portal.
 */
enum class BeginResult : uint8_t {
  Started = 0,
  AlreadyActive,
  NoNetwork,
  OfflineAp,
  ThermalHold,
  BatteryCritical,
  PartitionUnavailable,
  HttpError,
  ChipBusy,
};

const char* beginResultString(BeginResult r);
const char* phaseString(Phase p);

/** Kick off an OTA fetch from `<release base>/firmware/<board>/firmware.bin`. */
BeginResult beginUpdate();

/** Cooperative tick — call from the main `networkLoop` regularly. */
void tick();

bool isActive();
Status status();

}  // namespace azimuth_update
