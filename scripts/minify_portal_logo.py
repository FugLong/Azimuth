#!/usr/bin/env python3
r"""
Minify an SVG to one line for embedding in web/index.html / src/portal_html.cpp.

  python3 scripts/minify_portal_logo.py              # stdout (default: logo/AzimuthLogo.svg)
  python3 scripts/minify_portal_logo.py other.svg

Patch portal web source and remind to codegen:

  python3 scripts/minify_portal_logo.py --patch-web
  python3 scripts/portal_codegen.py --generate

**Primary portal logo:** **`logo/AzimuthLogo.svg`** + **`--patch-web`** (below).

DXF-only users: `scripts/dxf_simple_to_portal_svg.py --patch-portal`.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_SVG = ROOT / "logo" / "AzimuthLogo.svg"
WEB_INDEX_HTML = ROOT / "web" / "index.html"


def prepare_illustrator_svg(raw: str) -> str:
    """Map Illustrator/CSS hex fills to currentColor so the portal theme tints the logo."""
    raw = re.sub(r"#333\b", "currentColor", raw)
    raw = raw.replace(
        "font-family: PTMono-Bold, 'PT Mono'",
        "font-family: system-ui, -apple-system, 'Segoe UI', sans-serif",
    )
    # Illustrator writes stroke/font sizes as CSS `px`. In a scaled inline SVG those are
    # *screen* pixels, so strokes and text stay huge while paths shrink — looks broken.
    # Unitless values use viewBox user units and scale with the artwork (same behavior as
    # the old single-path DXF logo).
    raw = raw.replace("stroke-width: 35px", "stroke-width: 35")
    # ~9 user units → ~0.8 CSS px at typical portal logo width — below one pixel, reads as missing.
    raw = raw.replace("stroke-width: 9px", "stroke-width: 14")
    raw = raw.replace("font-size: 500px", "font-size: 500")
    return raw


def _short_floats(s: str, max_decimals: int = 4) -> str:
    """Shorten floats in paths/transforms. Keep enough precision that compound paths
    (star ring / inner cutouts) don't collapse when minified — 2 decimals was eating thin gaps."""

    def repl(m: re.Match[str]) -> str:
        n = float(m.group(0))
        if abs(n - round(n)) < 1e-9:
            return str(int(round(n)))
        fmt = f"{{:.{max_decimals}f}}"
        t = fmt.format(n).rstrip("0").rstrip(".")
        return t if t != "-0" else "0"

    return re.sub(r"-?\d+\.\d+", repl, s)


def minify_portal_svg(raw: str) -> str:
    raw = re.sub(r"<\?xml[^?]*\?>", "", raw)
    raw = re.sub(r"<!--.*?-->", "", raw, flags=re.DOTALL)
    # Inherited from root; saves ~20+ chars per path
    raw = raw.replace(' fill="currentColor"', "")
    m = re.search(r"<svg\s+[^>]+>", raw)
    if not m:
        raise ValueError("no <svg> opening tag found")
    vb = re.search(r'viewBox="([^"]+)"', m.group(0))
    view_box = vb.group(1) if vb else "0 0 470 480"
    # HTML5 inline SVG does not need xmlns; default preserveAspectRatio is fine
    head = (
        f'<svg viewBox="{view_box}" preserveAspectRatio="xMidYMid meet" '
        'shape-rendering="geometricPrecision" '
        'fill="currentColor" role="img" '
        'aria-labelledby="azLogoTitle"><title id="azLogoTitle">Azimuth</title>'
    )
    raw = raw[: m.start()] + head + raw[m.end() :]
    raw = _short_floats(raw)
    raw = re.sub(r"\s+", " ", raw).strip()
    return raw


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
    ap = argparse.ArgumentParser(description="Minify SVG for Azimuth portal embedding.")
    ap.add_argument(
        "svg",
        nargs="?",
        type=pathlib.Path,
        default=DEFAULT_SVG,
        help=f"path to SVG (default: {DEFAULT_SVG.relative_to(ROOT)})",
    )
    ap.add_argument(
        "--patch-web",
        action="store_true",
        help=f"Replace logo-wrap in {WEB_INDEX_HTML.relative_to(ROOT)}",
    )
    ap.add_argument(
        "--raw",
        action="store_true",
        help="Skip Illustrator color prep (use for already-currentColor SVGs).",
    )
    args = ap.parse_args()
    path = args.svg.resolve()
    if not path.is_file():
        print(f"missing {path}", file=sys.stderr)
        sys.exit(1)
    raw = path.read_text(encoding="utf-8")
    if not args.raw:
        raw = prepare_illustrator_svg(raw)
    out = minify_portal_svg(raw)
    if args.patch_web:
        patch_logo_wrap(WEB_INDEX_HTML, out)
        print(
            f"Patched {WEB_INDEX_HTML.relative_to(ROOT)} — run:\n"
            f"  python3 scripts/portal_codegen.py --generate",
            file=sys.stderr,
        )
    else:
        print(out)


if __name__ == "__main__":
    main()
