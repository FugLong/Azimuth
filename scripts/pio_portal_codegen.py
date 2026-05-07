from pathlib import Path
import subprocess
import sys

Import("env")  # type: ignore # noqa: F821


ROOT = Path(env["PROJECT_DIR"])
CODEGEN = ROOT / "scripts" / "portal_codegen.py"
HTML = ROOT / "web" / "index.html"


def main() -> int:
    if not CODEGEN.exists():
        print("[portal_codegen] skip: scripts/portal_codegen.py not found")
        return 0
    if not HTML.exists():
        print("[portal_codegen] skip: web/index.html not found")
        return 0

    cmd = [sys.executable, str(CODEGEN), "--generate"]
    print(f"[portal_codegen] running: {' '.join(cmd)}")
    return subprocess.call(cmd, cwd=str(ROOT))


if main() != 0:
    env.Exit(1)
