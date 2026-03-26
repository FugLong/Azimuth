# Azimuth — roadmap & progress (toward V1)

This document tracks **estimated** progress and planned work toward a **V1** release: stable hardware, enclosure, and firmware that meets the goals in the main README. Percentages are manual snapshots—update them when milestones land.

---

## Snapshot (workstreams)

| Workstream | Progress | Notes |
|------------|----------|--------|
| Hardware selection | **100%** | ESP32-C3 XIAO + BNO086-class IMU, etc. |
| Custom PCB (KiCad) | **~90%** | Layout + power nets (**`/Bat+`** / **`vcc`**) aligned with schematic; [wiring.md](wiring.md) / [parts-list.md](parts-list.md) match KiCad—run **DRC/ERC** and lock BOM before fab |
| Firmware | **~8%** | SPI IMU + debug serial + Hatire-over-USB only; no board IO, power, or wireless yet |
| 3D enclosure | **0%** | Not started |
| End-user docs & release | **TBD** | Flashing, build guide, OpenTrack setup—grow with firmware |

**Visual (rough):**

```
Hardware/BOM     [████████████████████] 100%
PCB design       [█████████████████░░░] ~90%
Firmware         [█░░░░░░░░░░░░░░░░░░░] ~8%
Enclosure        [░░░░░░░░░░░░░░░░░░░░] 0%
```

---

## V1 product posture

**Wireless is a V1 requirement** — The tracker is meant to run untethered with a battery; WiFi is the primary way to get tracking data to the PC in that mode. **USB stays supported** for desk use, flashing, serial debug, and as a fallback path (e.g. Hatire over USB as today).

**Enclosure** — Default plan is **one enclosure** sized for the **battery** build (internal volume for cell + PH connector clearance). A **second, slimmer** enclosure for **wired-only** builds is possible if there is demand: same PCB, but without a battery installed and ideally without the tall **JST PH2.0** connector populated (see assembly note below).

---

## V1 — what “done” means (draft)

Use this as a checklist; tighten or relax before tagging V1.

- [ ] **PCB** — Fabrication-ready: DRC clean, BOM fixed, assembly tested on at least one revision.
- [ ] **Bring-up** — IMU, USB serial, Hatire path verified on the **actual** PCB (not only breadboard).
- [ ] **Firmware core** — Clear module boundaries (HAL / drivers / app), pins and features match [wiring.md](wiring.md).
- [ ] **I/O** — Status LED, **FUNC1** button, buzzer as on PCB; debouncing and behavior documented.
- [ ] **Battery** — Voltage readout (divider on D0/GPIO2), low-battery indication or policy (thresholds TBD).
- [ ] **Wireless** — WiFi tracking path working for normal use (with USB still available for flash / debug / optional wired Hatire).
- [ ] **Updates** — Story for flashing firmware (USB minimum; OTA/web flashing if promised for V1).
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

**Goal:** Primary use case is **untethered / battery** operation: a stable wireless path for orientation (or raw) data to a host. **USB Hatire** remains supported in parallel where practical (priority/fallback rules TBD).

| Task | Status |
|------|--------|
| WiFi STA (and maybe AP for onboarding — decide) | ⬜ |
| Transport: protocol choice (UDP vs TCP; OpenTrack / companion expectations) | ⬜ |
| Reuse or mirror Hatire semantics over network where applicable | ⬜ |
| Coexistence with USB (priority, fallback) | ⬜ |
| Security baseline (WiFi credentials storage, at minimum) | ⬜ |

---

## Phase 3 — Easy flashing & settings from the web

**Goal:** Users can install or update firmware and manage settings without a full dev environment—ideally tied to GitHub releases or a small official tool.

| Task | Status |
|------|--------|
| Define artifact: release binaries + manifest/versioning | ⬜ |
| Web-based flasher (e.g. **esp-web-tools** / browser serial) or documented one-click flow | ⬜ |
| Settings: schema, storage (NVS), migration between versions | ⬜ |
| Document recovery if flash fails (USB bootloader, button combo, etc.) | ⬜ |

---

## Phase 4 — On-device web / phone UX (optional for V1)

**Goal:** Configure WiFi, calibration shortcuts, and modes from a phone or laptop without custom desktop software—**if** it fits V1 scope.

| Task | Status |
|------|--------|
| Captive portal or small HTTP server on ESP (when in AP or dual mode) | ⬜ |
| **Or** BLE GATT for lightweight config (often nicer for phones; more firmware work) | ⬜ |
| Same settings backend as Phase 3 (one model, multiple UIs) | ⬜ |

*Scope note:* V1 already assumes **WiFi streaming** (Phase 2). This phase is **on-device configuration UX** (portal / BLE / etc.)—can slip to **V1.1** if needed, as long as WiFi credentials and behavior are manageable another way (e.g. serial, build flags, or a minimal flow).

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
| **Privacy / safety** | No cloud requirement; local network only—state explicitly in README. |

---

## Changelog (manual)

| Date | Update |
|------|--------|
| 2026-03-22 | Initial roadmap: workstream snapshot + phases 1–4 + V1 draft criteria. |
| 2026-03-22 | V1 posture: wireless required; USB supported; enclosure + PH2 assembly tradeoff documented. |
| 2026-03-25 | Docs + BOM aligned with KiCad (**PWR1**, **`vcc`**, **BUZZER1**, **MLT-5020**, **FUNC1** footprint). |
