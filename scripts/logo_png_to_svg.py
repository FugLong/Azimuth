#!/usr/bin/env python3
"""
Vectorize repo logo PNGs with vtracer (pip install). Intended to be run via:

  ./scripts/run_logo_trace.sh

or, if vtracer is already in your env:

  python3 scripts/logo_png_to_svg.py

Re-run web post-process only (no Pillow/vtracer):

  python3 scripts/logo_png_to_svg.py --postprocess-only

Default input: logo/AzimuthLogo_Dark.png (dark artwork; flattened on white for tracing).
Default output: logo/AzimuthLogo_traced.svg (commit this file; venv is not committed).

**White / “paper” becomes empty (no vector ink):** use `--key-white` — near-white RGB
and transparent pixels become **white** in the trace image; everything else becomes **black**.
VTracer then outputs paths only for the dark artwork (good when “gaps” are painted white,
not real vector holes).

  python3 scripts/logo_png_to_svg.py --key-white --white-threshold 248
"""
from __future__ import annotations

import argparse
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def flatten_on_white(src: Path, max_side: int) -> Path:
    from PIL import Image  # noqa: E402

    im = Image.open(src).convert("RGBA")
    im.thumbnail((max_side, max_side), Image.Resampling.LANCZOS)
    bg = Image.new("RGB", im.size, (255, 255, 255))
    bg.paste(im, mask=im.split()[3])
    tmp = Path(tempfile.mkstemp(suffix=".png", prefix="az_logo_")[1])
    bg.save(tmp, format="PNG", optimize=True)
    return tmp


def black_on_white_for_binary_trace(
    src: Path,
    max_side: int,
    *,
    white_threshold: int,
    alpha_cutoff: int,
) -> Path:
    """Map source PNG to pure black (ink) on pure white (background) for vtracer binary.

    - Transparent / low-alpha → background (white).
    - R,G,B all >= white_threshold → background (illustrator “paper”).
    - Else → foreground (black).
    """
    from PIL import Image  # noqa: E402

    im = Image.open(src).convert("RGBA")
    im.thumbnail((max_side, max_side), Image.Resampling.LANCZOS)
    w, h = im.size
    px = im.load()
    out = Image.new("RGB", (w, h), (255, 255, 255))
    opx = out.load()
    wt = max(0, min(255, white_threshold))
    ac = max(0, min(255, alpha_cutoff))
    ink = (0, 0, 0)
    paper = (255, 255, 255)
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a <= ac:
                opx[x, y] = paper
            elif r >= wt and g >= wt and b >= wt:
                opx[x, y] = paper
            else:
                opx[x, y] = ink
    tmp = Path(tempfile.mkstemp(suffix=".png", prefix="az_keyw_")[1])
    out.save(tmp, format="PNG", optimize=True)
    return tmp


def main() -> None:
    p = argparse.ArgumentParser(description="PNG → SVG via vtracer")
    p.add_argument(
        "--input",
        type=Path,
        default=ROOT / "logo" / "AzimuthLogo_Dark.png",
        help="Source PNG",
    )
    p.add_argument(
        "--output",
        type=Path,
        default=ROOT / "logo" / "AzimuthLogo_traced.svg",
        help="Write SVG here",
    )
    p.add_argument(
        "--max-side",
        type=int,
        default=480,
        help="Longest edge after resize (smaller = simpler paths, less flash)",
    )
    p.add_argument(
        "--postprocess-only",
        action="store_true",
        help="Only run web post-process on existing --output (no vtracer)",
    )
    p.add_argument(
        "--key-white",
        action="store_true",
        help="Treat near-white + transparent as background; trace only dark ink (see docstring)",
    )
    p.add_argument(
        "--white-threshold",
        type=int,
        default=248,
        help="With --key-white: pixel is paper if R,G,B are all >= this (0-255)",
    )
    p.add_argument(
        "--alpha-cutoff",
        type=int,
        default=40,
        help="With --key-white: alpha <= this counts as transparent/paper",
    )
    args = p.parse_args()

    out = args.output.resolve()
    if args.postprocess_only:
        if not out.is_file():
            print(f"Missing {out}", file=sys.stderr)
            raise SystemExit(1)
        postprocess_traced_svg(out)
        print(f"Post-processed {out}")
        return

    inp = args.input.resolve()
    if not inp.is_file():
        print(f"Input not found: {inp}", file=sys.stderr)
        raise SystemExit(1)

    out.parent.mkdir(parents=True, exist_ok=True)

    try:
        import vtracer
    except ImportError as e:  # pragma: no cover
        print("Missing vtracer. Run: ./scripts/run_logo_trace.sh", file=sys.stderr)
        raise SystemExit(1) from e

    if args.key_white:
        tmp_png = black_on_white_for_binary_trace(
            inp,
            args.max_side,
            white_threshold=args.white_threshold,
            alpha_cutoff=args.alpha_cutoff,
        )
    else:
        tmp_png = flatten_on_white(inp, args.max_side)
    try:
        vtracer.convert_image_to_svg_py(
            str(tmp_png),
            str(out),
            colormode="binary",
            hierarchical="stacked",
            mode="spline",
            filter_speckle=10,
            color_precision=6,
            layer_difference=20,
            corner_threshold=70,
            length_threshold=5,
            max_iterations=12,
            splice_threshold=50,
            path_precision=5,
        )
    finally:
        tmp_png.unlink(missing_ok=True)

    postprocess_traced_svg(out)
    n = out.stat().st_size
    print(f"Wrote {out} ({n // 1024} KiB)")


def postprocess_traced_svg(path: Path) -> None:
    """Make VTracer output usable on the dark portal: currentColor + responsive viewBox."""
    import re

    text = path.read_text(encoding="utf-8")
    text = text.replace('fill="#000000"', 'fill="currentColor"')
    text = text.replace('fill="#000"', 'fill="currentColor"')
    # Responsive root; strip fixed pixel dimensions from trace
    text = re.sub(
        r'<svg version="1\.1" xmlns="http://www\.w3\.org/2000/svg" width="(\d+)" height="(\d+)">',
        r'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 \1 \2" preserveAspectRatio="xMidYMid meet" fill="currentColor" role="img">',
        text,
        count=1,
    )
    text = re.sub(
        r"<!-- Generator: visioncortex VTracer [\d.]+ -->\s*",
        "",
        text,
        count=1,
    )
    path.write_text(text, encoding="utf-8")


if __name__ == "__main__":
    main()
