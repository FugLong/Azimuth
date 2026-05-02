# Power and thermal behavior (firmware)

This document describes **how current firmware reduces heat and battery use** without changing the head-tracking path. Implementation lives mainly in [`src/track_network.cpp`](../src/track_network.cpp), [`src/power_policy.cpp`](../src/power_policy.cpp), and the settings UI in [`src/portal_html.cpp`](../src/portal_html.cpp).

**User-facing controls:** portal **Tracking & radio** — **Power profile**, **Wi‑Fi TX power**, and **IMU report interval** (see [**Using Azimuth → Power, heat, and battery**](using-azimuth.md#power-heat-and-battery)).

---

## What actually costs power

On a small ESP32‑C3 module, **Wi‑Fi** dominates current and perceived warmth: the radio staying associated, transmit power, and how often the stack runs. The **IMU fusion and pose output** are comparatively small compared to keeping Wi‑Fi fully busy.

Firmware therefore targets **radio duty**, **CPU time in the network stack**, and **unnecessary HTTP** — not the core tracking math.

---

## Tracking is not throttled by these changes

Pose updates (USB Hatire and OpenTrack **UDP**) still run **whenever a new rotation-vector report arrives** from the BNO08x. That rate is set by **IMU report interval** in the portal (and reboot), not by Wi‑Fi power logic.

Power-related code does **not** insert a 200 ms (or similar) delay into sending poses. Background timers only affect **portal HTTP servicing**, **DNS refresh for the OpenTrack host**, **UDP socket setup**, **modem sleep policy**, and **optional HTTPS** for the update banner.

---

## 1. Adaptive Wi‑Fi modem sleep

Previously, firmware could keep **`WiFi.setSleep(false)`** in station mode so the radio stayed maximally awake (helpful for mDNS responsiveness, at the cost of heat).

**Now:** On a normal **home Wi‑Fi** connection, after the device has been **idle from portal activity** for a profile-dependent time, firmware enables **modem sleep** again (`WiFi.setSleep(true)`), which reduces average radio duty.

**While idle is measured from:** any request that calls “portal activity” (e.g. loading the page, `/api/status`, `/api/config`, scan, etc.).

**Not sleeping:** **Azimuth‑Setup** AP / captive portal, or when **not** connected as STA — sleep stays off so setup and recovery stay reliable.

Idle delay before sleep is defined in [`src/power_policy.cpp`](../src/power_policy.cpp) (`wifiSleepIdleDelayMs`): longer in **performance**, shorter in **battery saver**.

---

## 2. Power profiles

Stored in NVS as `power_profile` (default **balanced** = `1`). They tune **how aggressively** the firmware saves power vs responsiveness:

| Value | Name | Effect (high level) |
|------:|------|----------------------|
| `0` | Performance tracking | Faster portal servicing; modem sleep only after **long** portal idle. |
| `1` | Balanced | Default compromise. |
| `2` | Battery saver | Slower portal servicing; modem sleep **sooner** after idle; **skips** the HTTPS “new firmware” check (see below). |

Concrete numbers (may change in code): **HTTP `handleClient` interval** (`networkServiceIntervalMs`) and **modem-sleep idle delay** (`wifiSleepIdleDelayMs`) — see [`src/power_policy.cpp`](../src/power_policy.cpp).

---

## 3. Time-sliced `trackNetworkLoop()`

The main loop calls `trackNetworkLoop()` often (driven by IMU / idle), but **work inside it is split**:

- **Portal HTTP** (`handleClient` for STA and AP, plus captive DNS on AP) runs on a **timer** (milliseconds, from power profile) — not on every single loop iteration.
- **Heavier “background” tasks** (OpenTrack hostname refresh cadence, UDP socket open, applying adaptive modem sleep) run on a **~200 ms** cadence so DNS / sleep policy are not re-evaluated thousands of times per second.

This cuts **CPU + Wi‑Fi stack** churn when the portal is open or the loop runs fast; it does **not** batch UDP pose packets.

---

## 4. Default Wi‑Fi transmit power

If NVS has no `wifi_tx` value yet, firmware defaults to **low** TX (~2 dBm) to favor **cooler / lower current** when the access point is close. Users can raise **balanced** or **high** in the portal for weak links.

---

## 5. Firmware update check (HTTPS)

On STA, after the link is stable, firmware may perform **one** HTTPS `GET` to the published manifest to decide if the portal should show an “update available” banner. **Timeouts are short** so a slow CDN does not hold the device in a long active session.

In **battery saver** profile, this check is **skipped** to avoid that HTTPS transaction entirely.

Updates are still **USB-only**; this is only a **notification** path.

---

## 6. Portal page polling (browser)

The settings page is mostly static. After the first load (which calls `/api/status` to fill the form), the script **polls `/api/status` every 15 seconds** while the tab is **visible** — only to refresh the **stats line**, banners, and toggle sync. Polling **pauses** when the tab is hidden (`document.hidden`).

That rate is **much slower** than once per second; it is **not** the tracking rate.

---

## Summary table

| Mechanism | Saves power / heat by… | Affects tracking pose rate? |
|-----------|-------------------------|-----------------------------|
| Modem sleep after portal idle | Lower average Wi‑Fi duty | No |
| Power profile | Tunable servicing + sleep timing | No |
| Sliced `trackNetworkLoop` | Less HTTP/DNS/sleep work per second | No |
| Default low `wifi_tx` | Less TX current | No (may affect Wi‑Fi range) |
| Shorter / skipped update check | Less HTTPS + radio active time | No |
| 15 s portal poll, hidden tab pause | Less HTTP when UI open | No |
| IMU interval (user setting) | Fewer reports → less USB/Wi‑Fi **traffic** from poses | **Yes** — this is the main tracking “refresh” control |

---

## Related docs

- [**Using Azimuth → Power, heat, and battery**](using-azimuth.md#power-heat-and-battery) — practical guidance and portal knobs.
- [**Development**](development.md) — build / flash; not specific to power.
