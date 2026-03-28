# Development

Build flags, **PlatformIO** environments, **CI**, and how the firmware repo is laid out. End-user setup and OpenTrack are in [**using-azimuth.md**](using-azimuth.md) and [**quickstart.md**](quickstart.md).

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or IDE)
- **`include/secrets.h`** — copy from **`include/secrets.h.example`** (file is gitignored). Can stay empty; optional compile-time Wi‑Fi / OpenTrack defaults when NVS has no SSID.

**Default build environment:** **`azimuth_main`** (release: Hatire + optional Wi‑Fi / portal / UDP). Use **`azimuth_debug`** for **USB serial only** (yaw / pitch / roll), no Wi‑Fi or portal.

## Build and upload

**Debug (serial monitor, IMU angles):**

```bash
python3 -m platformio run -e azimuth_debug -t upload
python3 -m platformio device monitor
```

**Main firmware (Hatire + optional UDP / portal):**

```bash
python3 -m platformio run -e azimuth_main -t upload
```

**Local web flasher bundle** (copy binaries into **`web-flasher/firmware/`** for testing GitHub Pages–style install):

```bash
pio run -e azimuth_main
./scripts/prepare_web_flasher_firmware.sh
```

## Firmware version and release URLs

- **Source of truth:** repo root **`VERSION`** (first line = semver, e.g. `0.1.0`).
- **`scripts/pio_set_version.py`** injects it as **`AZIMUTH_FW_VERSION`** into **`azimuth_main`** builds.
- **`web-flasher/manifest.json`** field **`version`** is synced by **`scripts/sync_manifest_version.py`** (via **`prepare_web_flasher_firmware.sh`** and the GitHub Pages workflow) so the USB installer and running firmware agree.
- **Intended scheme:** align with board generations—e.g. board **0.1** ships **0.1.0**, then patch bumps until a **1.0** board ships **1.0.0**, etc.
- **Update hint on device:** After STA is up, firmware may fetch the published **`manifest.json`** once per boot (default URL in **`platformio.ini`**). If the hosted **`version`** is newer (numeric semver), the portal shows a banner linking to the USB installer. **No OTA install**—users still flash from the browser. Forks can override **`AZIMUTH_RELEASE_MANIFEST_URL`** / **`AZIMUTH_RELEASE_FLASHER_URL`**. That HTTPS client uses **certificate validation off** for this single read-only check.
- **esp-web-tools / Improv:** The web installer can show live firmware info when the device implements **Improv Serial** and a non-zero **`new_install_improv_wait_time`**. Azimuth does **not** implement Improv yet (**wait time 0**), so the page does not auto-compare the connected board to the hosted build—use the portal **Device** line and the update banner on Wi‑Fi.

## CI and browser flasher

| What | Where |
|------|--------|
| **GitLab pipeline** | Pushes to **`main`** run **`.gitlab-ci.yml`**: builds **`azimuth_main`** with **`secrets.h`** copied from the example. Artifacts under **`ci-artifacts/firmware/`**. |
| **GitHub Pages USB flasher** | **`.github/workflows/github-pages-flasher.yml`** builds **`azimuth_main`**, runs **`prepare_web_flasher_firmware.sh`**, syncs **`manifest.json`**, deploys **`web-flasher/`** with [esp-web-tools](https://github.com/esphome/esp-web-tools). Repo: **Settings → Pages → Source: GitHub Actions**; environment **`azimuth-flasher`**. |

Default published URLs (see **`platformio.ini`**): manifest **`https://fuglong.github.io/Azimuth/manifest.json`**, flasher **`https://fuglong.github.io/Azimuth/`**.

## Hardware pins (this revision)

**MCU:** Seeed **XIAO ESP32-C3**. **IMU:** **BNO086** on the custom PCB (or **BNO08x** breakout in **SPI** mode with **PS0** / **PS1** → **3.3 V**).

Summary SPI map (full table, power, buttons, LED, buzzer in [**wiring.md**](wiring.md)):

| Signal | XIAO pin | GPIO |
|--------|----------|------|
| SCK | D8 | 8 |
| MISO | D9 | 9 |
| MOSI | D10 | 10 |
| CS | D3 | 5 |
| H_INT | D4 | 6 |
| PS0 / WAKE | D2 | 4 |
| NRST | D7 | 20 |

If you move SPI off **D8–D10**, call **`SPI.begin(sck, miso, mosi, -1)`** before **`imu.beginSPI(...)`** so the bus matches your wiring (the SparkFun driver uses the already-started **`SPI`** bus on ESP32).

## Repository layout (firmware)

| Path | Role |
|------|------|
| **`src/main.cpp`** | IMU bring-up, rotation vector, Hatire + optional OpenTrack UDP; pins match **ESP32_BNO086** PCB ([**wiring.md**](wiring.md)). |
| **`include/opentrack_pose.h`** | Fusion Euler → Hatire / UDP **Rot 0–2** with NVS axis map + per-slot invert. |
| **`src/track_network.cpp`** | Wi‑Fi, portal, UDP in **`azimuth_main`**; stubs in **`azimuth_debug`**. |
| **`src/portal_html.cpp`** | Settings UI in PROGMEM (**`azimuth_main`** only; filtered out of debug build). |
| **`platformio.ini`** | `espressif32`, `seeed_xiao_esp32c3`, SparkFun **BNO08x**; **`azimuth_main`** adds version script, **ArduinoJson**, default release URLs. |
| **`VERSION`** | Semver line for **`azimuth_main`** and **`web-flasher/manifest.json`**. |
| **`web-flasher/`** | Static USB installer + manifest for GitHub Pages. |

Planned work (board I/O, battery, enclosure, OTA, etc.) is tracked in [**roadmap.md**](roadmap.md).

---

**Related:** [README](../README.md) · [using-azimuth.md](using-azimuth.md) · [quickstart.md](quickstart.md)
