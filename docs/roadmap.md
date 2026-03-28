# Azimuth — roadmap & progress (toward V1)

This document tracks **estimated** progress and planned work toward a **V1** release: stable hardware, enclosure, and firmware that meets the goals in the main README. Percentages are manual snapshots—update them when milestones land.

---

## Snapshot (workstreams)

| Workstream | Progress | Notes |
|------------|----------|-------|
| Hardware selection | **100%** | ESP32-C3 XIAO + BNO086-class IMU, etc. |
| Custom PCB (KiCad) | **~95%** | Layout + nets aligned with [wiring.md](wiring.md); DRC/ERC + BOM lock before fab; **panelization** remaining for production ordering |
| Firmware | **60–70%** | SPI IMU; **`azimuth_debug`** bring-up; **`azimuth_main`**: Hatire + Wi‑Fi UDP, **NVS** portal, **Azimuth-Setup**, **VERSION** / web flasher / update banner. Still ahead: board I/O, battery ADC, OTA, modular `imu/` / `io/` refactor (Phase 1 table below). |
| 3D enclosure | **0%** | Not started |
| End-user docs & release | **~50%** | README + **[quickstart.md](quickstart.md)** + **[using-azimuth.md](using-azimuth.md)**; **`VERSION`** / USB flasher / portal banner documented; OTA TBD |

**Visual (rough):**

```
Hardware/BOM     [████████████████████] 100%
PCB design       [███████████████████░] ~95%
Firmware         [█████████████░░░░░░░] 60–70%
Enclosure        [░░░░░░░░░░░░░░░░░░░░] 0%
```

---

## V1 product posture

**Wireless is a V1 requirement** — The tracker is meant to run untethered with a battery; **WiFi → UDP (OpenTrack)** is implemented in firmware for LAN use; **USB stays supported** for desk use, flashing, serial debug, and **Hatire** as an alternative input path.

**Enclosure** — Default plan is **one enclosure** sized for the **battery** build (internal volume for cell + PH connector clearance). A **second, slimmer** enclosure for **wired-only** builds is possible if there is demand: same PCB, but without a battery installed and ideally without the tall **JST PH2.0** connector populated (see assembly note below).

---

## V1 — what “done” means (draft)

Use this as a checklist; tighten or relax before tagging V1.

- [ ] **PCB** — Fabrication-ready: DRC clean, BOM fixed, assembly tested on at least one revision.
- [ ] **Bring-up** — IMU, USB serial, Hatire, and WiFi/UDP verified on the **actual** PCB (breadboard path already exercised).
- [ ] **Firmware core** — Clear module boundaries (HAL / drivers / app), pins and features match [wiring.md](wiring.md).
- [ ] **I/O** — Status LED, **FUNC1** button, buzzer as on PCB; debouncing and behavior documented.
- [ ] **Battery** — Voltage readout (divider on D0/GPIO2), low-battery indication or policy (thresholds TBD).
- [x] **Wireless (MVP)** — WiFi STA + OpenTrack **UDP over network** working (credentials via **NVS portal** and optional **`secrets.h`**).
- [x] **Wireless (product)** — On-device settings: **HTTP UI** + **NVS** (`Preferences`), `secrets.h` fallback, provisioning AP + captive portal when SSID missing or STA fails ([README](../README.md), [using-azimuth.md](using-azimuth.md)). Still open: **richer reconnection** / backoff policy, schema **versioning** (see Phase 3).
- [x] **Updates (USB)** — GitHub Pages **esp-web-tools** flasher + repo **`VERSION`** / **`manifest.json`**; portal banner compares to hosted manifest (no OTA).
- [ ] **Updates (OTA)** — Optional later; not required for current posture.
- [ ] **Enclosure** — At least the **battery** reference enclosure; optional second slim shell documented if wired-only variant is offered.
- [ ] **User-facing docs** — Build, flash, OpenTrack connection (USB + wireless), troubleshooting.

---

## Phase 1 — I/O, architecture, power

**Goal:** Firmware matches the board: modular structure, all GPIO roles implemented, battery monitoring.

| Task | Status |
|------|--------|
| Pin map / config header (single source of truth vs [wiring.md](wiring.md)) | ⬜ |
| LED: patterns for status (boot, tracking, error, low battery) | ⬜ |
| Buttons: recenter / func; interrupts or polled + debounce | ⬜ |
| Buzzer: tones for feedback (optional minimal set for V1) | ⬜ |
| ADC: battery voltage, calibration constants, filtering | ⬜ |
| Power / charging behavior documented (what XIAO handles vs firmware) | ⬜ |
| Refactor `main.cpp` into layers (e.g. `imu/`, `io/`, `platform/`) without changing behavior | ⬜ |

---

## Phase 2 — WiFi + tracking path

**Goal:** Primary use case is **untethered / battery** operation: a stable wireless path for orientation to a host. **USB Hatire** remains supported in parallel.

