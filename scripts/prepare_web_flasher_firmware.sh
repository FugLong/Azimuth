#!/usr/bin/env bash
# Copy ESP32-C3 flash images into web-flasher/firmware/ for GitHub Pages + esp-web-tools manifest.
set -euo pipefail

ENV_NAME="${1:-azimuth_main}"
BUILD="${PLATFORMIO_BUILD_DIR:-.pio/build/${ENV_NAME}}"
DEST="${2:-web-flasher/firmware}"

mkdir -p "$DEST"

for f in bootloader.bin partitions.bin firmware.bin; do
  if [[ ! -f "${BUILD}/${f}" ]]; then
    echo "Missing ${BUILD}/${f} — run: pio run -e ${ENV_NAME}" >&2
    exit 1
  fi
  cp "${BUILD}/${f}" "$DEST/"
done

if [[ -f "${BUILD}/boot_app0.bin" ]]; then
  cp "${BUILD}/boot_app0.bin" "$DEST/"
else
  FOUND=""
  if [[ -d "${HOME}/.platformio/packages" ]]; then
    FOUND="$(find "${HOME}/.platformio/packages" -path '*framework-arduinoespressif32*' -name boot_app0.bin 2>/dev/null | head -1 || true)"
  fi
  if [[ -z "${FOUND}" || ! -f "${FOUND}" ]]; then
    echo "boot_app0.bin not in build dir and not found under ~/.platformio/packages" >&2
    exit 1
  fi
  cp "${FOUND}" "$DEST/boot_app0.bin"
fi

echo "Copied flash images to ${DEST}/"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [[ -f "${ROOT}/VERSION" ]]; then
  python3 "${ROOT}/scripts/sync_manifest_version.py" "${ROOT}/web-flasher/manifest.json"
fi
