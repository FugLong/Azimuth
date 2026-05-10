# I/O experience plan — LED, buzzer, FUNC button

This document describes a **layered, user-respecting model** for RGB, buzzer, and the FUNC button: what users can choose, when the system may **override** that choice, and how we grow features without spaghetti state.

It builds on code that already exists: `azimuth_io_led` defines `RgbPreset` (Rainbow, RainbowSlow, Status, Manual), IMU-driven `setStatus()`, rainbow timing in `tick()`, and portal **RGB brightness** (`rgb_brightness`). The FUNC button supports single/double tap and **long-press can be added** in `io_button` when needed.

**Primary product behavior for FUNC (single tap):** **Pause / stasis** — stop OpenTrack **UDP** output, push radio and workload toward **super low power**, play an **entry** buzzer sound; another single tap **resumes** tracking and plays a **distinct exit** sound. This is for leaving the device on (e.g. on battery) without wasting power while away from the PC.

---

## Design goals

1. **User intent is default.** In normal operation, LED behavior follows a **saved preference** (preset and optional parameters).
2. **System overrides are explicit and temporary.** Setup AP, critical battery, thermal protection, or factory-defined “alarm” states may **take over** LED (and optionally buzzer policy), then **restore** the user’s preference when the condition clears.
3. **Single policy owner.** One place decides “what should the LED show **this** frame?” so rainbow, status green, battery blink, setup mode, and **stasis** do not fight in scattered `if` statements.
4. **Incremental delivery.** Ship **pause/stasis** and a **small LED priority stack** first; add richer ambient modes (axis-mapped color) once the framework exists.
5. **Hardware-aware.** DIY builds without RGB keep single-LED semantics; PCB gets full RGB behavior (`board_config` capabilities).

---

## FUNC button — primary behavior: pause / stasis

