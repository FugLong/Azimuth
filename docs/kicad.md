# KiCad

**Canonical design:** **`Azimuth.kicad_sch`** and **`Azimuth.kicad_pcb`** define nets, GPIO usage, and layout. Repo markdown ([wiring.md](wiring.md), [parts-list.md](parts-list.md), etc.) and firmware constants (**`include/azimuth_hw.h`**) follow the schematic/PCB. After schematic edits, re-export whatever you track under **`fab/`** (netlist, BOM) so tooling matches the board.

**Integrated board:** open **`kicad/Azimuth_Design/Azimuth.kicad_pro`**. Schematic **`Azimuth.kicad_sch`**, layout **`Azimuth.kicad_pcb`**. Nets and GPIOs: [wiring.md](wiring.md) (PCB path). Human BOM summary: [parts-list.md](parts-list.md). **Tracked machine BOM:** **`kicad/Azimuth_Design/fab/Azimuth_bom.csv`** (KiCad **`kicad-cli sch export bom`** — regenerate with **`./scripts/export_azimuth_bom.sh`**; notes in **`fab/README.md`**). For each **fab order**, re-run **ERC** and **DRC**, refresh that CSV if the schematic changed, then export **gerbers / fab-specific BOM** as your house requires.

| Location | Contents |
|----------|----------|
| **`kicad/Azimuth_Design/`** | Current project — sym/fp tables point at `../libraries/...` and `../easyeda2kicad_parts/...` |
| **`kicad/libraries/1_MyLib.kicad_sym`** | Project symbols (edit here; **Update Symbol from Library** in sch if copies drift) |
| **`kicad/libraries/1_MyFootPrints.pretty/`** | Project footprints |
| **`kicad/libraries/panelization.pretty/`** | Mouse-bite / panel footprints ([upstream](https://github.com/madworm/Panelization.pretty)) for production arrays |
| **`kicad/easyeda2kicad_parts/`** | Footprints + 3D pulled from EasyEDA — [README](../kicad/easyeda2kicad_parts/README.md) for import script |
| **`kicad/3d/*.STEP`** | Local STEP models referenced from some footprints |

Always open the **`.kicad_pro`** inside **`Azimuth_Design/`** so **`${KIPRJMOD}`** and library paths work.

**Before each fab order:** Re-run **ERC** and **DRC**, refresh **`fab/Azimuth_bom.csv`** via **`./scripts/export_azimuth_bom.sh`**, then export **Gerbers** (and your fab’s BOM/position files). Add a **panel** per your fab’s rules if you are not ordering singles.

**BOM fields:** Placed symbols in **`Azimuth.kicad_sch`** include **`Description`** (functional / electrical role) and hidden **`Unit Price (USD)`** (low-qty ballpark from LCSC-style codes — **not** live pricing). Regenerate or edit prices in **`scripts/update_azimuth_sch_bom_fields.py`**, then run `python3 scripts/update_azimuth_sch_bom_fields.py`. Add **`Unit Price (USD)`** to your BOM export field list in KiCad if you want extended CSV with line estimates.

**Buzzer path:** **BUZZER1** is driven by **Q2** (**AO3400A**, **`ao3400_C20917`**) and **D2** (**B5819WS**, **`b5819ws_C64886`**) from **`easyeda2kicad_parts`** — see [parts-list.md](parts-list.md#buzzer-buzzer1) and [wiring.md](wiring.md#pcb-path-azimuth_design-u1-esp32-c3-wroom-02).

**Legacy:** **`kicad/ESP32_BNO086/`** — old layout, not maintained. Use **`Azimuth_Design`**.

**3D preview missing?** Footprints may list **`${AMZPATH}`** / **`${KICAD_3DMODELS}`** or paths under **`easyeda2kicad_parts`** — set env vars or rely on **`${KIPRJMOD}/../3d/...`** where used.
