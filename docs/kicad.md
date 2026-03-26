# KiCad — libraries and collaboration

## What’s in this repo

| Path | Purpose |
|------|---------|
| **`kicad/libraries/1_MyLib.kicad_sym`** | Project symbols: **BNO086**, **XIAO**, **K3-1280S-K1** (PWR1), **MLT-5020** (**BUZZER1**), **HX** tactile (**FUNC1**), **JS102011SAQN** (in library; **not** placed on the current PCB), etc. |
| **`kicad/libraries/1_MyFootPrints.pretty/`** | Project footprints: **XIAO-ESP32-C3-DIP-SMD**, **SW-SMD_K3-1280S-K1**, **KEY-SMD_L4.0-W3.0-LS4.9-1**, **BUZ-SMD_L5.0-W5.5-P4.60**, etc. |
| **`kicad/ESP32_BNO086/sym-lib-table`** | Points `${KIPRJMOD}/../libraries/1_MyLib.kicad_sym` — open **`ESP32_BNO086.kicad_pro`** so **`${KIPRJMOD}`** resolves. |
| **`kicad/ESP32_BNO086/fp-lib-table`** | Points `${KIPRJMOD}/../libraries/1_MyFootPrints.pretty`. |
| **`kicad/easyeda2kicad_parts/`** | Imported footprints / 3D from EasyEDA (e.g. switch); some **3D model** paths under footprints still reference this tree or absolute paths—verify after clone. |
| **`kicad/3d/*.STEP`** | Local STEP models; PCB may reference `${KIPRJMOD}/../3d/...` from the XIAO footprint. |

Open the project via **`kicad/ESP32_BNO086/ESP32_BNO086.kicad_pro`** so **`${KIPRJMOD}`** is that folder (required for the tables and 3D paths above).

## Board vs library names

| Doc / colloquial | KiCad |
|------------------|--------|
| “Battery slide switch” | **PWR1**, value **K3-1280S-K1** |
| Buzzer | **BUZZER1**, value **MLT-5020** |
| Raw pack positive net | **`/Bat+`** |
| Switched rail to XIAO Bat+ / divider / **C1** / **C2** | **`vcc`** |

## Schematic note

**`ESP32_BNO086.kicad_sch`** embeds copies of custom symbols in `(lib_symbols …)`. Edit **`1_MyLib.kicad_sym`** for future boards; use **Update Symbol from Library** when you want the sheet to track the file on disk.

## 3D models — XIAO / `${AMZPATH}`

The PCB may list **several** 3D models for the XIAO footprint (alternate vendors / env vars). If **`${AMZPATH}`** or **`${KICAD_3DMODELS}`** is not set on your machine, KiCad skips those entries and uses whatever resolves first (often the stock **`${KICAD9_3DMODEL_DIR}`** shapes). To guarantee the Seeed STEP everywhere, either set those env vars or trim the footprint’s model list to a single **`${KIPRJMOD}/../3d/...`** file and commit that STEP.

## Git and large STEP files

**`.STEP`** files can be large. If the repo grows unpleasantly, consider **[Git LFS](https://git-lfs.com/)** for `kicad/3d/*.STEP` only.
