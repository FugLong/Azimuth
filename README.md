<div align="center">

<img src="logo/AzimuthLogo_Dark.png#gh-light-mode-only" alt="Azimuth" width="240">
<img src="logo/AzimuthLogo_Light.png#gh-dark-mode-only" alt="Azimuth" width="240">

</div>

# Azimuth

Azimuth enables high-quality PC head tracking for sims, flight games, and anything that supports [OpenTrack](https://github.com/opentrack/opentrack).

It is a cheap but powerful tracker you can build yourself: a small ESP32-based board and an advanced IMU that gives stable head orientation, with open hardware and firmware so you can build, fix, or change the project instead of relying on a closed commercial device.

This repository holds firmware, PCB designs, 3D print designs, and documentation.

<div align="center">
<table>
  <tr>
    <td align="center"><img src="img/BoardTop.png" alt="PCB top" width="380"></td>
    <td align="center"><img src="img/BoardBottom.png" alt="PCB bottom" width="380"></td>
  </tr>
</table>
</div>

### Development status (toward V1)

| Area | Progress | Notes |
|------|:--------:|--------|
| Hardware / BOM | 100% | Parts chosen ([docs/parts-list.md](docs/parts-list.md)). |
| Custom PCB | ~95% | KiCad aligned with [docs/wiring.md](docs/wiring.md); **panelization** (fab-ready panel) is the remaining PCB task before ordering. |
| Firmware | ~40% | SPI IMU, USB debug, **Hatire + WiFi → OpenTrack UDP**, **on-device settings** over HTTP (NVS + `secrets.h` fallback; see below). **Board I/O** (LED, button, buzzer), **battery/ADC** still ahead ([roadmap](docs/roadmap.md)). |
| 3D enclosure | 0% | Not started. Plan: **battery-sized** shell first; optional slimmer **wired-only** enclosure if PH2 is omitted on those builds ([roadmap](docs/roadmap.md)). |

```
Hardware/BOM   [████████████████████] 100%
PCB            [███████████████████░] ~95%
Firmware       [████████░░░░░░░░░░░░] ~40%
Enclosure      [░░░░░░░░░░░░░░░░░░░░] 0%
```

**Roadmap & checklists:** [docs/roadmap.md](docs/roadmap.md)

---

## Goals

- **Performance** — Low-latency, stable orientation for gaming and desktop use.
- **Accessibility** — Cheap, easy-to-source parts; open firmware and tooling so people can build and modify their own trackers.
- **Interop** — First-class support for **OpenTrack** (and the same ecosystem of games and sims that already support standard head-tracking pipelines).
- **Evolvability** — Room for richer calibration, on-device UX, and PC-side apps without dragging forward the old platform’s constraints.
- **Lineage** — Successor to **[Nano33_PC_Head_Tracker](https://github.com/FugLong/Nano33_PC_Head_Tracker)**: same broad idea—DIY, affordable, OpenTrack-first—but the old **Nano 33 BLE** + onboard **LSM9DS1** stack topped out; fusion tuning couldn’t pull enough quality out of that hardware and software. **Azimuth** starts over on **ESP32** and an external **BNO08x**-class IMU, with new firmware and no legacy code path.

---

## What’s here now

| Area | Status |
|------|--------|
| **Firmware** | PlatformIO project for **Seeed XIAO ESP32-C3** + **BNO08x** over **SPI**: fused yaw / pitch / roll from the rotation-vector report. |
| **OpenTrack** | `xiao_esp32c3_hatire`: **Hatire Arduino** over USB (30-byte frames) **and** optional **WiFi → UDP** to the PC using OpenTrack’s **UDP over network** input (6× `double`, port 4242 by default). **HTTP settings** on port **8080** (`http://azimuth.local:8080`) for Wi‑Fi / UDP / reboot (NVS + `secrets.h` fallback). |
| **Debug** | Text telemetry over USB serial (`xiao_esp32c3` build). |
| **Hardware docs** | **[docs/wiring.md](docs/wiring.md)** (signals, power, GPIO map) · **[docs/parts-list.md](docs/parts-list.md)** (BOM + passives notes) · **[docs/kicad.md](docs/kicad.md)** (custom KiCad libs + collaboration). |

Planned work (board I/O, battery, web flashing / settings UX, enclosure, richer calibration) is tracked in **[docs/roadmap.md](docs/roadmap.md)**.

---

## Hardware (this revision)

- **MCU:** Seeed **XIAO ESP32-C3** (USB CDC serial, compact footprint).
- **IMU:** **BNO086** on the custom PCB (or **BNO08x** breakout in **SPI** mode with **PS0** / **PS1** → **3.3 V**).

Summary SPI / control pin map (full table, battery, buttons, LED, buzzer in **[docs/wiring.md](docs/wiring.md)**; BOM in **[docs/parts-list.md](docs/parts-list.md)**):

| Signal | XIAO pin | GPIO |
|--------|----------|------|
| SCK | D8 | 8 |
| MISO | D9 | 9 |
| MOSI | D10 | 10 |
| CS | D3 | 5 |
| H_INT | D4 | 6 |
| PS0 / WAKE | D2 | 4 |
| NRST | D7 | 20 |

---

## Building and flashing

Requires [PlatformIO](https://platformio.org/). Default environment: **`xiao_esp32c3`** (debug text).

**Debug (serial monitor, yaw/pitch/roll):**

```bash
python3 -m platformio run -e xiao_esp32c3 -t upload
python3 -m platformio device monitor
```

**OpenTrack — Hatire (USB) + optional UDP (WiFi):**

```bash
python3 -m platformio run -e xiao_esp32c3_hatire -t upload
```

Copy **`include/secrets.h.example`** to **`include/secrets.h`** and set **`WIFI_SSID`**, **`WIFI_PASSWORD`**, and **`OPENTRACK_UDP_HOST`** (your PC’s LAN IP). `secrets.h` is **gitignored** so credentials are not committed. If there is **no SSID in NVS and `WIFI_SSID` is empty**, or if it **tries to join a saved network and fails** (wrong password, etc.), the board starts an open provisioning network **`Azimuth-Setup`**. It runs a small **captive portal**: DNS sends lookups to the board, HTTP on **port 80** answers OS “sign in to Wi‑Fi” checks with a redirect to **`http://192.168.4.1/`** (same settings UI as on the LAN). **`azimuth.local` does not apply on this network**—mDNS is only advertised after the board joins your home Wi‑Fi. After you save home Wi‑Fi and reboot, **`Azimuth-Setup` turns off** and normal mode uses **`http://azimuth.local:8080`** (or **`http://<LAN-IP>:8080`**). UDP port **`OPENTRACK_UDP_PORT`** is set in **`platformio.ini`** (default **4242**).

- **USB:** Input **Hatire Arduino**, **115200**, **DTR** on; start tracking and **recenter** after the filter settles. Do not leave a text serial monitor open on that port.
- **Hatire axis mapping (important):** In the Hatire tracker settings, set **Yaw axis = Rot 0**, **Pitch axis = Rot 1**, **Roll axis = Rot 2** (some UIs say “axis 0 / 1 / 2”). That lines up with how this firmware fills the packet and keeps USB and UDP identical. OpenTrack’s *old* Hatire defaults use **0 / 2 / 1**, which swaps pitch and roll—change to **0 / 1 / 2** for the simplest setup.
- **WiFi / UDP:** Input **UDP over network**, same port as in **`platformio.ini`** (`OPENTRACK_UDP_PORT`); allow the port through the PC firewall. Hatire and UDP both run from the same firmware.

### On-device settings (WiFi + OpenTrack)

With the **hatire** build:

- **Already on your LAN:** open **`http://<hostname>.local:8080`** (default hostname **`azimuth`**) or **`http://<device-ip>:8080`**.
- **Provisioning (`Azimuth-Setup`):** join that network (no password). Many phones **open the settings page automatically**; if not, go to **`http://192.168.4.1/`** on **port 80**. After you save a real SSID (and password if needed), the device **reboots** into **station-only** mode—**`Azimuth-Setup` does not stay on**.

The portal is grouped into **Wi‑Fi**, **LAN & discovery**, **OpenTrack (PC)**, **Tracking & radio**, and **Device**:

| Section | What you can change |
|--------|---------------------|
| **Wi‑Fi** | SSID / password, **Scan networks** (brief tracking hitch). New Wi‑Fi credentials → **reboot**. |
| **LAN & discovery** | **mDNS** on/off, **device hostname** (letters, digits, hyphen; max 24). Changing these → **reboot** so DHCP/mDNS apply. |
| **OpenTrack (PC)** | **USB Hatire** on/off (Wi‑Fi‑only use), **UDP** on/off, **host** / **port** for OpenTrack’s *UDP over network* input. |
| **Tracking & radio** | **IMU report interval** (5 / 10 / 20 / 40 ms → 200 / 100 / 50 / 25 Hz). Change → **reboot** so the BNO08x report rate is reapplied. **Wi‑Fi TX power** (low / balanced / high) applies on save without reboot. |
| **Device** | Firmware version string, **Reboot**, **Erase saved settings** (clears NVS `azimuth` namespace and reboots into provisioning if no compile-time Wi‑Fi in `secrets.h`). |

Values live in **NVS** (`Preferences` namespace **`azimuth`**); empty NVS keys still fall back to **`include/secrets.h`**. Firmware version is set at build time (`AZIMUTH_FW_VERSION` in **`platformio.ini`** for the hatire env).

The page is served by the stock Arduino **`WebServer`**: when no browser is connected, the firmware only calls **`handleClient()`** once per main loop (no background worker). **Wi‑Fi scan** runs only when you press **Scan networks** and can stall tracking briefly for a second or two.

Saving **new Wi‑Fi credentials** triggers an automatic **reboot** so the radio can reconnect. If it **cannot join** the saved network (wrong password, AP missing, etc.) after the boot timeout, it opens the same recovery path: join **`Azimuth-Setup`** (captive portal or **`http://192.168.4.1/`**) to fix SSID/password (no reflash required).

**mDNS / `azimuth.local`:** Use **`http://azimuth.local:8080`** only after the board is on your **home** Wi‑Fi as a client. On **`Azimuth-Setup`**, use **`http://192.168.4.1/`** (port **80**) or wait for the captive portal sheet—**do not expect `azimuth.local` there**. If the name does not resolve on your LAN:

- **Windows:** Install **Bonjour Print Services** (Apple) or **iTunes** (includes Bonjour)—plain Windows does not resolve **`.local`** names by default. Then try again or use **`http://<LAN-IP>:8080`** (see your router’s DHCP client list; hostname may appear as **`azimuth`**).
- **Same Wi‑Fi / VLAN / guest network:** mDNS usually does not cross VLANs or some **guest/isolated** SSIDs; use the **IP** address instead.
- **Firmware:** STA mode uses **Wi‑Fi modem sleep off** while the portal is enabled so mDNS announcements are not starved; if you changed sleep/TX power in code, recheck behavior.

**Always works:** **`http://<device-LAN-IP>:8080`** as long as the board shows **connected** on your AP.

### OpenTrack on the PC (recommended)

Use **either** **Hatire Arduino** (USB) **or** **UDP over network** as the **Input**—not both at once. Pick the one you’re using. By default the firmware can drive **both** outputs; you can turn **USB Hatire** off in the portal for Wi‑Fi‑only use. OpenTrack should still only subscribe to **one** path.

| Step | What to do |
|------|------------|
| **Input** | **Hatire Arduino** (correct COM port, **115200**, **DTR** on) **or** **UDP over network** (OpenTrack listens on **`OPENTRACK_UDP_PORT`** from **`platformio.ini`**, usually **4242**; set **`OPENTRACK_UDP_HOST`** in `secrets.h` to **this PC’s LAN IP**; allow UDP in the firewall). |
| **Hatire axes** | **Yaw / Pitch / Roll** → **Rot 0 / Rot 1 / Rot 2** (see bullet above). |
| **Filter** | **Natural motion** filter (name in the filter dropdown may vary slightly by OpenTrack version; pick the **Natural motion** / natural-style preset if available). |
| **Responsiveness** | Turn **responsiveness** up to **maximum** (slider all the way up) so head motion matches the tracker with minimal lag. |
| **Start** | Click **Start**; use **Center** / **recenter** after the filter settles. |

Tweak filter and output mapping per game if needed; the table above is the baseline for Azimuth.

### Thermal / power (why the board feels warm)

A **Seeed XIAO ESP32-C3** runs warm in this use case mostly because of **Wi‑Fi**, not because UDP is “heavy.” The radio stays associated with your AP, listens for beacons, and transmits ~100 packets/s — that RF chain draws far more than the CPU spent packing 48-byte datagrams. **USB serial** adds a bit more. A small board with little heatsinking **will feel hot to the touch** when Wi‑Fi is up; that’s normal for this class of module.

Firmware already helps where it’s safe: **Wi‑Fi modem sleep** after connect (less always-on RF; disable in code if UDP gets flaky) and **`yield()`** when the IMU has no event yet (avoids spinning the CPU at full speed between 100 Hz samples). Firmware caps Wi‑Fi **TX power** at **8.5 dBm** by default (SDK default is near **~19 dBm** max). Same-room tracking usually doesn’t need full power; raising `kWifiTxPower` in `src/track_network.cpp` helps if UDP drops on a weak path.

### Battery runtime (estimate)

These are **rough order-of-magnitude** figures—not measured on your Azimuth PCB. Use them for planning; **measure** pack current at 3.7 V for a real product number.

**Seeed XIAO ESP32-C3** (Wi‑Fi on, from the [Getting Started specs](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)): **~75 mA** “active” Wi‑Fi, **~25 mA** modem-sleep. **Wi‑Fi dominates**; the IMU adds on the order of **a few mA** (typical for BNO08x-class fusion at ~100 Hz). Current firmware uses modem sleep + ~100 Hz IMU + optional UDP/Hatire.

**Example — a 400 mAh 1S LiPo, wireless tracking (Wi‑Fi + IMU + UDP, no USB):**

| Assumed average current | Ideal hours (400 mAh ÷ I) | After ~85% usable-capacity derating (typical) |
|-------------------------|---------------------------|-----------------------------------------------|
| ~40 mA (optimistic) | ~10 h | ~8.5 h |
| ~55 mA (mid) | ~7.3 h | ~6 h |
| ~80 mA (pessimistic, nearer “active” Wi‑Fi) | ~5 h | ~4 h |

So **~4–9 hours** on a **400 mAh** cell is a **reasonable band** until you bench it; **~5–7 hours** is a plausible **mid** guess for **UDP + Wi‑Fi** use. **USB-only Hatire** (empty `WIFI_SSID`, Wi‑Fi off) lasts **much longer**—RF is off. **Signal strength, AP distance, cell age, and temperature** all move the number.

---

## Firmware layout

- **`src/main.cpp`** — IMU bring-up, rotation vector, Hatire + optional OpenTrack UDP; `kPinCs` / `kPinInt` / `kPinRst` match the **ESP32_BNO086** PCB (see [docs/wiring.md](docs/wiring.md)).
- **`include/opentrack_pose.h`** — Single mapping from fusion Euler (deg) to Hatire / OpenTrack UDP rotation channels (Rot 0–2).
- **`src/track_network.cpp`** — Hatire build only: Wi‑Fi STA, provisioning AP + captive portal, HTTP handlers (NVS + `secrets.h`), OpenTrack UDP client.
- **`src/portal_html.cpp`** — PROGMEM settings UI (linked only in the Hatire env; see `platformio.ini`).
- **`platformio.ini`** — `espressif32`, `seeed_xiao_esp32c3`, **SparkFun BNO08x** library.
- **`include/secrets.h`** — local WiFi + OpenTrack host (copy from `secrets.h.example`; not tracked by git).

If you move SPI off the default D8–D10 pins, call `SPI.begin(sck, miso, mosi, -1)` **before** `imu.beginSPI(...)` so the bus matches your board (the SparkFun driver initializes `SPI` internally; ESP32 keeps an already-started bus).
