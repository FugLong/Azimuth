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
