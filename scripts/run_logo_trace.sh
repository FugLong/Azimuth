#!/usr/bin/env bash
# One-shot: ephemeral venv → pip install vtracer → trace logo → delete venv.
# First run can take several minutes (Rust wheel build). Output: logo/AzimuthLogo_traced.svg
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
VENV="$ROOT/.venv-logo-trace"
rm -rf "$VENV"
python3 -m venv "$VENV"
"$VENV/bin/pip" install -q --upgrade pip
"$VENV/bin/pip" install -q vtracer pillow
"$VENV/bin/python" "$ROOT/scripts/logo_png_to_svg.py" "$@"
rm -rf "$VENV"
echo "Removed ephemeral venv: .venv-logo-trace"