| Input | Behavior (target) |
|-------|---------------------|
| **Single tap** | **Toggle pause (stasis).** If running → enter stasis; if in stasis → resume normal operation. |
| **Double tap** | **Reserved** (not assigned for V1 — avoid accidental triggers; previous thermal **demo** hooks in `main.cpp` should be removed or repurposed). |
| **Long press (hold ~2 s)** | **Triggers wireless firmware update** — see [Wireless updates (OTA)](#wireless-updates-future--ota). Single-tap on the same press is suppressed (no accidental pause). |

### Stasis (paused) — functional requirements

- **UDP:** Do **not** send OpenTrack UDP packets while paused. This must be a **runtime** gate (session-only), independent of the portal **“UDP on”** NVS preference — resuming restores live UDP according to that saved setting.
- **USB Hatire:** **Decision for implementer:** Either pause Hatire as well for a consistent “tracker frozen” semantics, or leave Hatire on if pause is defined as “wireless only.” **Recommendation:** pause **both** UDP and Hatire during stasis so behavior is predictable; document in `using-azimuth.md`.
- **Power:** Enter aggressive low-power posture: e.g. confirm **Wi‑Fi modem sleep** (`WiFi.setSleep(true)`), rely on existing adaptive sleep policy where possible, and **avoid unnecessary work** in the loop (no UDP prep). Optional stretch: **lower IMU report rate** during stasis (requires `enableRotationVector` / prefs interaction — more invasive; document if deferred).
- **Sounds:** Distinct **enter-stasis** and **exit-stasis** cues in `io_buzzer` (short, differentiable; respect buzzer volume).
- **LED:** Distinct **stasis** appearance (e.g. slow pulse / dim color) so the headset visibly reads “idle” vs tracking. Stasis participates in the **priority stack** below safety-critical overrides.

### Resume

- Restore previous tracking outputs per prefs; play **exit** sound; clear stasis LED layer.
- If the device was in **thermal hold** or **critical battery** emergency, **safety policy wins** — stasis must not mask those (either block entering stasis or force-exit stasis when a critical event fires; implementer should choose one explicit rule).

---

## Concept: priority layers for LED output

Think of the LED pipeline each `tick()` as:

```
Evaluate active overrides (highest priority wins)
    → if none: render user's chosen "ambient" mode
```

Suggested **priority order** (high to low — tune during implementation):

| Priority | Source | Example behavior |
|----------|--------|------------------|
| Highest | Thermal hold / Wi‑Fi cut | Distinct pattern (e.g. slow red pulse or off — match thermal doc) |
| High | Critical battery | Fast warning pulse; coordinate with buzzer panic |
| Medium | Setup / captive AP (`setup_ap`) | Obvious “provisioning” pattern (e.g. cyan breathe) |
| **Stasis (pause)** | User pressed FUNC — tracking paused | Slow pulse / different hue so “idle but powered” is obvious |
| Low | Transient feedback | Short blink after save, calibration done, etc. |
| Base | User ambient mode | Rainbow, solid, axis map, manual RGB, status-only |

**Rules:**

- Overrides **stack logically** but only the **top active** layer drives pixels until it releases.
- **Stasis** is cleared on resume; it does not persist across reboot (runtime flag only).

---

## User-facing LED modes (ambient / base layer)

These complement today’s `RgbPreset` enum; names may stay as enum cases or collapse into a single `led_mode` NVS uint with sub-options.

| Mode | Description | Notes |
|------|-------------|--------|
| **Status** | IMU OK = green-ish accent; lost = dim/off | Already partially implemented via `setStatus()` |
| **Rainbow / Rainbow slow** | Decorative idle look | Already implemented |
| **Solid / palette** | A few curated colors | Easier than full RGB for many users |
| **Manual RGB** | User picks R,G,B in portal | Align with existing `setManualRgb` / portal brightness |
| **Axis hue (later)** | Map yaw/pitch/roll to color position | Fun but more tuning |
| **Off** | LED off when not overridden | Respects dark rooms |

**Portal:** Extend “Sound & light” (or a renamed card) with **LED mode** selector + optional color controls when mode = manual/solid. Persist to NVS alongside `rgb_brightness`.

---

## System overrides (non-ambient)

| Condition | LED idea | Buzzer |
|-----------|-----------|--------|
| **Azimuth-Tracker Offline Mode AP** | Unique animation — provisioning/off-grid | Optional short chirp on entering offline mode (once) |
| **Battery warning bands** | Amber / red pulse | Already partially integrated |
| **Thermal warn / emergency** | Red/orange patterns | Existing tunes; align with LED |
| **Save / cal success** | Brief green flash | Short positive beep (optional “quiet mode”) |

---

## Buzzer integration

The buzzer already plays structured tunes for battery and thermal events. Extend with **named cues** for stasis enter/exit and keep a small **policy** so tunes do not stomp each other incoherently.

- Map events → `(tune id, may_interrupt_previous)` for battery vs thermal vs **stasis**.
- **User preference:** Volume curve exists; optional future “quiet mode” suppresses non-safety cues.

---

## Wireless updates (future / OTA)

**Status:** Shipped as of 2026-05-08. The historical "future" framing is kept below for context — concrete implementation summary first.

**What ships today:**

- New **`src/track_update.{h,cpp}`** — chunked HTTPS pull state machine (`Idle → Connecting → Downloading → Finalizing → Success/Failed`).
- Pulls `firmware/<board>/firmware.bin` from the **same GitHub Pages release** the manifest check already trusts. The URL is derived at runtime from `AZIMUTH_RELEASE_MANIFEST_URL` (overridable with compile-time `AZIMUTH_RELEASE_FIRMWARE_URL`).
- **TLS** pinned to **ISRG Root X1** (Let's Encrypt), shared with the manifest check via `azimuth_net::releaseRootCaCert()` — a network MitM with a private CA cannot push a build.
- Writes through the Arduino-ESP32 **`Update`** class (not `HTTPUpdate` — that has a known ESP32-C3 MD5-mismatch regression in 2025+ Arduino frameworks; see [esphome#13255](https://github.com/esphome/esphome/issues/13255)).
- The default PlatformIO partition table for `seeed_xiao_esp32c3` / `esp32-c3-devkitc-02` (4 MB flash) already includes **`ota_0`** / **`ota_1`** / **`otadata`** — no `board_build.partitions` change needed, so **NVS preferences survive the upgrade**.
- Cooperative tick: `azimuth_update::tick()` is called from `networkLoop()`. Each tick reads up to **4 KB** from the TLS socket and writes one flash sector, so the WebServer + IMU loop stay responsive and the user can watch progress.
- **Triggers:**
  - **FUNC long-press (~2 s)** — added to `io_button` with deferred-release suppression so the press doesn't also fire single-tap pause.
  - **Portal button** — `Install over Wi-Fi` in the update banner *and* the Device card. New routes `POST /api/update` and `GET /api/update_status`.
- **LED override** — new `PolicyOverride::Update` (cyan throb at 360 ms; matching fast blink on status-only boards) wins over ambient/setup-AP/stasis but yields to thermal hold.
- **Buzzer cues** — `playUpdateStartTune` (rising fifth), `playUpdateOkTune` (resolved arpeggio just before reboot), `playUpdateFailTune` (descending stern). Distinct from FUNC / pause / battery / thermal motifs.
- **Safety gates** — refuses to begin in Offline-Mode AP (no internet), during thermal hold, with battery ≤15 % and not charging (a brownout mid-write would brick), or while another OTA is active. Stasis is forced **on** during the download so radio bandwidth and CPU go to the fetch.
- **Recovery** — USB **esp-web-tools** flasher path is unchanged. If OTA fails, the device aborts the half-written slot and stays on the working image; the failure tune + portal status both show the reason.

**Original feasibility framing (kept for archaeological reference):** ESP32 (Arduino / ESP-IDF) supports OTA via dual app partitions (`ota_0` / `ota_1`) and `HTTPUpdate` or custom HTTPS fetch. This repo previously shipped USB web flasher + manifest check from the portal — OTA is now additive, not replacing USB recovery.

Document detailed threat model and UX in a dedicated note when implementation starts; until then treat as **roadmap**, not V1-blocking.

---

## Implementation phases (suggested)

### Phase 1 — Pause / stasis (FUNC single tap)

- Runtime flag + API on network layer: e.g. `trackNetworkSetStasis(bool)` / `trackNetworkStasisActive()` gating `sendOpentrackUdp` and Hatire path from `main`.
- Buzzer: **enter** + **exit** stasis tones.
- LED: stasis pattern **or** minimal distinct brightness until full `LedPolicy` exists.
- Remove or replace **thermal demo** FUNC callbacks in `main.cpp`.
- **`docs/using-azimuth.md`:** Document pause behavior.

### Phase 2 — LedPolicy shell + persistence

- Centralize LED decisions (setup, thermal, battery, **stasis**, ambient).
- Portal: `led_mode` NVS + optional RGB fields.

### Phase 3 — Rich ambient modes

- Palette / axis hue; micro-interactions.

### Phase 4 — Long-press + OTA *(shipped)*

- Button driver: hold detection (`io_button::setLongPressCallback`, ~2 s window, suppresses single-tap on the same gesture).
- OTA module (`track_update`) reuses the default partition table's `ota_0` / `ota_1` slots — no `board_build.partitions` change.
- HTTPS pull from the same trusted release URL as the manifest check (TLS pinned to ISRG Root X1).
- Cyan-throb LED override + start/ok/fail buzzer cues; portal exposes `Install over Wi‑Fi` button + live progress (`POST /api/update`, `GET /api/update_status`).

---

## Testing and safety

- **DIY single LED:** Map stasis to slow blink vs steady tracking.
- **Thermal / battery:** Stasis must **not** hide critical warnings; define interaction explicitly.
- **Epilepsy / distraction:** Avoid harsh strobes on provisioning or stasis.

---

## Related code (today)

- `src/io_led.cpp` / `io_led.h` — presets, rainbow, status, manual RGB, brightness.
- `src/io_buzzer.cpp` — tunes and volume curve.
- `src/io_button.cpp` — debounce, single/double tap; **long-press TBD**.
- `src/main.cpp` — pose path, FUNC callbacks, `setStatus`.
- `src/track_network.cpp` — UDP gated by `gUdpSendEnabled` + connection; **add stasis gate**.
- Portal — `rgb_brightness`, buzzer volume, `udp_enabled`.

---

## Related documents

- **Architecture / refactors:** [firmware-architecture-plan.md](firmware-architecture-plan.md)
- **Power, heat, battery:** [power-and-thermal.md](power-and-thermal.md)
- **Roadmap:** [roadmap.md](roadmap.md)
- **Handoff prompt (for implementers):** [implementation-handoff-prompt.md](implementation-handoff-prompt.md)
