# Fab exports (Azimuth_Design)

## PCBWay-style “Other Parameters” (copy-paste)

| Form field | Value |
|------------|--------|
| **Unique Parts** | **33** |
| **SMD Parts** (tooltip *SMT Pads*) | **52** (SMD placements) **or** **234** (SMD pads in PCB — use if the estimator expects pad count) |
| **BGA/QFP Parts** | **0** |
| **Through-Hole Parts** | **0** |

**Not a contradiction:** **33** = distinct **part numbers** (unique SKUs to order). **52** = **placement count** — each symbol on the board is one placement, but many symbols share the same LCSC (many resistors/caps reuse a few codes). **234** = physical **SMD pads** on the PCB (pads per footprint), only if a form asks for pad count.

**Unique parts:** 32 distinct **LCSC Part** values on stuffed BOM lines + **IC1** (no LCSC on symbol) = **33**. **SMD placements:** **52** stuffed lines. **SMD pads:** **234** × `pad … smd …` in **`Azimuth.kicad_pcb`**. **IC1** is **LGA**, not BGA/QFP.

---

## `Azimuth_bom.csv`

Machine-generated **Bill of Materials** from **`../Azimuth.kicad_sch`** using **KiCad’s `kicad-cli sch export bom`** (same field stack the GUI BOM tool uses). The same script also writes **`Azimuth.net`** via **`kicad-cli sch export netlist`**. **Commit updates** whenever the schematic BOM-relevant fields change so docs can cite a real file instead of hand-wavy counts.

Regenerate from the repo root:

```bash
./scripts/export_azimuth_bom.sh
```

Or set `KICAD_CLI` if `kicad-cli` is not on your `PATH` (see script).

### Numbers taken from the committed CSV (for assembly forms)

These are derived from **`Azimuth_bom.csv`** as checked in — re-open the CSV after regeneration if the design changed.

| Question | Answer |
|----------|--------|
| BOM lines (`kicad-cli`, one row per schematic symbol) | **52** |
| Rows with `Value` = `DNP` | **0** |
| Rows for a **stuffed** build | **52** |
| Distinct **LCSC Part** codes on stuffed lines (non-empty) | **32** |
| Distinct **ordering lines** if you treat “no LCSC” as its own line (**IC1** / BNO086 has no LCSC on the symbol) | **33** |
| Through-hole **components** | **0** (all stuffed lines are SMD footprints) |
| BGA / QFP | **0** — **IC1** is **LGA** |

**PCBWay:** their KiCad plugin often writes under **`_pcbway_export_review/`** (gitignored here). If you use their CSV for an order, copy or regenerate into this **`fab/`** folder when you want those exact columns tracked in git.

## `Azimuth_bom_cost.txt`

**One-board parts-only USD estimate** (unit prices × qty). Written by **`./scripts/export_azimuth_bom.sh`** after the BOM CSV export. Uses **`Unit Price (USD)`** from the schematic when present; otherwise fills from the same LCSC/MPN table as **`scripts/update_azimuth_sch_bom_fields.py`**. **Not** live pricing, **not** PCB fab or shipping.

## `Azimuth.net` (optional)

Regenerate after schematic changes if you track a netlist export here.
