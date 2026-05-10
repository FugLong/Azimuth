#!/usr/bin/env python3
"""
Embed a PNG inside the portal header (pixel-perfect; preserves anti-aliasing and
“fake white” gaps that disappear when vectorizing).

  python3 scripts/portal_logo_embed_png.py --patch-web
  python3 scripts/portal_codegen.py --generate

Default image: logo/AzimuthLogo_Dark.png — override with --png.

For **vector** output from the same PNG (smaller flash, less crisp), use vtracer:

  ./scripts/run_logo_trace.sh
  python3 scripts/minify_portal_logo.py logo/AzimuthLogo_traced.svg --patch-web
"""
from __future__ import annotations

import argparse
import base64
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
WEB_INDEX_HTML = ROOT / "web" / "index.html"
DEFAULT_PNG = ROOT / "logo" / "AzimuthLogo_Dark.png"


def patch_logo_wrap_inner(html_path: pathlib.Path, inner: str) -> None:
    raw = html_path.read_text(encoding="utf-8")
    start = raw.find('<div class="logo-wrap">')
    if start < 0:
        raise SystemExit(f"{html_path}: logo-wrap div not found")
    end = raw.find("</div>", start)
    if end < 0:
        raise SystemExit(f"{html_path}: closing </div> for logo-wrap not found")
    end += len("</div>")
    new_block = f'<div class="logo-wrap">{inner}</div>'
    html_path.write_text(raw[:start] + new_block + raw[end:], encoding="utf-8")


def main() -> None:
    ap = argparse.ArgumentParser(description="Embed PNG in portal logo-wrap (base64 data URL).")
    ap.add_argument(
        "--png",
        type=pathlib.Path,
        default=DEFAULT_PNG,
        help=f"Raster logo (default: {DEFAULT_PNG.relative_to(ROOT)})",
    )
    ap.add_argument(
        "--patch-web",
        action="store_true",
        help=f"Write into {WEB_INDEX_HTML.relative_to(ROOT)}",
    )
    args = ap.parse_args()
    png_path = args.png.resolve()
    if not png_path.is_file():
        print(f"missing {png_path}", file=sys.stderr)
        sys.exit(1)

    raw = png_path.read_bytes()
    b64 = base64.standard_b64encode(raw).decode("ascii")
    mime = "image/png"
    if png_path.suffix.lower() in (".jpg", ".jpeg"):
        mime = "image/jpeg"
    elif png_path.suffix.lower() == ".webp":
        mime = "image/webp"

    inner = (
        f'<img class="portal-logo-img" src="data:{mime};base64,{b64}" '
        'alt="Azimuth" decoding="async" fetchpriority="high"/>'
    )

    if args.patch_web:
        patch_logo_wrap_inner(WEB_INDEX_HTML, inner)
        print(
            f"Patched {WEB_INDEX_HTML.relative_to(ROOT)} — run:\n"
            f"  python3 scripts/portal_codegen.py --generate",
            file=sys.stderr,
        )
    else:
        print(inner[:200] + "..." if len(inner) > 200 else inner, file=sys.stderr)
        print("(use --patch-web to apply)", file=sys.stderr)


if __name__ == "__main__":
    main()
