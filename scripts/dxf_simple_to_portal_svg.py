#!/usr/bin/env python3
r"""
Convert `logo/AzimuthLogo_dxf_SIMPLE.dxf` into a one-line inline SVG for
`src/portal_html.cpp` (same embedding style as `minify_portal_logo.py`).

Requires ezdxf (use repo venv):

  python3 -m venv .venv
  .venv/bin/pip install ezdxf
  .venv/bin/python scripts/dxf_simple_to_portal_svg.py
  .venv/bin/python scripts/dxf_simple_to_portal_svg.py --patch-portal

DXF uses SPLINE entities; we flatten each spline to a polyline, flip Y for SVG,
normalize to a 0-based viewBox, and emit one <path> with fill-rule="evenodd"
for sane compound fills.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_DXF = ROOT / "logo" / "AzimuthLogo_dxf_SIMPLE.dxf"
PORTAL_CPP = ROOT / "src" / "portal_html.cpp"


def _short_floats(s: str, max_decimals: int = 2) -> str:
    def repl(m: re.Match[str]) -> str:
        n = float(m.group(0))
        if abs(n - round(n)) < 1e-9:
            return str(int(round(n)))
        t = f"{{:.{max_decimals}f}}".format(n).rstrip("0").rstrip(".")
        return t if t != "-0" else "0"

    return re.sub(r"-?\d+\.\d+", repl, s)


def dxf_to_minified_svg(
    dxf_path: pathlib.Path,
    *,
    flat_tol: float = 0.55,
    fill_rule: str = "evenodd",
) -> str:
    try:
        import ezdxf
    except ImportError as e:
        raise SystemExit(
            "ezdxf is required. From repo root: python3 -m venv .venv && "
            ".venv/bin/pip install ezdxf"
        ) from e

    doc = ezdxf.readfile(str(dxf_path))
    msp = list(doc.modelspace())
    all_pts: list[list[tuple[float, float]]] = []
    for e in msp:
        pts = list(e.construction_tool().flattening(flat_tol))
        all_pts.append([(float(p.x), float(p.y)) for p in pts])

    xs = [x for poly in all_pts for x, _ in poly]
    ys = [y for poly in all_pts for _, y in poly]
    xmin, xmax = min(xs), max(xs)
    ymin, ymax = min(ys), max(ys)
    w = xmax - xmin
    h = ymax - ymin

    def fmt(n: float) -> str:
        if abs(n - round(n)) < 1e-6:
            return str(int(round(n)))
        return f"{n:.2f}".rstrip("0").rstrip(".")

    chunks: list[str] = []
    for poly in all_pts:
        if len(poly) < 2:
            continue
        x0, y0 = poly[0]
        parts = [f"M{fmt(x0 - xmin)} {fmt(ymax - y0)}"]
        for x, y in poly[1:]:
            parts.append(f"L{fmt(x - xmin)} {fmt(ymax - y)}")
        chunks.append("".join(parts))
    d_attr = "".join(chunks)

    vb_w = fmt(w)
    vb_h = fmt(h)
    head = (
        f'<svg viewBox="0 0 {vb_w} {vb_h}" fill="currentColor" role="img" '
        'aria-labelledby="azLogoTitle"><title id="azLogoTitle">Azimuth</title> '
        f'<path fill-rule="{fill_rule}" d="{d_attr}"/>'
    )
    out = head + "</svg>"
    out = _short_floats(out)
    out = re.sub(r"\s+", " ", out).strip()
    return out


def patch_portal_cpp(svg_line: str) -> None:
    raw = PORTAL_CPP.read_text(encoding="utf-8")
    start = raw.find('<div class="logo-wrap">')
    if start < 0:
        raise SystemExit("portal_html.cpp: logo-wrap div not found")
    end = raw.find("</div>", start)
    if end < 0:
        raise SystemExit("portal_html.cpp: closing </div> for logo-wrap not found")
    end += len("</div>")
    new_block = f'<div class="logo-wrap">{svg_line}</div>'
    PORTAL_CPP.write_text(raw[:start] + new_block + raw[end:], encoding="utf-8")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("dxf", nargs="?", type=pathlib.Path, default=DEFAULT_DXF)
    ap.add_argument(
        "--flat-tol",
        type=float,
        default=0.55,
        help="Spline flatten distance (drawing units); larger = smaller file",
    )
    ap.add_argument(
        "--fill-rule",
        choices=("evenodd", "nonzero"),
        default="evenodd",
    )
    ap.add_argument(
        "--patch-portal",
        action="store_true",
        help=f"Replace logo in {PORTAL_CPP.relative_to(ROOT)}",
    )
    args = ap.parse_args()
    if not args.dxf.is_file():
        print(f"missing {args.dxf}", file=sys.stderr)
        sys.exit(1)
    svg = dxf_to_minified_svg(
        args.dxf, flat_tol=args.flat_tol, fill_rule=args.fill_rule
    )
    if args.patch_portal:
        patch_portal_cpp(svg)
        print(f"Patched {PORTAL_CPP.relative_to(ROOT)}", file=sys.stderr)
    else:
        print(svg)


if __name__ == "__main__":
    main()
