# KiCad — libraries and collaboration

## What’s in this repo

| Path | Purpose |
|------|---------|
| **`kicad/libraries/1_MyLib.kicad_sym`** | Project symbol library (BNO086, XIAO, JS102011SAQN, SMT-0540, …). |
| **`kicad/libraries/1_MyFootPrints.pretty/`** | Project footprint library (XIAO footprint, switch, buzzer, …). |
| **`kicad/ESP32_BNO086/sym-lib-table`** | Points `${KIPRJMOD}/../libraries/1_MyLib.kicad_sym` — KiCad resolves this after opening the `.kicad_pro`. |
| **`kicad/ESP32_BNO086/fp-lib-table`** | Points `${KIPRJMOD}/../libraries/1_MyFootPrints.pretty`. |
| **`kicad/3d/*.STEP`** | Local 3D models that used to live only under `Downloads/`; PCB references `${KIPRJMOD}/../3d/...`. |

Open the project via **`kicad/ESP32_BNO086/ESP32_BNO086.kicad_pro`** so **`${KIPRJMOD}`** is that folder (required for the tables and 3D paths above).

## Schematic note

**`ESP32_BNO086.kicad_sch`** still embeds copies of custom symbols in `(lib_symbols …)`. The vendored **`1_MyLib.kicad_sym`** is what you edit for future boards; use **Update Symbol from Library** when you want the sheet to track the file on disk.

## 3D models — XIAO / `${AMZPATH}`

The PCB may list **several** 3D models for the XIAO footprint (alternate vendors / env vars). If **`${AMZPATH}`** or **`${KICAD_3DMODELS}`** is not set on your machine, KiCad skips those entries and uses whatever resolves first (often the stock **`${KICAD9_3DMODEL_DIR}`** shapes). To guarantee the Seeed STEP everywhere, either set those env vars or trim the footprint’s model list to a single **`${KIPRJMOD}/../3d/...`** file and commit that STEP.

## Git and large STEP files

**`.STEP`** files can be large. If the repo grows unpleasantly, consider **[Git LFS](https://git-lfs.com/)** for `kicad/3d/*.STEP` only.
