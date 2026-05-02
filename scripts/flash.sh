#!/usr/bin/env bash
set -euo pipefail

# One-command build + flash helper for Azimuth.
# Usage:
#   ./scripts/flash.sh                 # default: azimuth_main_pcb, auto port
#   ./scripts/flash.sh diy             # use azimuth_main_diy
#   ./scripts/flash.sh pcb /dev/cu.usbmodem1101
#   ./scripts/flash.sh azimuth_main_pcb /dev/cu.usbmodem1101
#   ./scripts/flash.sh --ask           # force interactive port picker

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

ASK_PORT=0
POSITIONAL=()
for arg in "$@"; do
  case "$arg" in
    --ask) ASK_PORT=1 ;;
    *)
      POSITIONAL+=("$arg")
      ;;
  esac
done

TARGET="${POSITIONAL[0]:-pcb}"
PORT="${POSITIONAL[1]:-}"

case "$TARGET" in
  pcb) ENV_NAME="azimuth_main_pcb" ;;
  diy) ENV_NAME="azimuth_main_diy" ;;
  azimuth_main_pcb|azimuth_main_diy|azimuth_debug_pcb|azimuth_debug_diy) ENV_NAME="$TARGET" ;;
  *)
    echo "Unknown target '$TARGET'. Use: pcb | diy | azimuth_*"
    exit 2
    ;;
esac

if command -v pio >/dev/null 2>&1; then
  PIO=(pio)
elif python3 -m platformio --version >/dev/null 2>&1; then
  PIO=(python3 -m platformio)
else
  echo "PlatformIO not found. Install it first:"
  echo "  python3 -m pip install --user platformio"
  exit 1
fi

if [[ ! -f "include/secrets.h" && -f "include/secrets.h.example" ]]; then
  cp include/secrets.h.example include/secrets.h
fi

detect_ports() {
  local detected=()
  local p
  for p in /dev/cu.usbmodem* /dev/tty.usbmodem* /dev/ttyACM* /dev/ttyUSB*; do
    [[ -e "$p" ]] && detected+=("$p")
  done
  printf "%s\n" "${detected[@]:-}"
}

if [[ -z "$PORT" ]]; then
  PORTS=()
  while IFS= read -r line; do
    [[ -n "$line" ]] && PORTS+=("$line")
  done < <(detect_ports)
  if [[ "${#PORTS[@]}" -eq 0 ]]; then
    echo "No serial port detected."
    echo "Plug in the board and re-run, or pass a port manually:"
    echo "  ./scripts/flash.sh $TARGET /dev/cu.usbmodem1101"
    exit 1
  elif [[ "${#PORTS[@]}" -eq 1 ]]; then
    PORT="${PORTS[0]}"
  elif [[ "$ASK_PORT" -eq 0 ]]; then
    # Fast path: prefer native ESP USB CDC names, avoid interactive pause.
    for p in "${PORTS[@]}"; do
      case "$p" in
        /dev/cu.usbmodem*|/dev/tty.usbmodem*|/dev/ttyACM*)
          PORT="$p"
          break
          ;;
      esac
    done
    if [[ -z "$PORT" ]]; then
      PORT="${PORTS[0]}"
    fi
    echo "Auto-selected serial port: $PORT (use --ask to choose manually)"
  else
    echo "Multiple serial ports found:"
    for i in "${!PORTS[@]}"; do
      printf "  [%d] %s\n" "$((i + 1))" "${PORTS[$i]}"
    done
    printf "Pick a port [1-%d]: " "${#PORTS[@]}"
    read -r choice
    if ! [[ "$choice" =~ ^[0-9]+$ ]] || (( choice < 1 || choice > ${#PORTS[@]} )); then
      echo "Invalid choice."
      exit 1
    fi
    PORT="${PORTS[$((choice - 1))]}"
  fi
fi

echo "Environment: $ENV_NAME"
echo "Port:        $PORT"
echo "Starting build + upload..."

"${PIO[@]}" run -e "$ENV_NAME" -t upload --upload-port "$PORT"

echo
echo "Flash complete."
