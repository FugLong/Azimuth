#!/usr/bin/env python3
"""
Sum Azimuth_Design BOM line cost: Unit Price (USD) × Qty per fab/Azimuth_bom.csv.

Uses CSV column when present; otherwise LCSC Part / Value lookup from
update_azimuth_sch_bom_fields (same ballpark table as schematic field sync).
"""
from __future__ import annotations

import argparse
import csv
import importlib.util
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
DEFAULT_BOM = REPO / "kicad" / "Azimuth_Design" / "fab" / "Azimuth_bom.csv"
DEFAULT_OUT = REPO / "kicad" / "Azimuth_Design" / "fab" / "Azimuth_bom_cost.txt"


def _load_price_tables():
    path = REPO / "scripts" / "update_azimuth_sch_bom_fields.py"
    spec = importlib.util.spec_from_file_location("azimuth_bom_fields", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Cannot load {path}")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod.UNIT_PRICE_USD, mod.UNIT_PRICE_MPN


def row_unit_usd(row: dict, lcsc_usd: dict, mpn_usd: dict) -> tuple[float | None, str]:
    """Return (price, source note) or (None, reason)."""
    refs = (row.get("Refs") or "").strip()
    for key in (
        "Unit Price (USD)",
        "Unit USD",
        "Unit_Price_USD",
    ):
        raw = (row.get(key) or "").strip()
        if raw:
            try:
                return float(raw), f"csv:{key}"
            except ValueError:
                pass
    lcsc = (row.get("LCSC Part") or "").strip()
    if lcsc and lcsc in lcsc_usd:
        return float(lcsc_usd[lcsc]), f"lcsc:{lcsc}"
    val = (row.get("Value") or "").strip()
    if val in mpn_usd:
        return float(mpn_usd[val]), f"mpn:{val}"
    return None, "unpriced"


def main() -> int:
    p = argparse.ArgumentParser(description="Summarize Azimuth BOM USD total.")
    p.add_argument(
        "--bom",
        type=Path,
        default=DEFAULT_BOM,
        help="Path to Azimuth_bom.csv",
    )
    p.add_argument(
        "--write-summary",
        type=Path,
        default=DEFAULT_OUT,
        help="Write human-readable summary (default: fab/Azimuth_bom_cost.txt)",
    )
    p.add_argument(
        "--no-write",
        action="store_true",
        help="Print to stdout only",
    )
    args = p.parse_args()

    if not args.bom.is_file():
        print(f"BOM not found: {args.bom}", file=sys.stderr)
        return 1

    lcsc_usd, mpn_usd = _load_price_tables()

    total = 0.0
    priced = 0
    unpriced: list[str] = []
    lines = 0

    with args.bom.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            dnp = (row.get("DNP") or "").strip()
            if dnp:
                continue
            lines += 1
            qty_s = (row.get("Qty") or "1").strip() or "1"
            try:
                qty = int(qty_s)
            except ValueError:
                qty = 1
            price, src = row_unit_usd(row, lcsc_usd, mpn_usd)
            refs = (row.get("Refs") or "").strip()
            if price is None:
                unpriced.append(refs or "?")
                continue
            total += price * qty
            priced += 1

    text = "\n".join(
        [
            "Azimuth_Design — 1× board, parts-only estimate (USD)",
            "",
            f"Total (qty-weighted): ${total:.2f}",
            f"Priced lines: {priced} / {lines}",
        ]
    )
    if unpriced:
        text += f"\nUnpriced (add Unit Price on symbol or LCSC in table): {', '.join(unpriced)}"
    text += (
        "\n\nBallpark low-qty LCSC / Mouser-class numbers — not live quotes. "
        "Excludes PCB fab, assembly, shipping.\n"
        "Regenerate: ./scripts/export_azimuth_bom.sh"
    )

    if args.no_write:
        print(text)
    else:
        args.write_summary.parent.mkdir(parents=True, exist_ok=True)
        args.write_summary.write_text(text + "\n", encoding="utf-8")
        print(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
