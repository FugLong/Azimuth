# Using Azimuth

How to reach the **settings portal**, connect **OpenTrack**, and interpret a few common behaviors. For the shortest path from a new flash to tracking, start with [**quickstart.md**](quickstart.md).

## Settings portal URLs

All **`azimuth_main_*`** builds share the same **portal**, **Hatire**, and **UDP** behavior. On-board **RGB**, **buzzer**, and **button** apply only where the hardware has them ([**hardware-profiles.md**](hardware-profiles.md)):

| Situation | Open in your browser |
|-----------|----------------------|
| Board already on your **home Wi‑Fi** | **`http://azimuth.local:8080`** or **`http://<device-ip>:8080`** (default hostname is **`azimuth`**) |
| **First setup** or recovery — you joined **Azimuth-Setup** | **`http://192.168.4.1/`** on port **80** (captive portal may open this for you) |

**`azimuth.local` does not work** on the setup access point—only after the device joins your LAN as a client. If **`.local`** fails on Windows, install **Bonjour** (e.g. Apple Bonjour Print Services) or use the device **IP** from your router. Guest or isolated Wi‑Fi often blocks discovery; use the IP.

## Portal sections (what you can change)

The UI is grouped roughly as follows:

| Section | What it controls |
|---------|------------------|
| **Wi‑Fi** | SSID / password, **Scan networks** (brief tracking pause). New credentials → **reboot**. |
| **LAN & discovery** | mDNS on/off, **hostname** (letters, digits, hyphen; max 24). Changes → **reboot**. |
| **OpenTrack (PC)** | USB Hatire on/off, UDP on/off, **UDP address** / **port**, **axis mapping** (which yaw/pitch/roll go to **Rot 0–2**, optional **invert**). Use **Fill address** when you open the portal on the PC running OpenTrack. **`something.local` from the ESP32** is unreliable—prefer a numeric LAN IP. |
| **Tracking & radio** | **Power profile** (performance / balanced / battery saver), IMU report interval (5–40 ms), and **Wi‑Fi TX power** (low / balanced / high). IMU interval change → **reboot**. |
| **Device** | Firmware **version**, battery state (currently **stub** until battery monitoring is wired), save / reboot, status. On Wi‑Fi, a banner may link to the **USB web installer** if a newer build is published. |
| **Advanced** | **Reset all settings** (clears stored config and reboots; may return to **Azimuth-Setup** if no home SSID remains). |

Settings live in **flash (NVS)** on the device. If something is unset, the build can fall back to optional compile-time defaults in **`include/secrets.h`** (see [**development.md**](development.md)).

Saving **new Wi‑Fi** triggers a **reboot**. If the board cannot join that network, it falls back to **Azimuth-Setup** so you can fix SSID/password without reflashing.

## Integrated PCB: RGB, FUNC, buzzer

On **`azimuth_main_pcb`**, the Azimuth board’s **common-anode** RGB (cathodes on **IO0 / IO1 / IO3**) is driven with **inverted** PWM in firmware so colors match the schematic. While running, firmware shows a **slow rainbow** on the RGB LED as an easy visual that the board is alive (DIY builds without that LED layout keep a simple GPIO3 status line instead).

**FUNC** (**IO7**, pull-up, switch to GND) triggers a **short four-note chime** on the buzzer (**IO21** → FET on the PCB). This is for feedback and bring-up; behavior may be extended later (e.g. recenter, menu).

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

On home Wi‑Fi, firmware may do **one** check per boot against the published installer metadata. If a **newer** version exists, the portal can show a link to the **USB web flasher** (updates are still **USB-only**, not over-the-air install). Your running version appears under **Device** on the portal; the flasher page shows the version it ships.

**Flasher:** [https://fuglong.github.io/Azimuth/](https://fuglong.github.io/Azimuth/) (Chrome or Edge).

## Power, heat, and battery

Expect the module to feel **warm when Wi‑Fi is on** — that is normal for a small ESP32‑C3. Tune **Tracking & radio** in the portal: **Power profile**, **Wi‑Fi TX power**, and **IMU report interval** (IMU interval affects tracking smoothness and load, not just “power”).

**Everything the firmware does to save power and heat** (modem sleep, network loop scheduling, portal polling, defaults): see [**power-and-thermal.md**](power-and-thermal.md). **Rough battery runtime guesses** and **which LiPo packs match the Azimuth PCB JST** are in that doc too, with full sourcing detail in [**parts-list**](parts-list.md#off-board-pack-azimuth-pcb-wireless).

---

**Related:** [quickstart.md](quickstart.md) · [power-and-thermal.md](power-and-thermal.md) · [development.md](development.md) · [README](../README.md)
