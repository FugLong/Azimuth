# Development

Build flags, **PlatformIO** environments, **CI**, and how the firmware repo is laid out. End-user setup and OpenTrack are in [**using-azimuth.md**](using-azimuth.md) and [**quickstart.md**](quickstart.md).

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or IDE)
- **`include/secrets.h`** — copy from **`include/secrets.h.example`** (file is gitignored). Can stay empty; optional compile-time Wi‑Fi / OpenTrack defaults when NVS has no SSID.

**Hardware profiles:** Same source, different **`board`** in PlatformIO — see **[hardware-profiles.md](hardware-profiles.md)**.

| Environment | Target |
|-------------|--------|
| **`azimuth_main_diy`**, **`azimuth_debug_diy`** | XIAO + BNO08x breakout |
| **`azimuth_main_pcb`**, **`azimuth_debug_pcb`** | Azimuth_Design PCB ([**`Azimuth_Design`**](../kicad/Azimuth_Design/)), **`esp32-c3-devkitc-02`** |

**Default release:** **`azimuth_main_diy`**. **`azimuth_debug_*`**: USB serial IMU angles only, no Wi‑Fi or portal. Full matrix: [**hardware-profiles.md**](hardware-profiles.md).

## Build and upload

**Debug (serial monitor, IMU angles):**

```bash
python3 -m platformio run -e azimuth_debug_diy -t upload
python3 -m platformio device monitor
```

**Main firmware (Hatire + optional UDP / portal):**

```bash
python3 -m platformio run -e azimuth_main_diy -t upload
```

**Azimuth_Design PCB:**

```bash
python3 -m platformio run -e azimuth_main_pcb -t upload
```

**Local web flasher bundle** (copy binaries into **`web-flasher/firmware/`** for testing GitHub Pages–style install):

```bash
pio run -e azimuth_main_diy
./scripts/prepare_web_flasher_firmware.sh
```

## Firmware version and release URLs

- **Source of truth:** repo root **`VERSION`** (first line = semver, e.g. `0.1.0`).
- **`scripts/pio_set_version.py`** injects it as **`AZIMUTH_FW_VERSION`** into **`azimuth_main_diy`** / **`azimuth_main_pcb`** builds.
- **`web-flasher/manifest.json`** field **`version`** is synced by **`scripts/sync_manifest_version.py`** (via **`prepare_web_flasher_firmware.sh`** and the GitHub Pages workflow) so the USB installer and running firmware agree.
- **Intended scheme:** align with board generations—e.g. board **0.1** ships **0.1.0**, then patch bumps until a **1.0** board ships **1.0.0**, etc.
- **Update hint on device:** After STA is up, firmware may fetch the published **`manifest.json`** once per boot (default URL in **`platformio.ini`**). If the hosted **`version`** is newer (numeric semver), the portal shows a banner linking to the USB installer. **No OTA install**—users still flash from the browser. Forks can override **`AZIMUTH_RELEASE_MANIFEST_URL`** / **`AZIMUTH_RELEASE_FLASHER_URL`**. That HTTPS client uses **certificate validation off** for this single read-only check.
- **esp-web-tools / Improv:** The web installer can show live firmware info when the device implements **Improv Serial** and a non-zero **`new_install_improv_wait_time`**. Azimuth does **not** implement Improv yet (**wait time 0**), so the page does not auto-compare the connected board to the hosted build—use the portal **Device** line and the update banner on Wi‑Fi.

## CI and browser flasher

| What | Where |
|------|--------|
| **GitLab pipeline** | Pushes to **`main`** run **`.gitlab-ci.yml`**: builds **`azimuth_main_diy`** with **`secrets.h`** copied from the example. Artifacts under **`ci-artifacts/firmware/`**. |
| **GitHub Pages USB flasher** | **`.github/workflows/github-pages-flasher.yml`** builds **`azimuth_main_diy`**, runs **`prepare_web_flasher_firmware.sh`**, syncs **`manifest.json`**, deploys **`web-flasher/`** with [esp-web-tools](https://github.com/esphome/esp-web-tools). Repo: **Settings → Pages → Source: GitHub Actions**; environment **`azimuth-flasher`**. |

Default published URLs (see **`platformio.ini`**): manifest **`https://fuglong.github.io/Azimuth/manifest.json`**, flasher **`https://fuglong.github.io/Azimuth/`**.

## Hardware pins (this revision)

Constants live in **`include/azimuth_hw.h`**. SPI map, straps, power, and optional I/O: [**wiring.md**](wiring.md). GPIO contract and PlatformIO targets: [**hardware-profiles.md**](hardware-profiles.md).

If you move SPI off **D8–D10**, call **`SPI.begin(sck, miso, mosi, -1)`** before **`imu.beginSPI(...)`** (SparkFun driver uses the already-started **`SPI`** bus on ESP32).

## Repository layout (firmware)

| Path | Role |
|------|------|
| **`src/main.cpp`** | IMU bring-up, rotation vector, Hatire + optional OpenTrack UDP; pins from **`include/azimuth_hw.h`** ([**wiring.md**](wiring.md)). |
| **`include/opentrack_pose.h`** | Fusion Euler → Hatire / UDP **Rot 0–2** with NVS axis map + per-slot invert. |
| **`src/track_network.cpp`** | Wi‑Fi, portal, UDP in **`azimuth_main_*`**; stubs in **`azimuth_debug_*`**. |
| **`src/portal_html.cpp`** | Settings UI in PROGMEM (**`azimuth_main_*`** only; filtered out of debug build). |
| **`platformio.ini`** | `espressif32`; **`azimuth_main_diy`** / **`azimuth_main_pcb`** (see [**hardware-profiles.md**](hardware-profiles.md)); SparkFun **BNO08x**; **`azimuth_main_*`** adds version script, **ArduinoJson**, default release URLs. |
| **`VERSION`** | Semver line for **`azimuth_main_*`** and **`web-flasher/manifest.json`**. |
| **`web-flasher/`** | Static USB installer + manifest for GitHub Pages. |

Planned work (board I/O, battery, enclosure, OTA, etc.) is tracked in [**roadmap.md**](roadmap.md).

---

**Related:** [README](../README.md) · [using-azimuth.md](using-azimuth.md) · [quickstart.md](quickstart.md)
