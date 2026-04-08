#!/usr/bin/env bash
# Regenerate kicad/Azimuth_Design/fab/Azimuth_bom.csv using KiCad's CLI BOM export.
# Requires KiCad 9+ (kicad-cli). On macOS, PATH often omits the binary — we probe KiCad.app.

set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCH="$ROOT/kicad/Azimuth_Design/Azimuth.kicad_sch"
OUT="$ROOT/kicad/Azimuth_Design/fab/Azimuth_bom.csv"

KICAD_CLI="${KICAD_CLI:-}"
if [[ -z "$KICAD_CLI" ]]; then
  if command -v kicad-cli >/dev/null 2>&1; then
    KICAD_CLI="$(command -v kicad-cli)"
  elif [[ -x "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli" ]]; then
    KICAD_CLI="/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli"
  fi
fi
if [[ -z "$KICAD_CLI" || ! -x "$KICAD_CLI" ]]; then
  echo "kicad-cli not found. Install KiCad 9+ or set KICAD_CLI to the kicad-cli binary." >&2
  exit 1
fi

mkdir -p "$(dirname "$OUT")"
# Single quotes so ${QUANTITY} / ${DNP} are passed to KiCad, not expanded by the shell.
"$KICAD_CLI" sch export bom \
  --fields 'Reference,Value,Footprint,LCSC Part,Description,Unit Price (USD),${QUANTITY},${DNP}' \
  --labels 'Refs,Value,Footprint,LCSC Part,Description,Unit Price (USD),Qty,DNP' \
  -o "$OUT" \
  "$SCH"

python3 "$ROOT/scripts/summarize_azimuth_bom_cost.py" --bom "$OUT" --write-summary "$ROOT/kicad/Azimuth_Design/fab/Azimuth_bom_cost.txt"
