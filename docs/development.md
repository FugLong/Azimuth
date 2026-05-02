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

**One-liner** (from repo root; defaults to **`azimuth_main_pcb`**, auto-picks a USB serial port; use `diy` for XIAO or pass a port as the second argument):

```bash
./scripts/flash.sh              # PCB
./scripts/flash.sh diy          # DIY
./scripts/flash.sh pcb --ask    # pick port if several devices
```

The script uses **`pio`** when it is on your **`PATH`**, otherwise **`python3 -m platformio`**.

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

**Local web flasher bundle** (copy binaries into **`web-flasher/firmware/{diy,pcb}/`** for testing GitHub Pages–style install):

```bash
pio run -e azimuth_main_diy
pio run -e azimuth_main_pcb
./scripts/prepare_web_flasher_firmware.sh azimuth_main_diy web-flasher/firmware/diy
./scripts/prepare_web_flasher_firmware.sh azimuth_main_pcb web-flasher/firmware/pcb
```

## Firmware version and release URLs

- **Source of truth:** repo root **`VERSION`** (first line = semver, e.g. `0.2.0`).
- **`scripts/pio_set_version.py`** injects it as **`AZIMUTH_FW_VERSION`** into **`azimuth_main_diy`** / **`azimuth_main_pcb`** builds.
- **`web-flasher/manifest.json`** and **`web-flasher/manifest-pcb.json`** fields **`version`** are synced by **`scripts/sync_manifest_version.py`** (via **`prepare_web_flasher_firmware.sh`** and the GitHub Pages workflow) so the USB installer and running firmware agree.
- **Intended scheme:** align with board generations—e.g. board **0.2** ships **0.2.0**, then patch bumps until a **1.0** board ships **1.0.0**, etc.
- **Update hint on device:** After STA is up, firmware may fetch the published **`manifest.json`** once per boot (default URL in **`platformio.ini`**). If the hosted **`version`** is newer (numeric semver), the portal shows a banner linking to the USB installer. **No OTA install**—users still flash from the browser. Forks can override **`AZIMUTH_RELEASE_MANIFEST_URL`** / **`AZIMUTH_RELEASE_FLASHER_URL`**. That HTTPS client uses **certificate validation off** for this single read-only check.
- **esp-web-tools / Improv:** The web installer can show live firmware info when the device implements **Improv Serial** and a non-zero **`new_install_improv_wait_time`**. Azimuth does **not** implement Improv yet (**wait time 0**), so the page does not auto-compare the connected board to the hosted build—use the portal **Device** line and the update banner on Wi‑Fi.

## CI and browser flasher

| What | Where |
|------|--------|
| **GitLab pipeline** | Pushes to **`main`** run **`.gitlab-ci.yml`**: builds **`azimuth_main_diy`** with **`secrets.h`** copied from the example. Artifacts under **`ci-artifacts/firmware/`**. |
| **GitHub Pages USB flasher** | **`.github/workflows/github-pages-flasher.yml`** builds **`azimuth_main_diy`** + **`azimuth_main_pcb`**, runs **`prepare_web_flasher_firmware.sh`** for both targets, syncs **`manifest.json`** + **`manifest-pcb.json`**, deploys **`web-flasher/`** with [esp-web-tools](https://github.com/esphome/esp-web-tools). Repo: **Settings → Pages → Source: GitHub Actions**; environment **`azimuth-flasher`**. |

Default published URLs (see **`platformio.ini`**): manifest **`https://fuglong.github.io/Azimuth/manifest.json`**, flasher **`https://fuglong.github.io/Azimuth/`**.

## Hardware pins (this revision)

Constants live in **`include/azimuth_hw.h`**. SPI map, straps, power, and optional I/O: [**wiring.md**](wiring.md). GPIO contract and PlatformIO targets: [**hardware-profiles.md**](hardware-profiles.md).

**`src/main.cpp`** always calls **`SPI.begin(8, 9, 10, -1)`** (via **`azimuth_hw`**) before **`imu.beginSPI(...)`** because the SparkFun driver’s **`beginSPI`** invokes **`SPI.begin()`** with no pin list. The XIAO variant already defaults to **8/9/10**, but **`esp32-c3-devkitc-02`** (Azimuth PCB profile) defaults to **4/5/6**, which would clash with **CS/INT** on **5/6** and break the IMU.

## Repository layout (firmware)

| Path | Role |
|------|------|
| **`src/main.cpp`** | IMU bring-up, rotation vector, Hatire + optional OpenTrack UDP; pins from **`include/azimuth_hw.h`** ([**wiring.md**](wiring.md)). |
| **`include/opentrack_pose.h`** | Fusion Euler → Hatire / UDP **Rot 0–2** with NVS axis map + per-slot invert. |
| **`src/track_network.cpp`** | Wi‑Fi, portal, UDP in **`azimuth_main_*`**; stubs in **`azimuth_debug_*`**. |
| **`src/portal_html.cpp`** | Settings UI in PROGMEM (**`azimuth_main_*`** only; filtered out of debug build). |
| **`platformio.ini`** | `espressif32`; **`azimuth_main_diy`** / **`azimuth_main_pcb`** (see [**hardware-profiles.md**](hardware-profiles.md)); SparkFun **BNO08x**; **`azimuth_main_*`** adds version script, **ArduinoJson**, default release URLs. |
| **`VERSION`** | Semver line for **`azimuth_main_*`**, **`web-flasher/manifest.json`**, and **`web-flasher/manifest-pcb.json`**. |
| **`web-flasher/`** | Static USB installer + manifest for GitHub Pages. |

Planned work (board I/O, battery, enclosure, OTA, etc.) is tracked in [**roadmap.md**](roadmap.md).

---

**Related:** [README](../README.md) · [using-azimuth.md](using-azimuth.md) · [quickstart.md](quickstart.md)
