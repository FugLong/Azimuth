#!/usr/bin/env python3
"""
Azimuth USB flasher — download official firmware from GitHub Pages and flash ESP32-C3.

No PlatformIO or repo clone required. Needs Python 3.8+ and esptool (auto-installed on first run).

Examples:
  python3 scripts/azimuth-flash.py              # Custom PCB (default)
  python3 scripts/azimuth-flash.py diy
  python3 scripts/azimuth-flash.py pcb --port /dev/cu.usbmodem101
  python3 scripts/azimuth-flash.py --list-ports

One-liner (Mac/Linux):
  curl -fsSL https://raw.githubusercontent.com/FugLong/Azimuth/main/scripts/azimuth-flash.sh | bash
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

DEFAULT_BASE = "https://fuglong.github.io/Azimuth/"
TARGETS = {
    "diy": "manifest.json",
    "pcb": "manifest-pcb.json",
}
CHIP = "esp32c3"
CONNECT_RETRIES = 80
CONNECT_DELAY_S = 0.25


def eprint(*args: object) -> None:
    print(*args, file=sys.stderr)


def ensure_esptool() -> None:
    try:
        subprocess.run(
            [sys.executable, "-m", "esptool", "version"],
            check=True,
            capture_output=True,
            text=True,
        )
        return
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass

    eprint("Installing esptool (one-time)…")
    subprocess.run(
        [sys.executable, "-m", "pip", "install", "--user", "esptool"],
        check=True,
    )
    subprocess.run(
        [sys.executable, "-m", "esptool", "version"],
        check=True,
    )


def http_get(url: str) -> bytes:
    req = urllib.request.Request(url, headers={"User-Agent": "azimuth-flash/1.0"})
    with urllib.request.urlopen(req, timeout=120) as resp:
        return resp.read()


def fetch_json(url: str) -> dict:
    return json.loads(http_get(url).decode("utf-8"))


def cache_dir() -> Path:
    base = os.environ.get("XDG_CACHE_HOME")
    if base:
        root = Path(base) / "azimuth-flash"
    else:
        root = Path.home() / ".cache" / "azimuth-flash"
    root.mkdir(parents=True, exist_ok=True)
    return root


def download_part(base_url: str, part_path: str, dest: Path) -> None:
    url = f"{base_url.rstrip('/')}/{part_path.lstrip('/')}"
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.is_file() and dest.stat().st_size > 0:
        return
    eprint(f"  Downloading {part_path}…")
    data = http_get(url)
    dest.write_bytes(data)


def load_firmware(base_url: str, target: str, force_refresh: bool) -> tuple[dict, list[tuple[int, Path]]]:
    manifest_name = TARGETS[target]
    manifest_url = f"{base_url.rstrip('/')}/{manifest_name}"
    eprint(f"Fetching manifest {manifest_url}")

    try:
        manifest = fetch_json(manifest_url)
    except urllib.error.URLError as err:
        raise SystemExit(f"Could not download manifest: {err}") from err

    version = manifest.get("version", "?")
    builds = manifest.get("builds") or []
    build = next((b for b in builds if b.get("chipFamily") == "ESP32-C3"), None)
    if not build:
        raise SystemExit("Manifest has no ESP32-C3 build.")

    tag = f"{target}-{version}"
    out_dir = cache_dir() / tag
    if force_refresh and out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    manifest_path = out_dir / manifest_name
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    images: list[tuple[int, Path]] = []
    for part in build["parts"]:
        offset = int(part["offset"])
        rel = part["path"]
        name = Path(rel).name
        dest = out_dir / name
        download_part(base_url, rel, dest)
        images.append((offset, dest))

    eprint(f"Firmware {version} ({target}) ready in {out_dir}")
    return manifest, images


def list_serial_ports() -> list[str]:
    try:
        from serial.tools import list_ports
    except ImportError:
        subprocess.run(
            [sys.executable, "-m", "pip", "install", "--user", "pyserial"],
            check=True,
        )
        from serial.tools import list_ports

    esp_vids = {0x303A, 0x10C4, 0x1A86, 0x0403}
    found: list[str] = []
    for info in list_ports.comports():
        dev = info.device
        if not dev:
            continue
        vid = info.vid
        desc = (info.description or "") + (info.manufacturer or "")
        if vid in esp_vids or "usbmodem" in dev.lower() or "Espressif" in desc:
            found.append(dev)
    return found


def pick_port(explicit: str | None, ask: bool) -> str:
    if explicit:
        return explicit

    ports = list_serial_ports()
    if not ports:
        raise SystemExit(
            "No USB serial port found. Plug in the board (data cable) and try again."
        )

    if len(ports) == 1 and not ask:
        eprint(f"Using port {ports[0]}")
        return ports[0]

    # macOS: prefer call-out (cu.*) device for upload
    cu_ports = [p for p in ports if "/cu." in p or p.upper().startswith("COM")]
    if len(cu_ports) == 1 and not ask:
        eprint(f"Using port {cu_ports[0]}")
        return cu_ports[0]

    eprint("Multiple serial ports:")
    for i, p in enumerate(ports, start=1):
        eprint(f"  [{i}] {p}")
    while True:
        try:
            choice = input(f"Pick port [1-{len(ports)}]: ").strip()
        except (EOFError, KeyboardInterrupt):
            raise SystemExit("\nCancelled.") from None
        if choice.isdigit() and 1 <= int(choice) <= len(ports):
            return ports[int(choice) - 1]
        eprint("Invalid choice.")


def run_esptool(args: list[str]) -> None:
    cmd = [sys.executable, "-m", "esptool", *args]
    subprocess.run(cmd, check=True)


def flash_images(port: str, images: list[tuple[int, Path]], erase: bool) -> None:
    write_args = ["--chip", CHIP, "--port", port, "--baud", "460800"]
    if erase:
        eprint("Erasing flash…")
        for attempt in range(CONNECT_RETRIES):
            try:
                run_esptool([*write_args, "erase-flash"])
                break
            except subprocess.CalledProcessError:
                if attempt + 1 >= CONNECT_RETRIES:
                    raise
                time.sleep(CONNECT_DELAY_S)

    flash_cmd = [*write_args, "write-flash", "--flash-mode", "dio", "--flash-freq", "80m", "--flash-size", "4MB"]
    for offset, path in images:
        flash_cmd.append(f"0x{offset:X}")
        flash_cmd.append(str(path))

    eprint("Writing firmware (retrying while board boots)…")
    for attempt in range(CONNECT_RETRIES):
        try:
            run_esptool(flash_cmd)
            eprint("Done.")
            return
        except subprocess.CalledProcessError:
            if attempt + 1 >= CONNECT_RETRIES:
                raise SystemExit(
                    "Flash failed — could not stay connected to the bootloader.\n"
                    "Try: hold BOOT/GPIO9, plug USB, run this command again immediately."
                ) from None
            if attempt % 8 == 0:
                eprint("  Waiting for ROM bootloader…")
            time.sleep(CONNECT_DELAY_S)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="Flash Azimuth firmware (ESP32-C3) from the official GitHub Pages build.",
    )
    p.add_argument(
        "target",
        nargs="?",
        default="pcb",
        choices=sorted(TARGETS),
        help="Hardware profile: diy (XIAO) or pcb (custom Azimuth board). Default: pcb",
    )
    p.add_argument("--port", "-p", help="Serial port (e.g. /dev/cu.usbmodem101, COM3)")
    p.add_argument("--ask", action="store_true", help="Always ask which port to use")
    p.add_argument("--list-ports", action="store_true", help="List likely ESP serial ports and exit")
    p.add_argument(
        "--base-url",
        default=os.environ.get("AZIMUTH_FLASH_BASE_URL", DEFAULT_BASE),
        help=f"Firmware host (default: {DEFAULT_BASE})",
    )
    p.add_argument("--refresh", action="store_true", help="Re-download firmware files")
    p.add_argument("--no-erase", action="store_true", help="Skip erase (not recommended for blank boards)")
    return p.parse_args()


def main() -> None:
    args = parse_args()

    if args.list_ports:
        ports = list_serial_ports()
        if not ports:
            eprint("No matching ports found.")
            sys.exit(1)
        for port in ports:
            print(port)
        return

    ensure_esptool()

    eprint("Azimuth flasher")
    eprint("  Profile: " + ("DIY (XIAO)" if args.target == "diy" else "Custom PCB"))
    eprint("  Source:  " + args.base_url.rstrip("/"))

    manifest, images = load_firmware(args.base_url, args.target, args.refresh)
    version = manifest.get("version", "?")
    eprint(f"  Version: {version}")

    port = pick_port(args.port, args.ask)
    flash_images(port, images, erase=not args.no_erase)

    eprint()
    eprint("Flash complete. Board will reboot.")
    eprint("  Wi‑Fi setup: join Azimuth-Tracker, open http://192.168.4.1")
    eprint("  Docs: https://github.com/FugLong/Azimuth/blob/main/docs/quickstart.md")


if __name__ == "__main__":
    main()
