# EasyEDA → KiCad imports (LCSC)

Symbols, footprints, and 3D models pulled from EasyEDA/LCSC using **[easyeda2kicad](https://pypi.org/project/easyeda2kicad/)**.

## Layout (simple, one folder per LCSC part)

Under **`easyeda2kicad/`**, each part is self-contained (same idea as **`button_C49108648`**, **`switch_C128953`**, etc.):

| Part | Files |
|------|--------|
| **WROOM-02** (C2934560) | **`../easyeda2kicad.kicad_sym`**, **`../easyeda2kicad.pretty/`**, **`../easyeda2kicad.3dshapes/`** (legacy single-module import) |
| **USB Type-C** C2765186 | **`easyeda2kicad/usb_type_c_C2765186.kicad_sym`**, **`usb_type_c_C2765186.pretty/`**, **`usb_type_c_C2765186.3dshapes/`** |
| **RGB LED** C779813 (TUOZHAN 1615 common anode) | **`easyeda2kicad/rgb_led_C779813.kicad_sym`**, **`rgb_led_C779813.pretty/`**, **`rgb_led_C779813.3dshapes/`** |
| **AO3400A** C20917 (buzzer low-side NMOS) | **`easyeda2kicad/ao3400_C20917.kicad_sym`**, **`ao3400_C20917.pretty/`**, **`ao3400_C20917.3dshapes/`** |
| **AO3401A** C347476 (reverse-polarity PMOS candidate) | **`easyeda2kicad/ao3401a_C347476.kicad_sym`**, **`ao3401a_C347476.pretty/`**, **`ao3401a_C347476.3dshapes/`** |
| **B5819WS** C64886 (buzzer flyback Schottky) | **`easyeda2kicad/b5819ws_C64886.kicad_sym`**, **`b5819ws_C64886.pretty/`**, **`b5819ws_C64886.3dshapes/`** |

Symbol **`usb_type_c_C2765186:TYPE-C16PIN2MD(073)`**; footprint **`usb_type_c_C2765186:USB-C-SMD_TYPE-C-6PIN-2MD-073`**.

Symbol **`rgb_led_C779813:TZ-P4-1615RGBTCA1-0.55T`**; footprint **`rgb_led_C779813:LED-SMD_4P-L1.6-W1.5-BR_TZ-P4-1615`**. EasyEDA labels cathode pins **`2`–`4`** as **`-`** — confirm **R/G/B** vs pad number against the vendor datasheet before layout.

Do **not** merge new LCSC parts into **`easyeda2kicad.kicad_sym`** — add a new **`descriptive_name_C<number>.kicad_sym`** (+ matching **`.pretty`** / **`.3dshapes`**) so **`--overwrite`** on a fresh import cannot wipe unrelated symbols.

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

**Option B — manual** (use a **dedicated** output prefix so you do not overwrite **`easyeda2kicad.kicad_sym`**):

```bash
source .venv_easyeda2kicad/bin/activate
easyeda2kicad --full --lcsc_id=C2934560 --output kicad/easyeda2kicad_parts/_import_C2934560 --overwrite
```

Then move/rename the generated **`.kicad_sym`**, **`.pretty`**, and **`.3dshapes`** into **`easyeda2kicad/<name>_<Cnumber>/`** (or merge only that symbol by hand if you must). Add **`sym-lib-table`** and **`fp-lib-table`** entries for your KiCad project.

## Notes

- Footprints may embed **absolute** 3D model paths from the machine that ran the import; after clone, fix **`model`** paths in **`.kicad_mod`** or re-run import locally.
- For RF modules, **always** cross-check the footprint and keepout against the **vendor datasheet**; EasyEDA geometry can be wrong or outdated.