| Task | Status |
|------|--------|
| WiFi STA | ✅ (UDP client to PC; credentials via NVS + `include/secrets.h`) |
| WiFi AP / onboarding portal | ✅ (`Azimuth-Setup`, captive DNS + HTTP :80; recovery if STA fails) |
| Transport: OpenTrack **UDP** (6× `double`, default port 4242) | ✅ |
| Semantics aligned with Hatire (yaw / pitch sign / roll) | ✅ (`opentrackMapEulerToRot` + NVS axis map in `include/opentrack_pose.h`) |
| Coexistence with USB (Hatire + UDP same build) | ✅ |
| Security baseline (WiFi credentials **not** in git) | 🟨 (`secrets.h` local; NVS on device; migration / hardening in Phase 3) |

---

## Phase 3 — Easy flashing & settings from the web

**Goal:** Users can install or update firmware and manage settings without a full dev environment—ideally tied to GitHub releases or a small official tool.

| Task | Status |
|------|--------|
| Define artifact: release binaries + manifest/versioning | ✅ ( **`VERSION`**, **`web-flasher/manifest.json`**, CI sync, semver in firmware) |
| Web-based flasher (e.g. **esp-web-tools** / browser serial) or documented one-click flow | ✅ ( **`.github/workflows/github-pages-flasher.yml`**, **`web-flasher/`** ) |
| On-device “newer build” hint | ✅ (HTTPS **`manifest.json`** once per STA boot; portal banner; no OTA) |
| Settings: schema, storage (NVS), migration between versions | 🟨 (NVS keys in use; **versioned schema** / migration ⬜) |
| Document recovery if flash fails (USB bootloader, button combo, etc.) | 🟨 (README bootloader note; expand if needed) |

---

## Phase 4 — On-device web / phone UX (optional for V1)

**Goal:** Configure WiFi, calibration shortcuts, and modes from a phone or laptop without custom desktop software—**if** it fits V1 scope.

| Task | Status |
|------|--------|
| Captive portal or small HTTP server on ESP (when in AP or dual mode) | ✅ (AP **:80** + STA **:8080**; same UI from `src/portal_html.cpp` in **`azimuth_main`**) |
| **Or** BLE GATT for lightweight config (often nicer for phones; more firmware work) | ⬜ |
| Same settings backend as Phase 3 (one model, multiple UIs) | 🟨 (HTTP uses same NVS namespace; BLE not started) |

*Scope note:* V1 already assumes **WiFi streaming** (Phase 2). Optional BLE / calibration UX can slip to **V1.1** if needed.

---

## Parking lot — ideas to consider (not all V1)

These are common for head trackers; pick what matches your audience.

| Idea | Why |
|------|-----|
| **Calibration / recenter UX** | Beyond one button: axis remap, smoothing presets, saved profiles. |
| **Drift / filter tuning** | Document OpenTrack filter settings; optional on-device presets. |
| **USB + WiFi simultaneously** | Useful for debug; define which path OpenTrack uses. |
| **CI** | Build firmware on every push; optional hardware-in-the-loop later. |
| **Signed OTA** | If OTA ships, plan keys and rollback. |
| **Desktop companion** | Small app for WiFi discovery—only if browser tooling is not enough. |
| **GitHub-hosted web flasher** | ✅ Shipped: **`web-flasher/`** + GitHub Actions; parking-lot ideas (WebRTC LAN hints, etc.) remain optional. |
| **Privacy / safety** | No cloud requirement; local network only—stated in README overview. |

---

## Changelog (manual)

| Date | Update |
|------|--------|
| 2026-03-22 | Initial roadmap: workstream snapshot + phases 1–4 + V1 draft criteria. |
| 2026-03-22 | V1 posture: wireless required; USB supported; enclosure + PH2 assembly tradeoff documented. |
| 2026-03-25 | Docs + BOM aligned with KiCad (**PWR1**, **`vcc`**, **BUZZER1**, **MLT-5020**, **FUNC1** footprint). |
| 2026-03-27 | Firmware ~30%: Hatire + WiFi/OpenTrack UDP; `secrets.h`; PCB ~95% with **panelization** left before fab. |
| 2026-03-27 | Firmware ~40%: NVS + HTTP settings, provisioning portal; roadmap/tasks aligned; shared OpenTrack axis helper; portal HTML split to `portal_html.cpp`. |
| 2026-03-28 | PlatformIO envs **`azimuth_main`** / **`azimuth_debug`**; GitLab CI + GitHub Pages flasher; NVS portal axis map; **`VERSION`** / manifest sync; update banner. Docs split: README for overview; **[using-azimuth.md](using-azimuth.md)**; **[development.md](development.md)**. |
| 2026-03-28 | Firmware snapshot toward V1 revised to **60–70%** (core tracking + wireless + settings + release path in place; I/O, battery, OTA, refactor still open). |
