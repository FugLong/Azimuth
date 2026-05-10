# Implementation handoff — prompt for a new agent / session

**How to use:** Paste the block under [“Prompt (copy below)”](#prompt-copy-below) into a fresh AI chat (or give it to a developer). The repo is **`Azimuth`** — ESP32-C3 + BNO08x head tracker firmware with OpenTrack UDP/Hatire, HTTP portal, KiCad PCB path.

This file is maintained alongside **[io-led-buzzer-plan.md](io-led-buzzer-plan.md)** and **[firmware-architecture-plan.md](firmware-architecture-plan.md)**. Update this handoff when scope or order changes.

---

## Current product state (2026‑05, ~90% firmware)

The items below are **shipped** on `main` unless noted.

- **CI:** GitLab builds **`azimuth_main_diy`** (artifacts), **`azimuth_main_pcb`**, **`azimuth_debug_diy`**, runs **`scripts/run_host_tests.sh`**.
- **Networking:** Split into **`src/track_network_{wifi,http,prefs,udp}.cpp`** + **`track_network_internal.h`**; public API **`track_network.h`** unchanged for **`main.cpp`**.
- **Pause/stasis:** `trackNetworkSetStasis` / `trackNetworkStasisActive`; UDP + Hatire gated; modem sleep forced in stasis; FUNC double-tap idle; thermal emergency clears stasis.
- **LED:** `io_led_policy` priority stack + `PolicyOverride`; **`led_mode`** in NVS + portal + `/api/status`.
- **OTA:** `track_update` — chunked HTTPS pull; FUNC long-press (~2 s) + portal **Install over Wi‑Fi**; `POST /api/update` + `GET /api/update_status`; refuses Offline AP / thermal hold / critical battery off‑charger.
- **Portal UX:** Power-aware `/api/status` polling, boot/focus bursts, manifest-pending faster polls (`web/app/main.js` + `web/app/config.js` → codegen to **`portal_html.cpp`**).

**Remaining toward V1:** See **[roadmap.md](roadmap.md)** — e.g. battery ADC polish, enclosure, optional **`main.cpp`** decomposition, NVS schema versioning.

---

## Prompt (copy below)

```
You are implementing firmware and documentation changes for the Azimuth repository (PlatformIO, Arduino-ESP32, ESP32-C3). Read these docs first — they are authoritative for intent:

- docs/io-led-buzzer-plan.md — LED/buzzer layers, FUNC button behavior
- docs/firmware-architecture-plan.md — module layout (network split complete), CI
- docs/power-and-thermal.md — modem sleep, portal polling, UDP/streaming interaction
- docs/user-guide.md — end-user manual (prefer this for “how do I use it?”)
- docs/using-azimuth.md — portal / OpenTrack reference detail

Constraints:
- Match existing code style (namespaces azimuth_*, minimal unrelated refactors).
- Networking changes: extend `track_network_*.cpp` / `track_network_internal.h`; keep `track_network.h` stable unless coordinated with main loop.
- IMU_DEBUG_MODE builds must still compile (network stubs / conditional compilation as today).
- After editing `web/`, run: python3 scripts/portal_codegen.py --generate

Prioritize items from docs/roadmap.md and any issue the user describes.
```

---

## Historical note (original phased plan)

The repo originally tracked work as: CI guardrails → pause/stasis → LED policy → **network split** → OTA. **OTA** and **network split** are complete. The embedded prompt above is trimmed for **maintenance**; older milestone text lives in git history and **[roadmap.md](roadmap.md)**.

---

## Related documents

- [io-led-buzzer-plan.md](io-led-buzzer-plan.md)
- [firmware-architecture-plan.md](firmware-architecture-plan.md)
- [roadmap.md](roadmap.md)
