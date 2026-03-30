#!/usr/bin/env bash
# Import an LCSC part via EasyEDA into kicad/easyeda2kicad_parts/_import_<Cnumber>/
# (Never writes to easyeda2kicad.kicad_sym / .pretty / .3dshapes at repo root — --overwrite would wipe WROOM.)
# Uses repo-root venv: .venv_easyeda2kicad (gitignored)
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV="$ROOT/.venv_easyeda2kicad"
REQ="$ROOT/kicad/easyeda2kicad_parts/requirements.txt"
PARTS="$ROOT/kicad/easyeda2kicad_parts"

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

OUT="$PARTS/_import_${LCSC}"
mkdir -p "$OUT"
"$VENV/bin/easyeda2kicad" --full --lcsc_id="$LCSC" --output "$OUT" --overwrite

echo "Done: $LCSC → $OUT"
echo "Move/rename into kicad/easyeda2kicad_parts/easyeda2kicad/<descriptive_name>_${LCSC}/ (see kicad/easyeda2kicad_parts/README.md), then add sym-lib-table / fp-lib-table entries."
