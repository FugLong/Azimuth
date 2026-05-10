#!/usr/bin/env python3
r"""
Convert `logo/AzimuthLogo.dxf` into a one-line inline SVG for the portal
(`web/index.html` logo-wrap). **Use this only if you are not using** `logo/AzimuthLogo.svg`
+ `minify_portal_logo.py` — vector effects and masks are lost here.

**DWG:** read **DXF only** — Save As → DXF from CAD. Native `.dwg` is not parsed in-repo.

Full-sheet **artboard rectangles** are skipped so `evenodd` fill does not become a solid slab.

Requires ezdxf (use repo venv):

  python3 -m venv .venv
  .venv/bin/pip install ezdxf
  .venv/bin/python scripts/dxf_simple_to_portal_svg.py --patch-portal
  python3 scripts/portal_codegen.py --generate

`--patch-portal` updates `web/index.html`; regenerate `src/portal_html.cpp` with
`portal_codegen.py` before building firmware.

Optional: `scripts/minify_portal_logo.py` can embed a hand-tuned **`logo/AzimuthLogo.svg`**
if you maintain SVG separately (Illustrator `px` stroke quirks apply).

DXF entities are converted via `ezdxf.path.make_path` (LWPOLYLINE, SPLINE,
HATCH, CIRCLE, etc.); **INSERT** blocks are expanded recursively.
Each path is flattened with `Path.flattening()`, Y is flipped for SVG,
bbox is normalized to a 0-based viewBox, and one `<path>` is emitted with
fill-rule="evenodd". **MTEXT** is skipped.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_DXF = ROOT / "logo" / "AzimuthLogo.dxf"
WEB_INDEX_HTML = ROOT / "web" / "index.html"


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
        from ezdxf.path import make_path
    except ImportError as e:
        raise SystemExit(
            "ezdxf is required. From repo root: python3 -m venv .venv && "
            ".venv/bin/pip install ezdxf"
        ) from e

    def walk_layout(layout):
        """Yield drawable entities, expanding INSERT recursively."""
        for ent in layout:
            if ent.dxftype() == "INSERT":
                yield from walk_layout(ent.virtual_entities())
            else:
                yield ent

    doc = ezdxf.readfile(str(dxf_path))
    all_pts: list[list[tuple[float, float]]] = []
    for ent in walk_layout(doc.modelspace()):
        if ent.dxftype() == "MTEXT":
            continue
        try:
            path = make_path(ent)
        except TypeError:
            continue
        if path is None:
            continue
        for sub in path.sub_paths():
            pts = list(sub.flattening(flat_tol))
            if len(pts) < 2:
                continue
            all_pts.append([(float(p.x), float(p.y)) for p in pts])

    if not all_pts:
        raise SystemExit(
            f"No drawable geometry in {dxf_path} — check DXF layers / contents."
        )

    xs = [x for poly in all_pts for x, _ in poly]
    ys = [y for poly in all_pts for _, y in poly]
    xmin, xmax = min(xs), max(xs)
    ymin, ymax = min(ys), max(ys)
    w = xmax - xmin
    h = ymax - ymin

    def _skip_artboard_frame(poly: list[tuple[float, float]], tol: float = 6.0) -> bool:
        """CAD exports often include a full-sheet rectangle; merged into one evenodd path it
        reads as a solid slab with tiny holes — skip that contour."""
        if len(poly) < 4:
            return False
        nx = [float(x) - xmin for x, _ in poly]
        ny = [float(ymax) - float(y) for _, y in poly]
        if max(nx) - min(nx) < max(w, h) * 0.85:
            return False
        if max(ny) - min(ny) < max(w, h) * 0.85:
            return False
        return (
            abs(min(nx)) <= tol
            and abs(min(ny)) <= tol
            and abs(max(nx) - w) <= tol
            and abs(max(ny) - h) <= tol
            and len(poly) <= 6
        )

    filtered = [p for p in all_pts if not _skip_artboard_frame(p)]
    if filtered:
        all_pts = filtered
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
        f'<svg viewBox="0 0 {vb_w} {vb_h}" preserveAspectRatio="xMidYMid meet" '
        'fill="currentColor" role="img" '
        'aria-labelledby="azLogoTitle"><title id="azLogoTitle">Azimuth</title> '
        f'<path fill-rule="{fill_rule}" d="{d_attr}"/>'
    )
    out = head + "</svg>"
    out = _short_floats(out)
    out = re.sub(r"\s+", " ", out).strip()
    return out


def patch_logo_wrap(html_path: pathlib.Path, svg_line: str) -> None:
    raw = html_path.read_text(encoding="utf-8")
    start = raw.find('<div class="logo-wrap">')
    if start < 0:
        raise SystemExit(f"{html_path}: logo-wrap div not found")
    end = raw.find("</div>", start)
    if end < 0:
        raise SystemExit(f"{html_path}: closing </div> for logo-wrap not found")
    end += len("</div>")
    new_block = f'<div class="logo-wrap">{svg_line}</div>'
    html_path.write_text(raw[:start] + new_block + raw[end:], encoding="utf-8")


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
        help=f"Replace logo-wrap in {WEB_INDEX_HTML.relative_to(ROOT)} (then run portal_codegen.py)",
    )
    args = ap.parse_args()
    if not args.dxf.is_file():
        print(f"missing {args.dxf}", file=sys.stderr)
        sys.exit(1)
    svg = dxf_to_minified_svg(
        args.dxf, flat_tol=args.flat_tol, fill_rule=args.fill_rule
    )
    if args.patch_portal:
        patch_logo_wrap(WEB_INDEX_HTML, svg)
        print(
            f"Patched {WEB_INDEX_HTML.relative_to(ROOT)} — run:\n"
            f"  python3 scripts/portal_codegen.py --generate",
            file=sys.stderr,
        )
    else:
        print(svg)


if __name__ == "__main__":
    main()
