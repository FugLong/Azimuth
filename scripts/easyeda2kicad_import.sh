#!/usr/bin/env bash
# Import an LCSC part via EasyEDA data into kicad/easyeda2kicad_parts/easyeda2kicad/
# Uses repo-root venv: .venv_easyeda2kicad (gitignored)
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV="$ROOT/.venv_easyeda2kicad"
REQ="$ROOT/kicad/easyeda2kicad_parts/requirements.txt"
# easyeda2kicad 0.8.x writes easyeda2kicad.kicad_sym, easyeda2kicad.pretty/, easyeda2kicad.3dshapes/ here:
DEST="$ROOT/kicad/easyeda2kicad_parts"

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <LCSC id, e.g. C2934560 or 2934560>" >&2
  exit 1
fi

LCSC="$1"
[[ "$LCSC" == C* ]] || LCSC="C${LCSC}"

if [[ ! -f "$REQ" ]]; then
  echo "Missing $REQ" >&2
  exit 1
fi

if [[ ! -d "$VENV" ]]; then
  python3 -m venv "$VENV"
fi
"$VENV/bin/pip" install -q --upgrade pip
"$VENV/bin/pip" install -q -r "$REQ"

mkdir -p "$DEST"
"$VENV/bin/easyeda2kicad" --full --lcsc_id="$LCSC" --output "$DEST" --overwrite

echo "Done: $LCSC → kicad/easyeda2kicad_parts/ (easyeda2kicad.kicad_sym + .pretty + .3dshapes). Add sym-lib-table / fp-lib-table if needed."
