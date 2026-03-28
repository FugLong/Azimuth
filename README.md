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
| Firmware | ~40% | SPI IMU, **`azimuth_debug`** / **`azimuth_main`**, **Hatire + Wi‑Fi → OpenTrack UDP**, **on-device settings** (NVS portal; optional `secrets.h`). **Board I/O**, **battery/ADC** still ahead ([roadmap](docs/roadmap.md)). |
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
| **OpenTrack** | **`azimuth_main`**: **Hatire Arduino** over USB (30-byte frames) **and** optional **Wi‑Fi → UDP** (OpenTrack **UDP over network**, 6× `double`, port 4242 by default). **HTTP settings** on **8080** (`http://azimuth.local:8080`) — **NVS** (portal); optional compile‑time **`secrets.h`** when NVS is empty. |
| **Debug** | **`azimuth_debug`**: yaw / pitch / roll over USB serial only (no Wi‑Fi / portal). |
| **Hardware docs** | **[docs/wiring.md](docs/wiring.md)** (signals, power, GPIO map) · **[docs/parts-list.md](docs/parts-list.md)** (BOM + passives notes) · **[docs/kicad.md](docs/kicad.md)** (custom KiCad libs + collaboration). |

Planned work (board I/O, battery, enclosure, richer calibration, OTA if desired) is tracked in **[docs/roadmap.md](docs/roadmap.md)**. **USB browser flashing** (GitHub Pages + **`web-flasher/`**) and **semver / `VERSION`** are already wired; updates are still **USB-only** (no OTA).

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

### Firmware version

