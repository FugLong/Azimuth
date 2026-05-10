#include "track_update.h"

#ifndef IMU_DEBUG_MODE
#define IMU_DEBUG_MODE 1
#endif

namespace azimuth_update {

const char* phaseString(Phase p) {
  switch (p) {
    case Phase::Idle:
      return "idle";
    case Phase::Connecting:
      return "connecting";
    case Phase::Downloading:
      return "downloading";
    case Phase::Finalizing:
      return "finalizing";
    case Phase::Success:
      return "success";
    case Phase::Failed:
      return "failed";
  }
  return "idle";
}

const char* beginResultString(BeginResult r) {
  switch (r) {
    case BeginResult::Started:
      return "started";
    case BeginResult::AlreadyActive:
      return "already_active";
    case BeginResult::NoNetwork:
      return "no_network";
    case BeginResult::OfflineAp:
      return "offline_ap";
    case BeginResult::ThermalHold:
      return "thermal_hold";
    case BeginResult::BatteryCritical:
      return "battery_critical";
    case BeginResult::PartitionUnavailable:
      return "partition_unavailable";
    case BeginResult::HttpError:
      return "http_error";
    case BeginResult::ChipBusy:
      return "chip_busy";
  }
  return "unknown";
}

}  // namespace azimuth_update

#if IMU_DEBUG_MODE

namespace azimuth_update {

BeginResult beginUpdate() { return BeginResult::ChipBusy; }
void tick() {}
bool isActive() { return false; }
Status status() {
  Status s;
  return s;
}

}  // namespace azimuth_update

#else

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <cstring>

#include "battery_monitor.h"
#include "board_config.h"
#include "io_buzzer.h"
#include "track_network.h"
#include "track_network_internal.h"

#ifndef AZIMUTH_RELEASE_FIRMWARE_URL
#define AZIMUTH_RELEASE_FIRMWARE_URL ""
#endif

