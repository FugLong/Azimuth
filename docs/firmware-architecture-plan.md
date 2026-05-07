# Firmware architecture — review and improvement plan

This document captures how Azimuth firmware is structured today, what is working well, where complexity concentrates, and a **phased plan** for refactors so the codebase stays maintainable as features grow (especially richer I/O and networking).

It is a planning artifact: implement in small PR-sized steps rather than as one large rewrite.

---

## Current strengths

- **Bounded real-time path.** `main.cpp` owns IMU sampling and pose output; networking is invoked from `trackNetworkLoop()` and stays out of the hottest path except where intentional (UDP send immediately after rotation-vector reports).
- **Build dimensions are explicit.** `IMU_DEBUG_MODE` separates a bare serial build from full firmware. PlatformIO environments distinguish DIY vs PCB (`AZIMUTH_BOARD_*`) so hardware differences stay mostly in `board_config` / `azimuth_hw`.
- **Subsystem boundaries.** Battery, thermal, LED, buzzer, button, and network each live in dedicated translation units with namespaces (`azimuth_battery`, `azimuth_thermal`, `azimuth_io_*`, etc.).
- **Thin API toward the loop.** `track_network.h` exposes a small surface (`trackNetworkInit`, `Loop`, UDP send, prefs accessors, thermal emergency hook).
- **Shipping discipline.** CI builds release artifacts; `secrets.h.example` covers optional compile-time Wi‑Fi without committing secrets; docs (`docs/development.md`, wiring, thermal) match how people actually flash and configure hardware.

---

## Pressure points (technical debt)

### 1. `track_network.cpp` is a convergence point (~1200+ lines)

It combines: Wi‑Fi STA/AP, captive DNS, HTTP server and routes, JSON config, NVS preferences, OpenTrack target resolution, UDP socket lifecycle, optional firmware update check, and portal-related constants.

**Risk:** Every feature that touches “settings” or “connectivity” lands in one file; reviews get large and merge conflicts frequent.

**Direction (not mandatory order):**

| Slice | Contents (illustrative) |
|--------|---------------------------|
| **Wi‑Fi session** | Connect, reconnect policy, TX power, sleep, AP provisioning entry |
| **HTTP / API** | Route registration, handlers, JSON helpers |
| **Preferences** | Load/save NVS, merge defaults, validation |
| **OpenTrack / UDP** | Host resolution, socket, send path |

Keep **one public façade** (`track_network.h` or a thin `network_facade.cpp`) so `main.cpp` does not sprawl. Internal `.cpp` files can live alongside each other under `src/` or a `src/net/` folder once include paths are adjusted in PlatformIO if needed.

### 2. Embedded portal asset (`portal_html.cpp`)

The portal is a large HTML/CSS/JS blob in PROGMEM. That is normal for a single-binary deliverable, but diffs are noisy and editing error-prone.

**Optional later improvement:** Build-time concatenation or minification from `portal/` fragments (script already exists for logo/minify patterns). Not urgent until portal churn grows.

### 3. CI breadth

GitLab CI currently builds **`azimuth_main_diy`** only. The PCB environment (`azimuth_main_pcb`) can drift until someone builds locally.

**Recommendation:** Add at least **`pio run -e azimuth_main_pcb`** to CI (same job or parallel job). Optionally add debug envs if compile time allows.

### 4. Automated verification

**Host-side tests:** `scripts/run_host_tests.sh` compiles and runs native harnesses for config validation, config planning (Wi‑Fi / OpenTrack apply rules), and semver parsing (`tests/*.cpp`). Outputs land under `.tmp-host-tests/` (ignored by git).

**Recommendation:** When adding pure logic (e.g. more helpers split from `track_network`), extend those harnesses or add a small new test file rather than relying on hardware-only checks.

### 5. `main.cpp` battery threshold ladder

Multiple similar latch variables work but are repetitive; a **table-driven** approach (threshold + severity + callback id) would shrink duplication and reduce mistakes when tuning percentages.

---

## Phased refactor roadmap

Phases are intentionally ordered by **risk vs payoff**. Ship behavior-preserving splits before behavior changes.

### Phase A — Guardrails (low risk)

1. **CI:** Build `azimuth_main_pcb` (and optionally `azimuth_debug_diy` / `azimuth_debug_pcb`) on every pipeline run or on `main` only if job cost matters.
2. **Documentation:** Keep `docs/development.md` pointed at this file or a short “architecture” index.

### Phase B — Structural splits (medium risk, high readability)

1. Split `track_network.cpp` along the slices above; **no intentional behavior change**.
2. Optionally introduce a tiny **`network_prefs`** or **`settings_store`** module if NVS access spreads further.

### Phase C — Quality-of-life (ongoing)

1. Table-driven battery alerts in `main.cpp` (or `battery_monitor` policy module).
2. Optional portal asset pipeline when editing HTML becomes painful.

---

## Cross-cutting product behaviors (see I/O plan)

- **Pause / stasis (FUNC single tap):** Requires a **runtime gate** for UDP (and likely Hatire) separate from NVS `udp_on`, plus power-policy hooks (modem sleep). Prefer a small **`track_network`** API (e.g. set/get stasis) called from `main` on button events rather than scattering globals.
- **Wireless OTA (later):** Likely touches **partition table**, **`track_network` or a dedicated update module**, and **long-press** handling in `io_button`. Keep OTA code paths isolated so USB flashing and current manifest check remain the fallback.

---

## Principles for future changes

- **Prefer extension over explosion.** New transports or APIs should attach via clear interfaces rather than growing anonymous globals.
- **Keep latency-sensitive code dumb.** Pose → USB/UDP should stay predictable; heavy work stays in `networkLoop` / lower priority paths with `yield()` where needed.
- **One source of truth for user-visible defaults.** NVS keys and portal defaults should stay aligned (already a discipline in config POST handlers).

---

## Non-goals (for this architecture track)

- Rewriting the Arduino/Wi‑Fi stack or replacing JSON with another format without user-facing need.
- Mandating a full RTOS migration; the current cooperative loop matches the product.

---

## Related documents

- **I/O and UX plan (LED, buzzer, FUNC, pause, OTA roadmap):** [io-led-buzzer-plan.md](io-led-buzzer-plan.md)
- **Implementation handoff (ordered tasks for agents):** [implementation-handoff-prompt.md](implementation-handoff-prompt.md)
- **Development workflow:** [development.md](development.md)
