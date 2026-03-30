# KiCad

**Integrated board:** open **`kicad/Azimuth_Design/Azimuth.kicad_pro`**. Schematic **`Azimuth.kicad_sch`**, layout **`Azimuth.kicad_pcb`**. Nets and GPIOs: [wiring.md](wiring.md) (PCB path). Human BOM summary: [parts-list.md](parts-list.md); **fab BOM** = export from KiCad after ERC/DRC.

| Location | Contents |
|----------|----------|
| **`kicad/Azimuth_Design/`** | Current project — sym/fp tables point at `../libraries/...` and `../easyeda2kicad_parts/...` |
| **`kicad/libraries/1_MyLib.kicad_sym`** | Project symbols (edit here; **Update Symbol from Library** in sch if copies drift) |
| **`kicad/libraries/1_MyFootPrints.pretty/`** | Project footprints |
| **`kicad/libraries/panelization.pretty/`** | Mouse-bite / panel footprints ([upstream](https://github.com/madworm/Panelization.pretty)) for production arrays |
| **`kicad/easyeda2kicad_parts/`** | Footprints + 3D pulled from EasyEDA — [README](../kicad/easyeda2kicad_parts/README.md) for import script |
| **`kicad/3d/*.STEP`** | Local STEP models referenced from some footprints |

Always open the **`.kicad_pro`** inside **`Azimuth_Design/`** so **`${KIPRJMOD}`** and library paths work.

**Before fab:** ERC, DRC, locked BOM; add panel per your house rules if you’re not ordering singles.

**Legacy:** **`kicad/ESP32_BNO086/`** — old layout, not maintained. Use **`Azimuth_Design`**.

**3D preview missing?** Footprints may list **`${AMZPATH}`** / **`${KICAD_3DMODELS}`** or paths under **`easyeda2kicad_parts`** — set env vars or rely on **`${KIPRJMOD}/../3d/...`** where used.
