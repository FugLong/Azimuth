# Using Azimuth

How to reach the **settings portal**, connect **OpenTrack**, and interpret a few common behaviors. For the shortest path from a new flash to tracking, start with [**quickstart.md**](quickstart.md).

## Settings portal URLs

All **`azimuth_main_*`** builds share the same **portal**, **Hatire**, and **UDP** behavior. On-board **RGB**, **buzzer**, and **button** apply only where the hardware has them ([**hardware-profiles.md**](hardware-profiles.md)):

| Situation | Open in your browser |
|-----------|----------------------|
| Board already on your **home Wi‑Fi** | **`http://azimuth.local:8080`** or **`http://<device-ip>:8080`** (default hostname is **`azimuth`**) |
| **Offline Mode** (first setup, recovery, or off-grid use) — you joined **Azimuth-Tracker** | **`http://192.168.4.1/`** on port **80** (captive portal may open this for you) |

**`azimuth.local` does not work** in **Offline Mode** AP—only after the device joins your LAN as a client. If **`.local`** fails on Windows, install **Bonjour** (e.g. Apple Bonjour Print Services) or use the device **IP** from your router. Guest or isolated Wi‑Fi often blocks discovery; use the IP.

## Portal sections (what you can change)

The UI is grouped roughly as follows:

| Section | What it controls |
|---------|------------------|
| **Wi‑Fi** | SSID / password, **Scan networks** (brief tracking pause). New credentials → **reboot**. |
| **LAN & discovery** | mDNS on/off, **hostname** (letters, digits, hyphen; max 24). Changes → **reboot**. |
| **OpenTrack (PC)** | USB Hatire on/off, UDP on/off, **UDP address** / **port**, **axis mapping** (which yaw/pitch/roll go to **Rot 0–2**, optional **invert**). Use **Fill address** when you open the portal on the PC running OpenTrack. **`something.local` from the ESP32** is unreliable—prefer a numeric LAN IP. |
| **Tracking & radio** | **IMU report interval** (5–40 ms) and **Wi‑Fi TX power** (low / balanced / high). IMU interval change → **reboot**. |
| **Device** | Firmware **version**, battery telemetry (pack mV, raw ADC mV, % estimate, inferred charge/discharge/idle trend), battery capacity + calibration offset settings, save / reboot, status. On Wi‑Fi, a banner may link to the **USB web installer** if a newer build is published. |
| **Sound & light** (RGB / buzzer boards) | **RGB brightness**, **buzzer volume**, **LED mode** (rainbow, slow rainbow, status, **manual RGB** with 0–255 sliders + quick presets + live preview), and stored **`led_r` / `led_g` / `led_b`** in flash. System warnings (thermal, very low battery, setup AP, **pause**) can override the ambient LED until the condition clears. |
| **Advanced** | **Reset all settings** (clears stored config and reboots; may return to **Azimuth-Tracker** Offline Mode if no home SSID remains). |

Settings live in **flash (NVS)** on the device. If something is unset, the build can fall back to optional compile-time defaults in **`include/secrets.h`** (see [**development.md**](development.md)).

Saving **new Wi‑Fi** triggers a **reboot**. If the board cannot join that network, it falls back to **Azimuth-Tracker** Offline Mode so you can fix SSID/password without reflashing.

## Integrated PCB: RGB, FUNC, buzzer

On **`azimuth_main_pcb`**, the Azimuth board’s **common-anode** RGB uses **inverted** PWM on **IO0** (green, **R8**), **IO1** (red, **R7**), **IO3** (blue, **R6**), with simple ballast scaling in firmware vs those resistors (DIY builds without that LED layout keep a simple GPIO3 status line instead).

