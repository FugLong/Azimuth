#!/usr/bin/env python3
"""
Generate src/portal_html.cpp from web/index.html.

Usage:
  python3 scripts/portal_codegen.py --generate
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
CPP_PATH = ROOT / "src" / "portal_html.cpp"
WEB_ROOT = ROOT / "web"
HTML_PATH = WEB_ROOT / "index.html"
CPP_HEADER = '#include <Arduino.h>\n#include "portal_html.h"\n\n'


def _read(path: pathlib.Path) -> str:
    return path.read_text(encoding="utf-8")


def _write(path: pathlib.Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def _render_cpp_from_html(html: str) -> str:
    # Keep deterministic output format for clean diffs.
    return (
        CPP_HEADER
        + 'const char kPortalIndexHtml[] PROGMEM = R"AZPORTAL('
        + html
        + ')AZPORTAL";\n'
    )


def _normalize_eol(s: str) -> str:
    return s.replace("\r\n", "\n").replace("\r", "\n")


def _inline_css_js_assets(html: str) -> str:
    """Inline local stylesheet/script assets for embedded firmware delivery."""
    html = _normalize_eol(html)

    # <link rel="stylesheet" href="...">
    link_re = re.compile(
        r'<link\s+[^>]*rel=["\']stylesheet["\'][^>]*href=["\']([^"\']+)["\'][^>]*>\s*',
        flags=re.IGNORECASE,
    )

    def repl_css(match: re.Match[str]) -> str:
        href = match.group(1).strip()
        if "://" in href or href.startswith("//"):
            return match.group(0)
        css_path = (WEB_ROOT / href).resolve()
        if not css_path.exists():
            raise ValueError(f"Missing stylesheet referenced by index.html: {href}")
        css = _normalize_eol(_read(css_path))
        return f"<style>\n{css}</style>\n"

    html = link_re.sub(repl_css, html)

    # <script src="..."></script>
    script_re = re.compile(
        r'<script\s+[^>]*src=["\']([^"\']+)["\'][^>]*>\s*</script>\s*',
        flags=re.IGNORECASE,
    )

    def repl_js(match: re.Match[str]) -> str:
        src = match.group(1).strip()
        if "://" in src or src.startswith("//"):
            return match.group(0)
        js_path = (WEB_ROOT / src).resolve()
        if not js_path.exists():
            raise ValueError(f"Missing script referenced by index.html: {src}")
        js = _normalize_eol(_read(js_path))
        return f"<script>\n{js}</script>\n"

    html = script_re.sub(repl_js, html)
    return html


def cmd_generate() -> int:
    html = _normalize_eol(_read(HTML_PATH))
    html = _inline_css_js_assets(html)
    # Ensure trailing newline in raw-string payload to keep stable formatting.
    if not html.endswith("\n"):
        html += "\n"
    cpp = _render_cpp_from_html(html)
    # Trim accidental extra blank lines before the raw-string closing marker.
    cpp = re.sub(r"\n+\)AZPORTAL\";\n$", "\n)AZPORTAL\";\n", cpp, flags=re.MULTILINE)
    _write(CPP_PATH, cpp)
    print(f"Generated portal C++ at {CPP_PATH}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Azimuth portal code generator")
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--generate", action="store_true", help="Generate src/portal_html.cpp from web/index.html")
    args = parser.parse_args()

    try:
        if args.generate:
            return cmd_generate()
    except FileNotFoundError as err:
        print(f"Missing file: {err}", file=sys.stderr)
        return 1
    except ValueError as err:
        print(str(err), file=sys.stderr)
        return 2

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
