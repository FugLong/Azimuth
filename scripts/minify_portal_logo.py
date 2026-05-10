#!/usr/bin/env python3
r"""
Minify an SVG to one line for embedding in src/portal_html.cpp.

  python3 scripts/minify_portal_logo.py

Optional: python3 scripts/minify_portal_logo.py path/to/other.svg

For the production logo from CAD, prefer generating from `logo/AzimuthLogo.dxf`
(patch `web/index.html`, then regenerate firmware HTML):

  .venv/bin/python scripts/dxf_simple_to_portal_svg.py --patch-portal
  python3 scripts/portal_codegen.py --generate
"""
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_SVG = ROOT / "logo" / "AzimuthLogo_traced.svg"


def _short_floats(s: str, max_decimals: int = 2) -> str:
    """Shorten 369.20703125-style numbers (paths + transforms)."""

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
        f'<svg viewBox="{view_box}" fill="currentColor" role="img" '
        'aria-labelledby="azLogoTitle"><title id="azLogoTitle">Azimuth</title>'
    )
    raw = raw[: m.start()] + head + raw[m.end() :]
    raw = _short_floats(raw)
    raw = re.sub(r"\s+", " ", raw).strip()
    return raw


def main() -> None:
    path = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else DEFAULT_SVG
    if not path.is_file():
        print(f"missing {path}", file=sys.stderr)
        sys.exit(1)
    raw = path.read_text(encoding="utf-8")
    print(minify_portal_svg(raw))


if __name__ == "__main__":
    main()
