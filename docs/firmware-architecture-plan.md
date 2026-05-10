# Firmware architecture — review and improvement plan

This document captures how Azimuth firmware is structured today, what is working well, where complexity concentrates, and a **phased plan** for refactors so the codebase stays maintainable as features grow (especially richer I/O and networking).

It is a planning artifact: implement in small PR-sized steps rather than as one large rewrite. **As of 2026‑05:** core networking has been split into multiple translation units (Phase B done); remaining items skew toward **battery ADC polish**, **main.cpp decomposition**, and optional **portal asset pipeline** improvements.

---

## Current strengths

- **Bounded real-time path.** `main.cpp` owns IMU sampling and pose output; networking is invoked from `trackNetworkLoop()` and stays out of the hottest path except where intentional (UDP send immediately after rotation-vector reports).
- **Build dimensions are explicit.** `IMU_DEBUG_MODE` separates a bare serial build from full firmware. PlatformIO environments distinguish DIY vs PCB (`AZIMUTH_BOARD_*`) so hardware differences stay mostly in `board_config` / `azimuth_hw`.
- **Subsystem boundaries.** Battery, thermal, LED, buzzer, button, and network each live in dedicated translation units with namespaces (`azimuth_battery`, `azimuth_thermal`, `azimuth_io_*`, etc.).
- **Thin API toward the loop.** `track_network.h` exposes a small surface (`trackNetworkInit`, `Loop`, UDP send, prefs accessors, thermal emergency hook). Implementation is split across `track_network_{wifi,http,prefs,udp}.cpp` plus shared `track_network_internal.h`; `track_network.cpp` holds the façade and global runtime object.
- **Shipping discipline.** CI builds release artifacts; `secrets.h.example` covers optional compile-time Wi‑Fi without committing secrets; docs (`docs/development.md`, wiring, thermal) match how people actually flash and configure hardware.

---

## Pressure points (technical debt)

### 1. ~~`track_network.cpp` monolith~~ → split complete

Previously one large file mixed Wi‑Fi STA/AP, captive DNS, HTTP routes, NVS, OpenTrack, UDP, and update checks.

**Now:** Logical slices live in dedicated files behind the same public API:

| File | Role |
|------|------|
| `track_network.cpp` | Public wrappers; `NetworkRuntime` / `gRuntime` definition |
| `track_network_internal.h` | Shared structs, prefs/network helpers, declarations |
| `track_network_wifi.cpp` | STA/AP bring-up, `networkLoop`, thermal cut, offline portal, manifest scheduling |
| `track_network_http.cpp` | HTTP handlers, JSON API, TLS manifest fetch, `registerRoutes` |
| `track_network_prefs.cpp` | NVS merge helpers, adaptive modem sleep, stasis |
| `track_network_udp.cpp` | Host resolution, UDP socket, OpenTrack send |

### 2. Embedded portal asset (`portal_html.cpp`)

The portal is a large HTML/CSS/JS blob in PROGMEM. That is normal for a single-binary deliverable, but diffs are noisy and editing error-prone.

**Optional later improvement:** Build-time concatenation or minification from `portal/` fragments (`scripts/portal_codegen.py` already regenerates from `web/`). Not urgent until portal churn grows.

### 3. CI breadth

GitLab CI builds **`azimuth_main_diy`** (artifacts), **`azimuth_main_pcb`** (compile), **`azimuth_debug_diy`**, and runs **`scripts/run_host_tests.sh`**. Debug PCB env can still be built locally if needed.

### 4. Automated verification

**Host-side tests:** `scripts/run_host_tests.sh` compiles and runs native harnesses for config validation, config planning (Wi‑Fi / OpenTrack apply rules), and semver parsing (`tests/*.cpp`). Outputs land under `.tmp-host-tests/` (ignored by git).

**Recommendation:** When adding pure logic (e.g. more helpers split from networking), extend those harnesses or add a small new test file rather than relying on hardware-only checks.

### 5. `main.cpp` battery threshold ladder

Multiple similar latch variables work but are repetitive; a **table-driven** approach (threshold + severity + callback id) would shrink duplication and reduce mistakes when tuning percentages.

---

## Phased refactor roadmap

Phases are intentionally ordered by **risk vs payoff**. Structural network split is **complete**.

### Phase A — Guardrails (low risk)

1. **CI:** ✅ Builds **DIY + PCB** main and debug targets plus host tests on pipeline runs.
2. **Documentation:** Keep `docs/development.md` pointed at this file or a short “architecture” index.

### Phase B — Structural splits (medium risk, high readability)

1. ✅ Split networking along Wi‑Fi / HTTP / prefs / UDP slices; **preserve public `track_network.h` API**.
2. Optionally introduce a tiny **`network_prefs`** or **`settings_store`** module if NVS access spreads further — partially satisfied by `track_network_prefs.cpp`.

### Phase C — Quality-of-life (ongoing)

1. Table-driven battery alerts in `main.cpp` (or `battery_monitor` policy module).
2. Optional portal asset pipeline when editing HTML becomes painful.

---

## Cross-cutting product behaviors (see I/O plan)

- **Pause / stasis (FUNC single tap):** Runtime gate for UDP (and Hatire) separate from NVS `udp_on`, plus power-policy hooks (modem sleep in stasis). Implemented via `trackNetworkSetStasis` / `trackNetworkStasisActive`.
- **Wireless OTA *(shipped)*:** **`src/track_update.{h,cpp}`** (IMU debug builds get stubs). Uses the **default partition table's existing `ota_0` / `ota_1` / `otadata`** slots. Driven by `io_button` long-press *and* `POST /api/update`; cooperative chunk pump runs from `networkLoop`. TLS root pin is shared with the manifest check via `azimuth_net::releaseRootCaCert()`. USB flashing and the per-boot manifest version banner remain recovery and discovery fallbacks.

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
