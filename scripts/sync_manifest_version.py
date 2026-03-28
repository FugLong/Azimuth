#!/usr/bin/env python3
"""Set web-flasher/manifest.json \"version\" from repo VERSION (first line)."""
import json
import sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent
manifest = Path(sys.argv[1]) if len(sys.argv) > 1 else root / "web-flasher" / "manifest.json"
vf = root / "VERSION"
if not vf.is_file():
    print("sync_manifest_version: no VERSION file, skip", file=sys.stderr)
    sys.exit(0)

ver = vf.read_text(encoding="utf-8").strip().splitlines()[0].strip()
with manifest.open(encoding="utf-8") as f:
    data = json.load(f)
data["version"] = ver
with manifest.open("w", encoding="utf-8") as f:
    json.dump(data, f, indent=2)
    f.write("\n")
print(f"sync_manifest_version: {manifest.name} version -> {ver}")
