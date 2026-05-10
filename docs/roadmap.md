# Azimuth — roadmap & progress (toward V1)

This document tracks **estimated** progress and planned work toward a **V1** release: stable hardware, enclosure, and firmware that meets the goals in the main README. Percentages are manual snapshots—update them when milestones land.

---

## Snapshot (workstreams)

| Workstream | Progress | Notes |
|------------|----------|-------|
| Hardware selection | **100%** | DIY: **XIAO ESP32-C3** + **BNO08x**; product: **Azimuth** PCB ([**Azimuth_Design**](../kicad/Azimuth_Design/)) |
| Custom PCB — **V0.1** | **100%** (design) · **assembled units received** | **Azimuth_Design**: layout + nets per [wiring.md](wiring.md); **ERC and DRC clean**. **V0.1** fab complete; **boards in hand**; **bring-up / testing in progress**. **Panelization** still optional for future arrays. |
| Firmware | **~90%** | **`azimuth_main_*`**: Hatire + Wi‑Fi UDP, **NVS** portal, **Offline Mode AP**, power/thermal, **pause/stasis**, **wireless OTA**, modular networking (`track_network_*.cpp`), LED policy + portal. Polish: **battery ADC** refinement, optional **`main.cpp`** split, NVS schema versioning (Phase 3). |
| 3D enclosure | **~90%+** | **Design nearly complete** — [**Onshape document**](https://cad.onshape.com/documents/fa78666ff0e219ba32d45d9f/w/679625cc5ba48f8e098cba83/e/0cd4f9cad8e872128d491b46) (interactive CAD); print / fit validation still ahead |
| End-user docs & release | **~85%** | **[user-guide.md](user-guide.md)** (manual) + **[quickstart.md](quickstart.md)** + **[using-azimuth.md](using-azimuth.md)**; **`VERSION`** / USB flasher / portal / OTA documented; enclosure assembly + field troubleshooting still thin |

**Visual (rough):**

```
Hardware/BOM     [████████████████████] 100%
PCB design       [████████████████████] 100%  (V0.1 · boards received · testing)
Firmware         [██████████████████░░] ~90%
Enclosure        [██████████████████░░] ~90%+ (design)
```

---

## V1 product posture

**Wireless is a V1 requirement** — The tracker is meant to run untethered with a battery; **WiFi → UDP (OpenTrack)** is implemented in firmware for LAN use; **USB stays supported** for desk use, flashing, serial debug, and **Hatire** as an alternative input path.

**Enclosure** — Default plan is **one enclosure** sized for the **battery** build (internal volume for cell + PH connector clearance). **CAD:** [Onshape — Azimuth enclosure](https://cad.onshape.com/documents/fa78666ff0e219ba32d45d9f/w/679625cc5ba48f8e098cba83/e/0cd4f9cad8e872128d491b46) (open in a browser for the live viewer). A **second, slimmer** enclosure for **wired-only** builds is possible if there is demand: same PCB, but without a battery installed and ideally without the tall **JST PH2.0** connector populated (see assembly note below).

---

## V1 — what “done” means (draft)

Use this as a checklist; tighten or relax before tagging V1.

- [x] **PCB (KiCad)** — **Azimuth_Design** fabrication-ready: **ERC and DRC clean**; BOM matches schematic/layout (re-export from KiCad for each order — [parts-list.md](parts-list.md), [kicad.md](kicad.md)).
- [x] **PCB (V0.1 manufacturing)** — **V0.1** design locked; fab **complete**; **assembled boards received** (2026-05).
- [ ] **PCB (bring-up)** — **In progress:** validate IMU, USB serial, Hatire, and Wi‑Fi/UDP on **production** boards (breadboard path already exercised).
- [x] **Firmware core (tracking + wireless)** — Networking split into `track_network_*.cpp`; pins and features match [wiring.md](wiring.md). Optional: further `imu/` / `platform/` layering ([Phase 1](#phase-1--io-architecture-power) table).
- [ ] **I/O** — Status LED, **FUNC1** button, buzzer as on PCB; debouncing and **product** behavior documented (bring-up demos exist in firmware).
- [x] **Battery** — Voltage readout + curve-based **%** estimate (`battery_monitor`); portal capacity / cal offset; stepped alerts (~25% downward); **≤1%** emergency cues + Wi‑Fi cut; **≤15%** blocks wireless OTA off-charger; FUNC pause toggle suppressed **≤1%** — documented in **[user-guide.md](user-guide.md)**. Residual: ADC tuning / “bench your pack” refinement.
- [x] **Wireless (MVP)** — WiFi STA + OpenTrack **UDP over network** working (credentials via **NVS portal** and optional **`secrets.h`**).
- [x] **Wireless (product)** — On-device settings: **HTTP UI** + **NVS** (`Preferences`), `secrets.h` fallback, provisioning AP + captive portal when SSID missing or STA fails ([README](../README.md), [using-azimuth.md](using-azimuth.md)). Still open: **richer reconnection** / backoff policy, schema **versioning** (see Phase 3).
- [x] **Updates (USB)** — GitHub Pages **esp-web-tools** flasher + repo **`VERSION`** / **`manifest.json`**; portal banner compares to hosted manifest.
- [x] **Updates (OTA)** — **`track_update`** streams `firmware/<board>/firmware.bin` from the same trusted GitHub Pages release (Let's Encrypt root pinned) into the standby `ota_0`/`ota_1` slot via the Arduino `Update` class, then reboots into it. Triggered by **FUNC long-press (~2 s)** or the portal **“Install over Wi‑Fi”** button (banner + Device card). Refuses to start in Offline‑Mode AP, during thermal hold, or with battery ≤15 % off‑charger; USB flasher remains recovery.
- [ ] **Enclosure** — At least the **battery** reference enclosure; optional second slim shell documented if wired-only variant is offered. **CAD:** [Onshape document](https://cad.onshape.com/documents/fa78666ff0e219ba32d45d9f/w/679625cc5ba48f8e098cba83/e/0cd4f9cad8e872128d491b46). **Design ~90%+ complete** (2026-05); verify fit & manufacturing next.
- [x] **User-facing docs (baseline)** — **[user-guide.md](user-guide.md)** manual + existing guides; deeper troubleshooting + enclosure still expanding.

---

## Phase 1 — I/O, architecture, power

**Goal:** Firmware matches the board: modular structure, all GPIO roles implemented, battery monitoring.

| Task | Status |
|------|--------|
| Pin map / config header (single source of truth vs [wiring.md](wiring.md)) | In progress: **`include/azimuth_hw.h`** + [hardware-profiles.md](hardware-profiles.md); add alternate defines only if a PCB GPIO map diverges |
| LED: patterns for status, **stasis (pause)**, setup, battery, thermal | 🟩 **`io_led_policy`** + NVS **`led_mode`** · thermal/battery/setup/stasis layered |
| Buttons: **FUNC = pause/stasis** (UDP off + low power + cues); long-press **OTA** | 🟩 single-tap pause · double reserved · long-hold (~2 s) triggers Wi‑Fi OTA |
| Buzzer: tones for feedback (optional minimal set for V1) | 🟨 (FUNC chime on PCB; wider set ⬜) |
| ADC: battery voltage, calibration constants, filtering | 🟩 implemented · calibration polish optional |
| Power / charging behavior documented (what XIAO handles vs firmware) | ⬜ |
| Refactor `main.cpp` into layers (e.g. `imu/`, `io/`, `platform/`) without changing behavior | ⬜ |

---

## Phase 2 — WiFi + tracking path

**Goal:** Primary use case is **untethered / battery** operation: a stable wireless path for orientation to a host. **USB Hatire** remains supported in parallel.

| Task | Status |
|------|--------|
| WiFi STA | ✅ (UDP client to PC; credentials via NVS + `include/secrets.h`) |
| WiFi AP / onboarding portal | ✅ (`Azimuth-Tracker` Offline Mode AP, captive DNS + HTTP :80; recovery if STA fails) |
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
| On-device “newer build” hint | ✅ (HTTPS **`manifest.json`** after STA associate + backoff on failure; portal banner — install is still **user-triggered** USB or Wi‑Fi OTA) |
| Settings: schema, storage (NVS), migration between versions | 🟨 (NVS keys in use; **versioned schema** / migration ⬜) |
| Document recovery if flash fails (USB bootloader, button combo, etc.) | 🟨 (README bootloader note; expand if needed) |

---

## Phase 4 — On-device web / phone UX (optional for V1)

**Goal:** Configure WiFi, calibration shortcuts, and modes from a phone or laptop without custom desktop software—**if** it fits V1 scope.

| Task | Status |
|------|--------|
| Captive portal or small HTTP server on ESP (when in AP or dual mode) | ✅ (AP **:80** + STA **:8080**; same UI from `src/portal_html.cpp` in **`azimuth_main_*`**) |
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
| **Signed OTA** | OTA ships unsigned today; plan ECDSA / Espressif secure boot keys + rollback for a future hardening pass. |
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
| 2026-04-02 | Docs + BOM: buzzer path **Q2** **AO3400A**, **D2** **B5819WS**, **R20**/**R21** gate network; **parts-list** / **wiring** / **hardware-profiles** / **kicad** / **README** / **easyeda2kicad** README. |
| 2026-03-27 | Firmware ~30%: Hatire + WiFi/OpenTrack UDP; `secrets.h`; PCB ~95% with **panelization** left before fab. |
| 2026-03-27 | Firmware ~40%: NVS + HTTP settings, provisioning portal; roadmap/tasks aligned; shared OpenTrack axis helper; portal HTML split to `portal_html.cpp`. |
| 2026-03-28 | PlatformIO envs **`azimuth_main_diy`** / **`azimuth_debug_diy`** (+ **`_*_pcb`**); GitLab CI + GitHub Pages flasher; NVS portal axis map; **`VERSION`** / manifest sync; update banner. Docs split: README for overview; **[using-azimuth.md](using-azimuth.md)**; **[development.md](development.md)**. |
| 2026-03-28 | Firmware snapshot toward V1 revised to **60–70%** (core tracking + wireless + settings + release path in place; I/O, battery, OTA, refactor still open). |
| 2026-04-05 | **Azimuth_Design**: docs aligned with schematic/PCB; **ERC/DRC clean**; PCB design workstream **100%**; V1 checklist splits **KiCad ready** vs **assembled bring-up**; panelization remains optional for production. |
| 2026-04-18 | **V0.1** PCB **finalized**; **ordered** from fab; PCBA nearly complete (**~15 days** to assembled units). README + roadmap + V1 checklist: manufacturing milestone checked; **bring-up** pending board arrival. |
| 2026-05-02 | **Assembled V0.1 boards received**; **bring-up / testing in progress**. **Enclosure:** design **started** (not in repo yet). **Firmware:** power/thermal + modular I/O bring-up on PCB; **battery ADC** still open. README + roadmap + parts-list + kicad snapshot lines updated. |
| 2026-05-08 | **Wireless updates (OTA) shipped.** New **`track_update`** module pulls `firmware/<board>/firmware.bin` from the same trusted GitHub Pages release into the standby `ota_0` / `ota_1` slot via the Arduino `Update` class (TLS pinned to ISRG Root X1, async chunk pump in `networkLoop`), then reboots into it. Triggered by **FUNC long-press (~2 s)** *or* the portal **“Install over Wi‑Fi”** button (banner + Device card). Distinct **cyan throb** LED override + **start / ok / fail** buzzer tunes. Refuses to start in Offline‑Mode AP, during thermal hold, or with battery ≤15 % off‑charger. USB flasher remains the recovery path. Default partition table already had `ota_0` / `ota_1` slots — no NVS migration. Roadmap, [`io-led-buzzer-plan.md`](io-led-buzzer-plan.md), [`firmware-architecture-plan.md`](firmware-architecture-plan.md), and [`using-azimuth.md`](using-azimuth.md) updated. |
| 2026-05-10 | **Docs sweep:** Firmware snapshot **~90%**; **`track_network`** split reflected across [development.md](development.md), [firmware-architecture-plan.md](firmware-architecture-plan.md), [implementation-handoff-prompt.md](implementation-handoff-prompt.md), [power-and-thermal.md](power-and-thermal.md) (portal `/api/status` activity + UDP streaming vs modem sleep). **Enclosure** progress set to **~90%+** (design). |
| 2026-05-10 | **User Guide** ([user-guide.md](user-guide.md)): end-user manual; battery policy / OTA wording aligned with firmware; [development.md](development.md) + [power-and-thermal.md](power-and-thermal.md) updated for wireless OTA (manifest check ≠ USB-only). |
