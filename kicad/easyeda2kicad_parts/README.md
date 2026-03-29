# EasyEDA → KiCad imports (LCSC)

Symbols, footprints, and 3D models pulled from EasyEDA/LCSC using **[easyeda2kicad](https://pypi.org/project/easyeda2kicad/)** (same generator noted in existing `*.kicad_sym` files here).

With **easyeda2kicad 0.8.x**, imports land **in this folder** as:

- **`easyeda2kicad.kicad_sym`** (one library; `--overwrite` replaces the whole file — back up before re-importing other parts into the same lib)
- **`easyeda2kicad.pretty/`**
- **`easyeda2kicad.3dshapes/`**

Older parts in **`easyeda2kicad/easyeda2kicad/`** (`switch_C128953`, etc.) used an earlier layout; both can coexist.

## One-time: venv at repo root

From the **repository root**:

```bash
python3 -m venv .venv_easyeda2kicad
./.venv_easyeda2kicad/bin/pip install -r kicad/easyeda2kicad_parts/requirements.txt
```

`.venv_easyeda2kicad/` is **gitignored**.

## Import a part (LCSC `C` number)

**Option A — wrapper script** (creates the venv if missing):

```bash
./scripts/easyeda2kicad_import.sh C2934560
```

**Option B — manual**:

```bash
source .venv_easyeda2kicad/bin/activate
easyeda2kicad --full --lcsc_id=C2934560 --output kicad/easyeda2kicad_parts --overwrite
```

Then in KiCad, add the new **`*.kicad_sym`** to **`sym-lib-table`** and **`*.pretty`** to **`fp-lib-table`** for your project (same pattern as `button_C49234124`, `switch_C128953`, etc.).

## Notes

- Footprints may embed **absolute** 3D model paths from the machine that ran the import; after clone, fix **`model`** paths in **`.kicad_mod`** or re-run import locally.
- For RF modules, **always** cross-check the footprint and keepout against the **vendor datasheet**; EasyEDA geometry can be wrong or outdated.