**FUNC** (**IO7**, pull-up, switch to GND): **single tap** toggles **Pause** (see below). **Double-tap** is reserved (no action). **Long press (~2 s)** triggers a **wireless firmware update** (see [Wireless updates](#wireless-updates-ota)).

## Pause (FUNC single tap)

**Pause** puts the tracker in **stasis**: it **stops sending OpenTrack UDP** and **USB Hatire** pose packets for as long as pause is on, plays a **short sound** on enter and a **different sound** on exit, forces **aggressive Wi‑Fi modem sleep** on STA when idle, and shows a **distinct LED pattern** (slow blink on DIY; cyan‑ish pulse on RGB) so you can leave the device powered without streaming to the PC.

- **Not saved in NVS** — pause is session-only. Your portal **UDP enabled** / **Hatire** preferences apply again after you resume.
- **USB serial** is unchanged (you can still flash or use a serial monitor; pause only affects the Hatire packet stream used by OpenTrack).
- **Safety** — you cannot enter pause while the pack is at **~1%** (critical) or after **thermal hold** has cut Wi‑Fi; an overheating event **clears** pause automatically because the network path is torn down.

Press **FUNC** once to pause, once again to resume.

## OpenTrack on the PC

Use **either** **Hatire Arduino** (USB) **or** **UDP over network** as the **input**—not both at once. You can disable USB Hatire in the portal for Wi‑Fi-only use; OpenTrack should still subscribe to only one path.

| Step | Action |
|------|--------|
| **Input** | **Hatire Arduino** (COM, **115200**, **DTR** on) **or** **UDP over network** (port matches portal / default **4242**; allow UDP in the OS firewall). |
| **Hatire axes** | With **default** portal mapping: **Yaw→0, Roll→1, Pitch→2** (wording may say “axis 0 / 1 / 2”). If you change **Rot 0–2** in the portal, match OpenTrack to those slots. USB and UDP share the same mapping. |
| **Filter** | Prefer a **natural motion**-style filter if your OpenTrack version offers it. |
| **Responsiveness** | Turn **responsiveness** up (e.g. maximum) so motion feels direct. |
| **Start** | **Start** tracking; **Center** / **recenter** after the filter settles. |

**USB Hatire:** Do not leave a **serial monitor** open on the same COM port while OpenTrack is using it.

**UDP:** The PC listens on the configured port; the portal sets where the ESP32 **sends** datagrams (your PC’s LAN IPv4).

## Firmware updates

### Wireless updates (OTA)

The board can pull the latest signed build from the same trusted GitHub Pages release the version banner already tracks, write it into the **standby OTA partition**, and reboot into it — no USB cable required.

| Trigger | What to expect |
|---------|----------------|
| **Portal button** | On home Wi‑Fi, the **Device** section (and the yellow update banner when a newer version is published) has an **Install over Wi‑Fi** button. A blue progress card appears showing percent / KB downloaded. |
| **FUNC long-press** (~2 s) | Hold **FUNC** for about two seconds — you'll hear a **rising 4-note tune** when the download starts. Single-tap pause is suppressed for that press. |

While the OTA runs, the LED switches to a **fast cyan throb** (or a fast blink on status-LED-only boards). On success the board plays a **resolved arpeggio** and reboots into the new image automatically; on failure it plays a **stern descending tone** and stays on the current image. Tracking is **paused** during the download to give all bandwidth and CPU to the fetch.

The board **refuses to start** an OTA when:

- It’s in **Offline-Mode AP** (no internet route).
- A **thermal-hold** event has cut Wi‑Fi.
- Battery is **≤15 %** and not on USB charge — a brownout mid-write would brick the device.

If wireless updates aren’t appropriate (no Wi‑Fi, lost device, or you want to flash a custom build), the **USB web flasher** is unchanged and remains the recovery path.

**USB flasher:** [https://fuglong.github.io/Azimuth/](https://fuglong.github.io/Azimuth/) (Chrome or Edge). Your running version appears under **Device** in the portal.

## Power, heat, and battery

Expect the module to feel **warm when Wi‑Fi is on** — that is normal for a small ESP32‑C3. Tune **Tracking & radio** in the portal: **Wi‑Fi TX power** and **IMU report interval** (IMU interval affects tracking smoothness and load, not just “power”).

**Everything the firmware does to save power and heat** (modem sleep, network loop scheduling, portal polling, defaults): see [**power-and-thermal.md**](power-and-thermal.md). **Rough battery runtime guesses** and **which LiPo packs match the Azimuth PCB JST** are in that doc too, with full sourcing detail in [**parts-list**](parts-list.md#off-board-pack-azimuth-pcb-wireless).

---

**Related:** [quickstart.md](quickstart.md) · [power-and-thermal.md](power-and-thermal.md) · [development.md](development.md) · [README](../README.md)
