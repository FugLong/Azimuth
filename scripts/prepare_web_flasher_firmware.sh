#!/usr/bin/env bash
# Copy ESP32-C3 flash images into a web-flasher firmware directory.
set -euo pipefail

ENV_NAME="${1:-azimuth_main_diy}"
BUILD="${PLATFORMIO_BUILD_DIR:-.pio/build/${ENV_NAME}}"
DEST="${2:-web-flasher/firmware/diy}"

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
    FOUND="$(python3 - <<'PY'
import os
from pathlib import Path
root = Path.home() / ".platformio" / "packages"
for p in root.glob("**/framework-arduinoespressif32*/tools/partitions/boot_app0.bin"):
    print(str(p))
    break
PY
)"
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
  if [[ -f "${ROOT}/web-flasher/manifest-pcb.json" ]]; then
    python3 "${ROOT}/scripts/sync_manifest_version.py" "${ROOT}/web-flasher/manifest-pcb.json"
  fi
fi
