#!/usr/bin/env bash
# Download and run the Azimuth flasher (no repo clone required).
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/FugLong/Azimuth/main/scripts/azimuth-flash.sh | bash
#   curl -fsSL ... | bash -s diy
#   ./scripts/azimuth-flash.sh pcb
set -euo pipefail

REPO_RAW="${AZIMUTH_FLASH_SCRIPT_URL:-https://raw.githubusercontent.com/FugLong/Azimuth/main}"
BRANCH_PATH="scripts/azimuth-flash.py"

if [[ -n "${AZIMUTH_FLASH_LOCAL:-}" && -f "${AZIMUTH_FLASH_LOCAL}" ]]; then
  PY="${AZIMUTH_FLASH_LOCAL}"
elif [[ -f "$(dirname "$0")/azimuth-flash.py" ]]; then
  PY="$(cd "$(dirname "$0")" && pwd)/azimuth-flash.py"
else
  TMP="$(mktemp -d)"
  trap 'rm -rf "${TMP}"' EXIT
  PY="${TMP}/azimuth-flash.py"
  echo "Downloading Azimuth flasher…" >&2
  curl -fsSL "${REPO_RAW}/${BRANCH_PATH}" -o "${PY}"
fi

if ! command -v python3 >/dev/null 2>&1; then
  echo "python3 is required. Install Python 3 and re-run." >&2
  exit 1
fi

exec python3 "${PY}" "$@"
