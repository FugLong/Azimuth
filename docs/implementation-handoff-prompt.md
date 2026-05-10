# Implementation handoff — prompt for a new agent / session

**How to use:** Paste the block under [“Prompt (copy below)”](#prompt-copy-below) into a fresh AI chat (or give it to a developer). The repo is **`Azimuth`** — ESP32-C3 + BNO08x head tracker firmware with OpenTrack UDP/Hatire, HTTP portal, KiCad PCB path.

This file is maintained alongside **[io-led-buzzer-plan.md](io-led-buzzer-plan.md)** and **[firmware-architecture-plan.md](firmware-architecture-plan.md)**. Update this handoff when scope or order changes.

---

## Prompt (copy below)

```
You are implementing firmware and documentation changes for the Azimuth repository (PlatformIO, Arduino-ESP32, ESP32-C3). Read these docs first — they are authoritative for intent:

- docs/io-led-buzzer-plan.md — LED/buzzer layers, FUNC button = pause/stasis (primary), future long-press OTA
- docs/firmware-architecture-plan.md — refactors (split track_network, CI matrix), hooks for pause/OTA
- docs/power-and-thermal.md — WiFi sleep, thermal hold (must interact safely with stasis)
- docs/using-azimuth.md — end-user facing; update when pause ships

High-level goals (in dependency order):

1) Documentation alignment (quick pass)
   - Ensure docs/io-led-buzzer-plan.md, firmware-architecture-plan.md, roadmap.md, and README “Documentation” table stay consistent with: FUNC single-tap = pause/stasis; double-tap reserved; long-press = future wireless update mode; OTA as future milestone using ESP32 dual-app partitions + isolated update path.
   - Add or adjust user-facing text in docs/using-azimuth.md for “Pause” when behavior exists (what pauses, sounds, resume, interaction with USB vs UDP).

2) CI guardrail (low risk, from architecture plan Phase A)
   - Extend .gitlab-ci.yml (or equivalent) so `azimuth_main_pcb` is built in addition to `azimuth_main_diy` (parallel job or matrix). Goal: PCB env cannot silently rot.

3) Pause / stasis — core product behavior (FUNC single tap)
   - Single press toggles stasis ON/OFF.
   - Stasis ON: stop OpenTrack UDP transmission (runtime gate, NOT the same as saving udp_enabled=false to NVS — user’s portal preference must apply again after resume).
   - Recommended: also pause USB Hatire output during stasis for consistent “tracker frozen” semantics; document if you choose otherwise.
   - Power: aggressive low power — at minimum force WiFi modem sleep consistent with docs/power-and-thermal.md; avoid unnecessary work each loop. Optional stretch: reduce IMU report rate during stasis (document if deferred — touches enableRotationVector / prefs).
   - Buzzer: two new distinct cues — one on enter-stasis, one on exit-stasis; respect existing buzzer volume curve.
   - LED: visible stasis state (distinct from tracking). Until LedPolicy exists, a minimal implementation is OK (e.g. slow pulse or fixed dim color on PCB RGB; DIY single-LED blink pattern).
   - Safety: stasis must not override thermal hold or critical battery behavior — define explicit rules (e.g. auto-exit stasis on thermal emergency, or block pause when critical).
   - Remove or replace temporary FUNC callbacks in main.cpp that only demo thermal tunes (see current onFuncButtonSingleTap / DoubleTap).
   - API sketch: expose from track layer something like trackNetworkStasisActive(), trackNetworkSetStasis(bool), and gate sendOpentrackUdp / Hatire send inside main or inside thin wrappers — keep main.cpp readable.

4) LED policy module (from io-led-buzzer-plan Phase 2)
   - Single owner for “what LED shows this frame”: thermal > critical battery > setup AP > stasis > transient FX > user ambient preset.
   - Refactor io_led usage so rainbow/status/manual and overrides are not scattered.
   - Portal + NVS: persist led_mode (and optional colors) alongside rgb_brightness; POST /api/config handling in track_network.cpp pattern.

5) Structural refactors (architecture plan Phase B — behavior-preserving splits)
   - Split src/track_network.cpp into logical files (wifi session, http/api, prefs, opentrack/udp) behind the same track_network.h public API.
   - Optional: table-driven battery threshold latches in main.cpp.

6) FUNC long-press + wireless OTA *(shipped 2026-05-08)*
   - `io_button::setLongPressCallback(...)` — fires once at ~2 s hold, then suppresses single-tap on release of the same press.
   - **`src/track_update.{h,cpp}`** — chunked HTTPS pull state machine, uses Arduino `Update` class directly (avoids HTTPUpdate ESP32-C3 MD5-mismatch regression). TLS pinned to ISRG Root X1 (shared with manifest check via `azimuth_net::releaseRootCaCert()`).
   - Default partition table on `seeed_xiao_esp32c3` / `esp32-c3-devkitc-02` already includes `ota_0` / `ota_1` / `otadata`, so no `board_build.partitions` change — NVS preferences survive.
   - Triggers: long-press *or* `POST /api/update`. Live progress on `GET /api/update_status`. Portal shows **Install over Wi‑Fi** in the banner + Device card, plus a blue progress card.
   - Distinct cyan‑throb LED override + start / ok / fail buzzer tunes.
   - Refuses to start in Offline AP, thermal hold, or battery ≤15 % off-charger. Stasis is forced on for the duration so radio bandwidth/CPU go to the fetch.
   - USB esp-web-tools flasher remains the recovery path.

Constraints:
- Match existing code style (namespaces azimuth_*, minimal unrelated refactors).
- Do not remove USB web flasher workflow; OTA is additive.
- IMU_DEBUG_MODE builds must still compile (network stubs / conditional compilation as today).

Deliver work in small commits/PRs: CI first, then pause feature + docs, then LED policy, then track_network split.

When done, update docs and this handoff file if the actual order or APIs differ from what was planned.

---

## Status (maintenance)

**Last implementation pass:** CI builds **`azimuth_main_diy`** (artifacts) + **`azimuth_main_pcb`** (compile-only) on `main`. **Pause/stasis:** `trackNetworkSetStasis` / `trackNetworkStasisActive`, UDP + Hatire gated in firmware, modem sleep forced in stasis, FUNC double-tap idle, thermal emergency clears stasis. **LED:** `io_led_policy` priority stack + `PolicyOverride` in `io_led`; **`led_mode`** in NVS + portal + `/api/status`. **OTA:** **`track_update`** module — chunked HTTPS pull from same trusted release URL as manifest check; long-press FUNC (~2 s) + portal **Install over Wi‑Fi** button trigger; `POST /api/update` + `GET /api/update_status`; cyan-throb LED override + start/ok/fail buzzer tunes; refuses to start in Offline AP / thermal hold / battery ≤15 % off-charger. **`track_network.cpp` split** not done yet — next refactor PR.
```

---

## Why this order

| Step | Rationale |
|------|-----------|
| Docs first | Avoids implementers inventing different FUNC semantics. |
| CI PCB | Cheap regression catch before larger edits. |
| Pause/stasis | Highest user value; touches main loop, network, buzzer, LED — establishes APIs LedPolicy will consume. |
| LedPolicy + portal | Builds on stasis as one override layer. |
| Split `track_network` | Easier once new hooks (stasis, prefs) are clear; reduces merge pain if done too early without design. |
| OTA last | Partition changes and security deserve isolation; depends on stable button + network boundaries. *(Shipped — kept default partition table to avoid NVS migration; isolated module.)* |

---

## Related documents

- [io-led-buzzer-plan.md](io-led-buzzer-plan.md)
- [firmware-architecture-plan.md](firmware-architecture-plan.md)
- [roadmap.md](roadmap.md)