namespace azimuth_update {
namespace {

/**
 * Per-tick I/O budget. ESP32-C3 flash erase is ~50 ms / 4 KB sector; reading 8 KB
 * across the TLS socket and writing it lands a single tick in roughly 100–200 ms,
 * which keeps the WebServer / IMU loop responsive while still finishing a ~1 MB
 * application image in well under a minute on a healthy LAN.
 */
constexpr size_t kChunkBytes = 4096;
/** Hard ceiling so a stuck server can't pin us in `Downloading` forever. */
constexpr uint32_t kStreamIdleTimeoutMs = 12000;
/** TLS handshake / GET timeout (ms). */
constexpr uint32_t kHttpTimeoutMs = 8000;

struct Runtime {
  Phase phase = Phase::Idle;
  uint32_t totalBytes = 0;
  uint32_t writtenBytes = 0;
  uint32_t lastByteAtMs = 0;
  uint32_t startedAtMs = 0;
  const char* errorMessage = "";
  WiFiClientSecure* tls = nullptr;
  HTTPClient* http = nullptr;
  WiFiClient* stream = nullptr;
  uint8_t buf[kChunkBytes] = {0};
};

Runtime gRt;

void releaseHttp() {
  if (gRt.http) {
    gRt.http->end();
    delete gRt.http;
    gRt.http = nullptr;
  }
  if (gRt.tls) {
    gRt.tls->stop();
    delete gRt.tls;
    gRt.tls = nullptr;
  }
  gRt.stream = nullptr;
}

void abortUpdate(const char* reason) {
  if (Update.isRunning()) {
    Update.abort();
  }
  releaseHttp();
  gRt.phase = Phase::Failed;
  gRt.errorMessage = reason ? reason : "unknown";
  azimuth_io_buzzer::playUpdateFailTune();
}

bool deriveFirmwareUrl(char* out, size_t outLen) {
  // Compile-time override always wins (handy for staging).
  const char* fixed = AZIMUTH_RELEASE_FIRMWARE_URL;
  if (fixed && fixed[0]) {
    if (strlen(fixed) >= outLen) {
      return false;
    }
    strncpy(out, fixed, outLen);
    out[outLen - 1] = '\0';
    return true;
  }
  // Otherwise derive `<base>/firmware/<board>/firmware.bin` from the manifest URL
  // (e.g. `https://fuglong.github.io/Azimuth/manifest.json` →
  //  `https://fuglong.github.io/Azimuth/firmware/diy/firmware.bin`).
  const char* manifest = AZIMUTH_RELEASE_MANIFEST_URL;
  if (!manifest || !manifest[0]) {
    return false;
  }
  const char* slash = strrchr(manifest, '/');
  if (!slash) {
    return false;
  }
  const size_t baseLen = static_cast<size_t>(slash - manifest);
  const char* board = azimuth_board::boardName();
  // base + "/firmware/" + board + "/firmware.bin" + NUL
  const size_t need = baseLen + 10 + strlen(board) + 13 + 1;
  if (need > outLen) {
    return false;
  }
  memcpy(out, manifest, baseLen);
  out[baseLen] = '\0';
  strncat(out, "/firmware/", outLen - strlen(out) - 1);
  strncat(out, board, outLen - strlen(out) - 1);
  strncat(out, "/firmware.bin", outLen - strlen(out) - 1);
  return true;
}

bool batterySafe() {
  const auto batt = azimuth_battery::readStatus();
  if (!batt.supported || batt.stub) {
    return true;
  }
  if (strcmp(batt.chargeState, "absent") == 0 ||
      strcmp(batt.chargeState, "charging") == 0) {
    return true;
  }
  return batt.percent < 0 || batt.percent > 15;
}

}  // namespace

bool isActive() {
  return gRt.phase == Phase::Connecting || gRt.phase == Phase::Downloading ||
         gRt.phase == Phase::Finalizing;
}

Status status() {
  Status s;
  s.phase = gRt.phase;
  s.totalBytes = gRt.totalBytes;
  s.writtenBytes = gRt.writtenBytes;
  if (gRt.totalBytes > 0) {
    const uint64_t pct = (static_cast<uint64_t>(gRt.writtenBytes) * 100ULL) / gRt.totalBytes;
    s.progressPercent = static_cast<uint8_t>(pct > 100 ? 100 : pct);
  } else {
    s.progressPercent = 0;
  }
  s.errorMessage = (gRt.phase == Phase::Failed) ? gRt.errorMessage : "";
  return s;
}

BeginResult beginUpdate() {
  if (isActive()) {
    return BeginResult::AlreadyActive;
  }
  if (azimuth_net::gRuntime.thermalHoldActive) {
    return BeginResult::ThermalHold;
  }
  if (azimuth_net::gRuntime.offlineApMode) {
    return BeginResult::OfflineAp;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return BeginResult::NoNetwork;
  }
  if (!batterySafe()) {
    return BeginResult::BatteryCritical;
  }
  if (Update.isRunning()) {
    return BeginResult::ChipBusy;
  }

  char url[160] = {0};
  if (!deriveFirmwareUrl(url, sizeof(url))) {
    return BeginResult::HttpError;
  }

  // Park UDP/Hatire so radio bandwidth and CPU go to the download.
  trackNetworkSetStasis(true);

  gRt.tls = new (std::nothrow) WiFiClientSecure();
  gRt.http = new (std::nothrow) HTTPClient();
  if (!gRt.tls || !gRt.http) {
    releaseHttp();
    return BeginResult::HttpError;
  }
  gRt.tls->setTimeout(kHttpTimeoutMs / 1000);
  // Same trust anchor as the existing manifest check (Let's Encrypt ISRG Root X1
  // by default; configurable via -DAZIMUTH_RELEASE_MANIFEST_CA_CERT). A MitM
  // would need a chain rooted there, not just any CA.
  gRt.tls->setCACert(azimuth_net::releaseRootCaCert());
  gRt.http->setTimeout(kHttpTimeoutMs);
  gRt.http->setReuse(false);
  gRt.http->useHTTP10(true);  // disable chunked; we want Content-Length for progress.

  if (!gRt.http->begin(*gRt.tls, url)) {
    releaseHttp();
    return BeginResult::HttpError;
  }

  gRt.phase = Phase::Connecting;
  gRt.errorMessage = "";
  gRt.totalBytes = 0;
  gRt.writtenBytes = 0;
  gRt.startedAtMs = millis();
  gRt.lastByteAtMs = millis();

  const int code = gRt.http->GET();
  if (code != HTTP_CODE_OK) {
    static char errBuf[40];
    snprintf(errBuf, sizeof(errBuf), "HTTP %d", code);
    abortUpdate(errBuf);
    return BeginResult::HttpError;
  }

  const int len = gRt.http->getSize();
  if (len <= 0) {
    abortUpdate("missing Content-Length");
    return BeginResult::HttpError;
  }
  gRt.totalBytes = static_cast<uint32_t>(len);

  if (!Update.begin(gRt.totalBytes, U_FLASH)) {
    static char errBuf[64];
    snprintf(errBuf, sizeof(errBuf), "Update.begin: %s", Update.errorString());
    abortUpdate(errBuf);
    return BeginResult::PartitionUnavailable;
  }

  gRt.stream = gRt.http->getStreamPtr();
  if (!gRt.stream) {
    abortUpdate("no stream");
    return BeginResult::HttpError;
  }

  gRt.phase = Phase::Downloading;
  azimuth_io_buzzer::playUpdateStartTune();
  return BeginResult::Started;
}

void tick() {
  if (gRt.phase != Phase::Downloading) {
    return;
  }
  if (!gRt.stream || !gRt.http) {
    abortUpdate("stream lost");
    return;
  }

  const uint32_t now = millis();
  if (now - gRt.lastByteAtMs > kStreamIdleTimeoutMs) {
    abortUpdate("stream idle timeout");
    return;
  }

  // Drain at most one chunk per tick to keep IMU + WebServer responsive.
  size_t avail = static_cast<size_t>(gRt.stream->available());
  if (avail == 0) {
    // Connection still alive? If TLS closed and we have what we needed, finalize.
    if (!gRt.http->connected()) {
      if (gRt.writtenBytes >= gRt.totalBytes) {
        // fall through to finalize below
      } else {
        abortUpdate("connection closed early");
        return;
      }
    } else {
      return;
    }
  }

  if (avail > kChunkBytes) {
    avail = kChunkBytes;
  }
  if (avail > 0) {
    const int got = gRt.stream->readBytes(reinterpret_cast<char*>(gRt.buf), avail);
    if (got <= 0) {
      // Treat as transient — we'll retry next tick (until idle timeout).
      return;
    }
    const size_t written = Update.write(gRt.buf, static_cast<size_t>(got));
    if (written != static_cast<size_t>(got)) {
      static char errBuf[64];
      snprintf(errBuf, sizeof(errBuf), "Update.write: %s", Update.errorString());
      abortUpdate(errBuf);
      return;
    }
    gRt.writtenBytes += written;
    gRt.lastByteAtMs = millis();
  }

  if (gRt.writtenBytes >= gRt.totalBytes) {
    gRt.phase = Phase::Finalizing;
    if (!Update.end(true)) {
      static char errBuf[64];
      snprintf(errBuf, sizeof(errBuf), "Update.end: %s", Update.errorString());
      abortUpdate(errBuf);
      return;
    }
    releaseHttp();
    gRt.phase = Phase::Success;
    gRt.errorMessage = "";
    azimuth_io_buzzer::playUpdateOkTune();
    // Give the buzzer one tune-length to flush, then reboot into the new partition.
    delay(900);
    ESP.restart();
  }
}

}  // namespace azimuth_update

#endif  // !IMU_DEBUG_MODE