- **Source of truth:** repo root **`VERSION`** (first line = semver string, e.g. `0.1.0`). **`scripts/pio_set_version.py`** injects it as **`AZIMUTH_FW_VERSION`** into **`azimuth_main`** builds. **`web-flasher/manifest.json`**’s **`version`** field is kept in sync by **`scripts/sync_manifest_version.py`** (called from **`prepare_web_flasher_firmware.sh`** and the **GitHub Pages** workflow) so the USB flasher and the running firmware agree.
- **Scheme (intended):** align with **board** generations — e.g. board **0.1** ships firmware **0.1.0**, then **0.1.1**, **0.1.2**, … until a **1.0** board ships with firmware **1.0.0**, then **1.1.0**, **1.1.1**, etc. Patch bumps are fine-grained firmware releases; minor/major track product/board tiers.
- **Update hint on device:** After **STA** is up, firmware does **one** HTTPS fetch per boot of the published **`manifest.json`** (default URL in **`platformio.ini`**: GitHub Pages installer). If the hosted **`version`** is **newer** (numeric **major.minor.patch**), the portal shows a short banner with a link to the USB installer. No OTA install — users still flash from the browser. Forks can override **`AZIMUTH_RELEASE_MANIFEST_URL`** / **`AZIMUTH_RELEASE_FLASHER_URL`**. The TLS client uses **certificate validation off** for that single read-only check (no credentials).
- **USB flasher vs plugged-in board:** [esp-web-tools](https://github.com/esphome/esp-web-tools) can show the **currently running** firmware name/version when the device implements **Improv Serial** and **`new_install_improv_wait_time`** is non-zero. Azimuth does **not** implement Improv yet and keeps that wait at **0**, so the web installer does **not** auto-compare the connected board to the hosted build; use the **Device** line on the portal for **FW** and the **update** banner when you’re on Wi‑Fi.

### CI (GitLab) and browser flasher (GitHub Pages)

| What | Where |
|------|--------|
| **GitLab pipeline** | Pushes to **`main`** run **`.gitlab-ci.yml`**: builds **`azimuth_main`** using **`include/secrets.h.example`** → **`include/secrets.h`** in CI. **Artifacts** → **`ci-artifacts/firmware/`** (`bootloader.bin`, `partitions.bin`, `boot_app0.bin`, `firmware.bin`). |
| **GitHub Pages USB flasher** | Workflow **`.github/workflows/github-pages-flasher.yml`** runs on **`main`**, builds the same env, runs **`scripts/prepare_web_flasher_firmware.sh`** (syncs **`manifest.json`** **`version`** from **`VERSION`**), and deploys **`web-flasher/`** (USB installer using [esp-web-tools](https://github.com/esphome/esp-web-tools)). The flasher page shows that **`version`**. In the repo: **Settings → Pages → Build and deployment → Source: GitHub Actions**. The deploy job uses GitHub Environment **`azimuth-flasher`**. Users need **Chrome** or **Edge** (Web Serial) and a **USB data** cable. When the installer offers **erase flash**, use it for a **factory-clean** device (clears NVS like the portal **Advanced → Reset all settings**). The page links to **`http://azimuth.local:8080/`** for the settings portal after install. |

Local check: `pio run -e azimuth_main` then `./scripts/prepare_web_flasher_firmware.sh` copies binaries into **`web-flasher/firmware/`** for testing.

Requires [PlatformIO](https://platformio.org/). **Default environment:** **`azimuth_main`** (release). Use **`azimuth_debug`** for serial-only bring-up.

**Debug (serial monitor, yaw/pitch/roll):**

```bash
python3 -m platformio run -e azimuth_debug -t upload
python3 -m platformio device monitor
```

**OpenTrack — Hatire (USB) + optional UDP (WiFi):**

```bash
python3 -m platformio run -e azimuth_main -t upload
```

Most users configure Wi‑Fi and OpenTrack **only in the portal** (NVS) and leave **`include/secrets.h`** empty (copy from **`secrets.h.example`** for a valid build; file is **gitignored**). You *may* set **`WIFI_SSID`**, **`WIFI_PASSWORD`**, **`OPENTRACK_UDP_HOST`** there as compile‑time defaults when NVS has no SSID. If there is **no usable home SSID** (NVS + `secrets.h`), or **STA fails** (wrong password, AP missing), the board opens **`Azimuth-Setup`** and a **captive portal**: HTTP **port 80** redirects to **`http://192.168.4.1/`** (same UI as on the LAN). **`azimuth.local` does not apply** on that AP—mDNS starts after joining home Wi‑Fi. Normal use: **`http://azimuth.local:8080`** or **`http://<LAN-IP>:8080`**. UDP port **`OPENTRACK_UDP_PORT`** is in **`platformio.ini`** (default **4242**).

- **USB:** Input **Hatire Arduino**, **115200**, **DTR** on; start tracking and **recenter** after the filter settles. Do not leave a text serial monitor open on that port.
- **Hatire axis mapping (important):** With the **portal default** axis map (Yaw→**Rot 0**, Roll→**Rot 1**, Pitch→**Rot 2** with pitch inverted in firmware), set OpenTrack Hatire to **Yaw→0, Roll→1, Pitch→2** (some UIs say “axis 0 / 1 / 2”). If you change **Rot 0–2** assignments in the portal, match OpenTrack’s inputs to those slots. USB and UDP use the same mapping.
- **WiFi / UDP:** Input **UDP over network**, same port as in **`platformio.ini`** (`OPENTRACK_UDP_PORT`); allow the port through the PC firewall. Hatire and UDP both run from the same firmware.

### On-device settings (WiFi + OpenTrack)

With **`azimuth_main`**:

- **Already on your LAN:** open **`http://<hostname>.local:8080`** (default hostname **`azimuth`**) or **`http://<device-ip>:8080`**.
- **Provisioning (`Azimuth-Setup`):** join that network (no password). Many phones **open the settings page automatically**; if not, go to **`http://192.168.4.1/`** on **port 80**. After you save a real SSID (and password if needed), the device **reboots** into **station-only** mode—**`Azimuth-Setup` does not stay on**.

The portal is grouped into **Wi‑Fi**, **LAN & discovery**, **OpenTrack (PC)**, **Tracking & radio**, **Device**, and **Advanced**:

| Section | What you can change |
|--------|---------------------|
| **Wi‑Fi** | SSID / password, **Scan networks** (brief tracking hitch). New Wi‑Fi credentials → **reboot**. |
| **LAN & discovery** | **mDNS** on/off, **device hostname** (letters, digits, hyphen; max 24). Changing these → **reboot** so DHCP/mDNS apply. |
| **OpenTrack (PC)** | **USB Hatire** on/off (Wi‑Fi‑only use), **UDP** on/off, **UDP address** / **port**, **axis mapping** (which fusion yaw/pitch/roll feeds Hatire/UDP **Rot 0–2**, each axis once, optional **invert** per slot). **`something.local` (mDNS)** often **does not** resolve from the ESP32; prefer numeric LAN IP or a DHCP hostname. **This browser’s IP** + **Fill address** when the portal is opened on the PC running OpenTrack. |
| **Tracking & radio** | **IMU report interval** (5 / 10 / 20 / 40 ms → 200 / 100 / 50 / 25 Hz). Change → **reboot** so the BNO08x report rate is reapplied. **Wi‑Fi TX power** (low / balanced / high) applies on save without reboot. |
| **Device** | Firmware **version** string (from repo **`VERSION`** at build time), **Save** / **Reboot**, status line. If a **newer** build is published on GitHub Pages, a one-time-per-boot check may show an **update** banner (link to USB installer). |
| **Advanced** | Collapsible **Reset to factory defaults** — clears NVS `azimuth` and reboots (provisioning AP if no home SSID in NVS / `secrets.h`). |

**NVS** (`Preferences` **`azimuth`**) is the normal source of truth; unset fields fall back to **`include/secrets.h`** (often empty). **`AZIMUTH_FW_VERSION`** for **`azimuth_main`** comes from the first line of repo **`VERSION`** via **`scripts/pio_set_version.py`** (see **Firmware version** above); **`platformio.ini`** only sets release **manifest / flasher URLs** and build options.

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
| **Input** | **Hatire Arduino** (COM port, **115200**, **DTR** on) **or** **UDP over network** (OpenTrack listens on **`OPENTRACK_UDP_PORT`**, usually **4242**; set UDP target in the **portal** or **`secrets.h`**; allow UDP in the firewall). |
| **Hatire axes** | With default portal mapping: **Yaw→0, Roll→1, Pitch→2** (see bullet above). |
| **Filter** | **Natural motion** filter (name in the filter dropdown may vary slightly by OpenTrack version; pick the **Natural motion** / natural-style preset if available). |
| **Responsiveness** | Turn **responsiveness** up to **maximum** (slider all the way up) so head motion matches the tracker with minimal lag. |
| **Start** | Click **Start**; use **Center** / **recenter** after the filter settles. |

Tweak filter and output mapping per game if needed; the table above is the baseline for Azimuth.

### Thermal / power (why the board feels warm)

A **Seeed XIAO ESP32-C3** runs warm in this use case mostly because of **Wi‑Fi**, not because UDP is “heavy.” The radio stays associated with your AP, listens for beacons, and transmits ~100 packets/s — that RF chain draws far more than the CPU spent packing 48-byte datagrams. **USB serial** adds a bit more. A small board with little heatsinking **will feel hot to the touch** when Wi‑Fi is up; that’s normal for this class of module.

While **STA** is up and the **settings server** is enabled, firmware calls **`WiFi.setSleep(false)`** so **modem sleep does not starve mDNS** (`azimuth.local`); that keeps the radio in a **higher-duty** regime than “modem sleep” marketing figures. **`yield()`** when the IMU has no event yet avoids spinning the CPU between samples. **TX power** is user-selectable in the portal (**Tracking & radio**): **low (~2 dBm)**, **balanced (~8.5 dBm, default)**, **high (~19.5 dBm)** — pick **high** only if UDP is flaky at distance.

### Battery runtime (estimate)

These are **rough order-of-magnitude** figures—not measured on your Azimuth PCB. Use them for planning; **measure** pack current at 3.7 V for a real product number.

**Seeed XIAO ESP32-C3** (Wi‑Fi on, from the [Getting Started specs](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)): **~75 mA** “active” Wi‑Fi, **~25 mA** modem-sleep. **Wi‑Fi dominates**; the IMU adds on the order of **a few mA** (typical for BNO08x-class fusion at ~100 Hz). **`azimuth_main`** keeps **modem sleep off** on STA while the portal stack is active (see above), so **budget closer to active/associated** figures for wireless use, not the low modem-sleep line item alone.

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
- **`include/opentrack_pose.h`** — Fusion Euler (deg) → Hatire / OpenTrack UDP **Rot 0–2** with NVS‑configurable **per‑slot axis + invert** (defaults match README **Yaw→0, Roll→1, Pitch→2** with pitch negated).
- **`src/track_network.cpp`** — Full Wi‑Fi / portal / OpenTrack UDP in **`azimuth_main`**; no‑op stubs in **`azimuth_debug`** (`IMU_DEBUG_MODE`).
- **`src/portal_html.cpp`** — PROGMEM settings UI (built only in **`azimuth_main`**; excluded from **`azimuth_debug`** via `build_src_filter`).
- **`platformio.ini`** — `espressif32`, `seeed_xiao_esp32c3`, **SparkFun BNO08x** library; **`azimuth_main`** adds **`scripts/pio_set_version.py`**, **`ArduinoJson`**, and default **release manifest / flasher URLs** (overridable for forks).
- **`VERSION`** — one-line semver for **`azimuth_main`** builds and **`web-flasher/manifest.json`** (via sync scripts / CI).
- **`include/secrets.h`** — optional compile‑time Wi‑Fi / OpenTrack defaults (copy from `secrets.h.example`; gitignored).

If you move SPI off the default D8–D10 pins, call `SPI.begin(sck, miso, mosi, -1)` **before** `imu.beginSPI(...)` so the bus matches your board (the SparkFun driver initializes `SPI` internally; ESP32 keeps an already-started bus).
