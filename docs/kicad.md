# KiCad — libraries and collaboration

## What’s in this repo

| Path | Purpose |
|------|---------|
| **`kicad/Azimuth_Design/`** | **Azimuth custom PCB** (main platform) — **ESP32-C3-WROOM-02**, **RGB LED**, **buzzer**, **FUNC** button, USB-C, battery area. [hardware-profiles.md](hardware-profiles.md), [wiring.md](wiring.md). Open **`Azimuth.kicad_pro`**. |
| **`kicad/libraries/1_MyLib.kicad_sym`** | Project symbols: **BNO086**, **XIAO**, **ESP32-C3-WROOM-02**, **K3-1280S-K1** (PWR1), **MLT-5020** (**BUZZER1**), **HX** tactile (**FUNC1**), **JS102011SAQN** (in library; **not** placed on every PCB), etc. |
| **`kicad/libraries/1_MyFootPrints.pretty/`** | Project footprints: **XIAO-ESP32-C3-DIP-SMD**, **SW-SMD_K3-1280S-K1**, **KEY-SMD_L4.0-W3.0-LS4.9-1**, **BUZ-SMD_L5.0-W5.5-P4.60**, etc. |
| **`kicad/libraries/panelization.pretty/`** | Vendored **[madworm/Panelization.pretty](https://github.com/madworm/Panelization.pretty)** mouse-bite footprints (`mouse-bite-1mm-slot` … `5mm-slot`) for fab panels. |
| **`kicad/Azimuth_Design/sym-lib-table`** / **`fp-lib-table`** | Point at **`../libraries/...`** and **`../easyeda2kicad_parts/...`** — open **`Azimuth.kicad_pro`** so **`${KIPRJMOD}`** resolves. |
| **`kicad/easyeda2kicad_parts/`** | Imported footprints / 3D from EasyEDA (e.g. switch); some **3D model** paths under footprints still reference this tree or absolute paths—verify after clone. **How to import:** [kicad/easyeda2kicad_parts/README.md](../kicad/easyeda2kicad_parts/README.md) — venv **`.venv_easyeda2kicad`** at repo root, **`scripts/easyeda2kicad_import.sh`**. |
| **`kicad/3d/*.STEP`** | Local STEP models; PCB may reference `${KIPRJMOD}/../3d/...` from footprints. |

Open a project via its **`.kicad_pro`** so **`${KIPRJMOD}`** is that project folder (required for **`sym-lib-table`** / **`fp-lib-table`** and 3D paths). **Azimuth_Design** loads **`easyeda2kicad`** (WROOM-02) and **`usb_type_c_C2765186`** from **`../easyeda2kicad_parts/easyeda2kicad/`**.

### Legacy design (`ESP32_BNO086`)

The directory **`kicad/ESP32_BNO086/`** is a **legacy** XIAO + BNO086 carrier layout — **untested** and not maintained to the same bar as **`Azimuth_Design`**. For a current BOM and supported layout, use **`kicad/Azimuth_Design/`**; for hand-wired DIY, see [wiring.md](wiring.md) (XIAO + BNO08x breakout).

## Pre-fabrication

Run **ERC** and **DRC**, freeze the **BOM**, then add **panelization** (array + rails/fiducials per fab house rules) for production ordering. Use footprints from library **`panelization`** (e.g. **`mouse-bite-2mm-slot`**) on **Edge.Cuts** per the upstream [README](https://github.com/madworm/Panelization.pretty) in that folder. Until the panel step is done, treat the single-board design as layout-complete but not fab-submitted.

## Board vs library names

| Doc / colloquial | KiCad |
|------------------|--------|
| “Battery slide switch” | **PWR1**, value **K3-1280S-K1** |
| Buzzer | **BUZZER1**, value **MLT-5020** |
| Raw pack positive net | **`/Bat+`** |
| Switched rail (example net names) | **`vcc`** or **`VBAT_SW`** — see **`Azimuth.kicad_sch`** |

## Schematic note

KiCad sheets may embed copies of custom symbols in `(lib_symbols …)`. Edit **`kicad/libraries/1_MyLib.kicad_sym`** as the source of truth; use **Update Symbol from Library** when you want a schematic to track the file on disk.

## 3D models — `${AMZPATH}`

Some footprints list **several** 3D models (alternate vendors / env vars). If **`${AMZPATH}`** or **`${KICAD_3DMODELS}`** is not set on your machine, KiCad skips those entries and uses whatever resolves first. To guarantee a specific STEP, set those env vars or trim the footprint’s model list to a single committed **`${KIPRJMOD}/../3d/...`** file.

## Git and large STEP files

**`.STEP`** files can be large. If the repo grows unpleasantly, consider **[Git LFS](https://git-lfs.com/)** for `kicad/3d/*.STEP` only.
