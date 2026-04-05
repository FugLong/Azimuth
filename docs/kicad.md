# KiCad

**Integrated board:** open **`kicad/Azimuth_Design/Azimuth.kicad_pro`**. Schematic **`Azimuth.kicad_sch`**, layout **`Azimuth.kicad_pcb`**. Nets and GPIOs: [wiring.md](wiring.md) (PCB path). Human BOM summary: [parts-list.md](parts-list.md); **fab BOM** = export from KiCad for each order (re-run **ERC** and **DRC** after any design change — repo snapshot is clean).

| Location | Contents |
|----------|----------|
| **`kicad/Azimuth_Design/`** | Current project — sym/fp tables point at `../libraries/...` and `../easyeda2kicad_parts/...` |
| **`kicad/libraries/1_MyLib.kicad_sym`** | Project symbols (edit here; **Update Symbol from Library** in sch if copies drift) |
| **`kicad/libraries/1_MyFootPrints.pretty/`** | Project footprints |
| **`kicad/libraries/panelization.pretty/`** | Mouse-bite / panel footprints ([upstream](https://github.com/madworm/Panelization.pretty)) for production arrays |
| **`kicad/easyeda2kicad_parts/`** | Footprints + 3D pulled from EasyEDA — [README](../kicad/easyeda2kicad_parts/README.md) for import script |
| **`kicad/3d/*.STEP`** | Local STEP models referenced from some footprints |

Always open the **`.kicad_pro`** inside **`Azimuth_Design/`** so **`${KIPRJMOD}`** and library paths work.

**Before each fab order:** Re-run **ERC** and **DRC**, then export BOM and Gerbers; add a **panel** per your fab’s rules if you are not ordering singles.

**Buzzer path:** **BUZZER1** is driven by **Q2** (**AO3400A**, **`ao3400_C20917`**) and **D2** (**B5819WS**, **`b5819ws_C64886`**) from **`easyeda2kicad_parts`** — see [parts-list.md](parts-list.md#buzzer-buzzer1) and [wiring.md](wiring.md#pcb-path-azimuth_design-u1-esp32-c3-wroom-02).

**Legacy:** **`kicad/ESP32_BNO086/`** — old layout, not maintained. Use **`Azimuth_Design`**.

**3D preview missing?** Footprints may list **`${AMZPATH}`** / **`${KICAD_3DMODELS}`** or paths under **`easyeda2kicad_parts`** — set env vars or rely on **`${KIPRJMOD}/../3d/...`** where used.
