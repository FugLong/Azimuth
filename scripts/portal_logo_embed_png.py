#!/usr/bin/env python3
"""
Embed a raster image inside the portal header.

  python3 scripts/portal_logo_embed_png.py --patch-web
  python3 scripts/portal_codegen.py --generate

Default image: **logo/AzimuthLogo_Light.png** (portal is dark-themed). Output is **WebP**
(lossy, still crisp on line art) at **384px** max edge by default — smaller in firmware than PNG
at the same pixel count. Use `--format png` if you need PNG only; tune with `--max-side`,
`--webp-quality`, or `--full-res` (risks huge firmware).

Requires **Pillow** for any scaled embed (`pip install pillow`).

**Accuracy vs size (practical order):**
- Clean **Illustrator SVG** (when everything exports) → smallest + crisp.
- **PNG at portal resolution** (this script, default resize) → pixel-accurate raster, flash‑friendly.
- **vtracer** SVG from PNG → small, quality varies (fine for silhouettes; noisy on gradients).
- **DXF** path in-repo for CAD geometry; use PNG/raster for parts that won’t export cleanly.
**DWG** is not useful in-browser/firmware — export **DXF** if you need vectors.

**BMP**: decoded with Pillow, scaled like other rasters (raw BMP is huge and unsuitable for PROGMEM).
"""
from __future__ import annotations

import argparse
import base64
import io
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
WEB_INDEX_HTML = ROOT / "web" / "index.html"
DEFAULT_PNG = ROOT / "logo" / "AzimuthLogo_Light.png"

# Portal CSS caps logo at ~232px wide; 384px ≈ 1.65× (enough for retina) without paying for 512² pixels.
DEFAULT_MAX_SIDE = 384
DEFAULT_WEBP_QUALITY = 82


def _mime_for_suffix(suffix: str) -> str:
    s = suffix.lower()
    if s in (".jpg", ".jpeg"):
        return "image/jpeg"
    if s == ".webp":
        return "image/webp"
    if s in (".bmp", ".dib"):
        return "image/bmp"
    return "image/png"


def _prepare_embed_bytes(
    path: pathlib.Path,
    max_side: int | None,
    *,
    force_png_output: bool,
    embed_format: str,
    webp_quality: int,
) -> tuple[bytes, str]:
    """Return (raw_bytes, mime_type) for the data URL."""
    suffix = path.suffix.lower()
    use_pillow = force_png_output or max_side is not None or suffix in (".bmp", ".dib")

    if not use_pillow:
        raw = path.read_bytes()
        return raw, _mime_for_suffix(suffix)

    try:
        from PIL import Image  # noqa: E402
    except ImportError:
        print(
            "Pillow is required for scaled raster embeds and BMP. Install: pip install pillow",
            file=sys.stderr,
        )
        sys.exit(1)

    im = Image.open(path)
    if im.mode not in ("RGB", "RGBA"):
        im = im.convert("RGBA")
    if max_side is not None:
        im.thumbnail((max_side, max_side), Image.Resampling.LANCZOS)

    buf = io.BytesIO()
    if embed_format == "webp":
        # method=6 = slowest/best compression; small script cost, fewer flash bytes.
        im.save(
            buf,
            format="WEBP",
            quality=webp_quality,
            method=6,
            lossless=False,
        )
        return buf.getvalue(), "image/webp"

    im.save(buf, format="PNG", optimize=True, compress_level=9)
    return buf.getvalue(), "image/png"


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
    ap = argparse.ArgumentParser(description="Embed raster logo in portal logo-wrap (base64 data URL).")
    ap.add_argument(
        "--png",
        type=pathlib.Path,
        default=DEFAULT_PNG,
        metavar="PATH",
        help=f"Raster logo: PNG, JPEG, WebP, or BMP (default: {DEFAULT_PNG.relative_to(ROOT)})",
    )
    ap.add_argument(
        "--max-side",
        type=int,
        default=None,
        metavar="PX",
        help=f"Resize so longest edge is at most PX (default {DEFAULT_MAX_SIDE} if omitted).",
    )
    ap.add_argument(
        "--format",
        choices=("webp", "png"),
        default="webp",
        dest="embed_format",
        help="Embedded image format: WebP is usually smallest (default). Use png for widest compatibility.",
    )
    ap.add_argument(
        "--webp-quality",
        type=int,
        default=DEFAULT_WEBP_QUALITY,
        metavar="1-100",
        help=f"WebP quality when --format webp (default {DEFAULT_WEBP_QUALITY}; lower = smaller, more artifact).",
    )
    ap.add_argument(
        "--full-res",
        action="store_true",
        help="Embed file bytes with no resize (PNG/JPEG/WebP only; risks huge firmware).",
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

    if args.full_res and png_path.suffix.lower() in (".bmp", ".dib"):
        print(
            "Use PNG/WebP export instead of raw BMP for --full-res, or omit --full-res to scale.",
            file=sys.stderr,
        )
        sys.exit(1)

    max_side: int | None
    if args.full_res:
        max_side = None
    elif args.max_side is not None:
        max_side = args.max_side
    else:
        max_side = DEFAULT_MAX_SIDE

    if not (1 <= args.webp_quality <= 100):
        print("--webp-quality must be 1–100", file=sys.stderr)
        sys.exit(1)

    raw, mime = _prepare_embed_bytes(
        png_path,
        max_side,
        force_png_output=png_path.suffix.lower() in (".bmp", ".dib"),
        embed_format=args.embed_format,
        webp_quality=args.webp_quality,
    )
    b64 = base64.standard_b64encode(raw).decode("ascii")

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
