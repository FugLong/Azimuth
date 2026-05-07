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
| **Long press (hold)** | **Future:** enter **wireless update / OTA mode** (see [Wireless updates (future)](#wireless-updates-future-ota)). Not required for first pause implementation. |

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
| **Azimuth-Setup AP** | Unique animation — provisioning | Optional short chirp on entering setup (once) |
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

**Goal:** Let users refresh firmware **over Wi‑Fi** without USB, aligned with a **long-press** path on FUNC (after pause/resume is stable).

**Feasibility:** ESP32 (Arduino / ESP-IDF) supports **OTA** via dual **app partitions** (`ota_0` / `ota_1`) and `HTTPUpdate` or custom HTTPS fetch. This repo today ships **USB web flasher** + optional manifest check from the portal — **OTA is additive**, not replacing USB recovery.

**Likely building blocks (for a later milestone):**

- Partition table with **OTA slots** (PlatformIO `board_build.partitions` or custom CSV).
- **Trusted URL** or **upload endpoint** on-device (only in a dedicated “update mode” to shrink attack surface).
- **Long-press FUNC** → enter **update mode** (distinct LED + optional lightweight HTTP server or documented SSID behavior); exit safely on success or revert.

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

### Phase 4 — Long-press + OTA

- Button driver: hold detection.
- Partition scheme + OTA pull or upload mode.

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
