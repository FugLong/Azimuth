"""Inject AZIMUTH_FW_VERSION from repo VERSION (runs only for envs that include this script)."""
Import("env")

from pathlib import Path

root = Path(env["PROJECT_DIR"])
vf = root / "VERSION"
ver = "0.1.0"
if vf.is_file():
    lines = vf.read_text(encoding="utf-8").strip().splitlines()
    if lines and lines[0].strip():
        ver = lines[0].strip()
# C string macro: -DAZIMUTH_FW_VERSION=\"x.y.z\" (json.dumps breaks PP: 0.1.0 looks like a float)
env.Append(BUILD_FLAGS=['-DAZIMUTH_FW_VERSION=\\"' + ver + '\\"'])
