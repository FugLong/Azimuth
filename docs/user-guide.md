# Azimuth User Guide

This is the **manual for using** Azimuth: Wi‑Fi setup, OpenTrack, the on-device web UI, the FUNC button, updates, and battery behavior. You do **not** need PlatformIO or the source code to follow it.

**Ultra-short path:** [quickstart.md](quickstart.md)  
**Deeper reference** (same topics, more detail): [using-azimuth.md](using-azimuth.md)  
**Builders / firmware from source:** [development.md](development.md)

---

## Two hardware versions

| Version | What it is |
|---------|------------|
| **DIY** | Seeed **XIAO ESP32‑C3** + **BNO08x** breakout (breadboard or hand-wired). Default in the web installer. |
| **Azimuth PCB** | Integrated board ([KiCad project](../kicad/Azimuth_Design/)) with RGB LED, **FUNC** button, buzzer, and **JST** for a LiPo. |

The **same tracking firmware** runs on both where features overlap. The DIY build may use a simple **status LED** instead of RGB; the PCB has full **sound and light** feedback.

---

## What you need

| Item | Why |
|------|-----|
| A **PC** running **OpenTrack** | Receives head tracking over **UDP** (Wi‑Fi) or **USB Hatire**. |
| **Chrome** or **Edge** | Required for the **USB web flasher** (Web Serial). |
| Your **Wi‑Fi** name and password | Stored only on the device. |
| Optional: **1S LiPo** (PCB or DIY pads) | Untethered use; see **Battery** below and [parts-list.md](parts-list.md). |

---

## 1. Install firmware over USB (first time or recovery)

1. Open the **Azimuth web flasher**: [https://fuglong.github.io/Azimuth/](https://fuglong.github.io/Azimuth/)
2. Connect the board with a **data** USB cable.
3. Choose **DIY** or **PCB** to match your hardware, then install.
4. If the installer offers **erase flash**, use it for a clean install (same idea as a full factory reset).

---

## 2. Connect the board to your home Wi‑Fi

Do this once (or whenever you change networks).

1. Power the board.
2. On your **phone or PC**, join the Wi‑Fi network **`Azimuth-Tracker`** (this is **Offline / setup mode**).
3. A setup page may open automatically. If not, open **`http://192.168.4.1/`** in a browser (port **80** — no `:8080` here).
4. Enter your **home Wi‑Fi name** and **password**.
5. **Scroll down** and **save**. The board reboots and joins your network.

If it cannot connect (wrong password, weak signal), it will come back to **`Azimuth-Tracker`** so you can fix credentials without re-flashing.

---

## 3. Open the settings portal (after Wi‑Fi works)

Use the portal to set **UDP address**, **axis mapping**, **sound/light**, and more.

| Situation | Open in your browser |
|-----------|----------------------|
| Board on **home Wi‑Fi** | `http://azimuth.local:8080` or `http://192.168.x.x:8080` (router/device IP) |
| **Offline mode** (you are connected to **`Azimuth-Tracker`**) | **`http://192.168.4.1/`** on port **80** |

Important:

- **`azimuth.local` only works on your LAN** after the device has joined your router — not while you are connected only to **`Azimuth-Tracker`**.
- The portal on home Wi‑Fi uses port **`:8080`**. Forgetting it is a common mistake.
- **Windows:** If **`.local`** never resolves, install **Bonjour** (e.g. Apple Bonjour Print Services) or use the device **IP** from your router’s admin page.
- **Guest Wi‑Fi** or client isolation often blocks phone ↔ PC discovery — use the printed IP if needed.

---

## 4. Tell the tracker where OpenTrack runs

Do this on the **same PC** that runs OpenTrack, while that PC is on the **same home Wi‑Fi** as the board.

1. Open `http://azimuth.local:8080` (or `http://192.168.x.x:8080` using the board’s IP).
2. Enable **UDP to PC** if it is off.
3. Click **Fill address** if the button appears (it fills this PC’s LAN IPv4). Otherwise type your PC’s IPv4 (often `192.168.x.x`).
4. **Save** at the bottom of the page.

---

## 5. OpenTrack on the PC

Use **either** UDP **or** USB Hatire — **not both at once**.

### Wi‑Fi (usual)

1. Input: **UDP over network** (wording may vary by OpenTrack version).
2. Port: match the **UDP port** in the portal (default **4242** unless you changed it).
3. On **Windows**, allow the port through the firewall if prompted.
4. Click **Start** in OpenTrack, then **Center** / **recenter** after motion stabilizes.

### USB (Hatire)

1. In the portal you can enable **USB Hatire** and disable UDP if you want a wired-only path.
2. In OpenTrack, choose **Hatire Arduino**: **115200** baud, **DTR** on.
3. Do **not** leave a **serial monitor** open on the same COM port while OpenTrack is using it.

**Axis mapping:** Portal settings for **Rot 0–2** apply to **both** UDP and Hatire. Match OpenTrack’s expectations to those slots.

---

## 6. Physical controls (Azimuth PCB)

The **FUNC** button is the round tact switch on the board (**GPIO7**, active low).

| Gesture | Action |
|---------|--------|
| **Single tap** | Toggles **Pause** (see below). |
| **Double tap** | Reserved — no action. |
| **Long press (~2 s)** | Starts **wireless firmware update** (see **Updates**). You hear a short rising tune when the download begins; a normal tap is **not** counted for that gesture. |

**DIY builds** without FUNC ignore this section.

---

## 7. Pause (FUNC single tap)

**Pause** stops sending pose data to OpenTrack over **UDP** and **USB Hatire** until you tap again. It is meant for breaks without powering down.

- Pause is **session-only** (not saved permanently).
- You get **different buzzer cues** when entering vs leaving pause.
- The LED shows a **clear “paused” pattern** (depends on DIY vs RGB hardware).

**When pause or FUNC may not respond**

- **Severe overheating** — firmware may force Wi‑Fi off for safety; pause state is cleared as part of that.
- **Battery at about 1% or below** (with a battery connected) — FUNC **does not toggle** pause, to avoid trapping you in a bad state.
- While a **wireless update** is running, normal tap behavior is suppressed for that gesture.

---

## 8. Lights and sounds (overview)

Exact patterns depend on **DIY vs PCB**. In general:

- **Normal tracking** — Status LED or RGB “ambient” pattern from your **Sound & light** settings.
- **Pause** — Distinct slow blink / cyan-style pulse on RGB boards.
- **Wireless update** — Fast **cyan** throb (or fast blink on LED-only builds); start / success / failure tunes on PCB.
- **Thermal warning** — Firmware reduces radio activity to cool down; LED shows a **thermal** pattern until recovered.
- **Low battery** — Escalating cues (tones and LED overrides); see **Battery**.

Details for RGB mapping and buzzer motifs: [io-led-buzzer-plan.md](io-led-buzzer-plan.md).

---

## 9. Firmware updates

### How you know an update exists

On **home Wi‑Fi**, after the board connects, firmware checks the official **`manifest.json`** on GitHub Pages **about once per boot**. If the published **version** is **newer** than what runs on the chip, the portal shows a **yellow banner** (and the **Device** section shows version info).

### Two ways to update

| Method | When to use |
|--------|-------------|
| **Install over Wi‑Fi** (portal button) or **FUNC long-press** | Convenient when the board already has internet via your router. |
| **USB web flasher** ([same URL as install](https://fuglong.github.io/Azimuth/)) | Custom builds, bad Wi‑Fi, bricked networking, or you want a full erase. |

The board downloads the **official release** matching your hardware (**DIY** vs **PCB**) from the **same GitHub Pages site** the banner trusts. Transport is **HTTPS** with a **pinned certificate authority** (see maintainer docs — this is **not** the same as “signed firmware” inside the image; image signing is a possible future hardening step).

### When wireless install is **blocked**

Wireless update **does not start** if:

- You are in **Offline mode** / device AP only (**no route to the internet**).
- **Thermal protection** has shut down Wi‑Fi.
- **Battery is at 15% or below** and the board is **not** on USB power — flashing while the cell browns out could brick the device.

If wireless update fails, you still have the **USB flasher**.

---

## 10. Battery and charging

### What the percentage means

The device estimates **state of charge** from battery voltage (with smoothing and noise rejection). It is an **estimate**, not a lab coulomb counter.

In the portal **Device** section you can set:

- **Pack capacity (mAh)** — improves remaining‑time style readouts.
- **Calibration offset** — fine‑tune voltage reading for your pack.

If no battery is detected (USB‑only), telemetry may show **absent** / unsupported states depending on hardware.

### Alerts (typical behavior)

As charge drops, firmware plays **stepped low‑battery cues** (buzzer on PCB). Crossing thresholds near **25%, 15%, 10%, 5%… down to 1%** escalates urgency.

**Below ~1%** (with a battery present): panic‑style cues repeat; firmware may **shut down Wi‑Fi** to protect the system — **plug in USB** to recover.

### Wireless updates and battery

Wireless OTA is **refused** at **≤15%** unless **USB** provides power — plug in before updating.

### PCB LiPo connector

**Polarity and connector type matter.** Wrong packs or reversed JST wiring can damage the board. Follow [parts-list.md → Off-board pack](parts-list.md#off-board-pack-azimuth-pcb-wireless).

---

## 11. Power, heat, and Wi‑Fi tuning

The ESP32‑C3 feels **warm** when Wi‑Fi is active — that is normal on a small module.

In the portal **Tracking & radio**:

- **Wi‑Fi TX power** — lower can reduce heat and current; higher helps marginal signal.
- **IMU report interval** — affects how often orientation updates (smoothness vs load); changing it requires **reboot**.

Full detail: [power-and-thermal.md](power-and-thermal.md).

---

## 12. Troubleshooting

| Problem | Things to try |
|---------|----------------|
| **`azimuth.local` fails** | Use `http://192.168.x.x:8080`. Windows: install Bonjour or copy IP from router DHCP list. |
| **Portal won’t load on home Wi‑Fi** | Confirm **`:8080`**, same LAN as the PC, firewall not blocking. Ping or check router client list. |
| **Stuck in setup Wi‑Fi** | Rejoin **`Azimuth-Tracker`**, open **`http://192.168.4.1`**, fix SSID/password. |
| **OpenTrack gets no motion** | UDP: PC IP/port match portal; Windows firewall; only one input (UDP **or** Hatire). Hatire: correct COM, DTR on, no serial monitor on same port. |
| **Tracking feels wrong** | **Recenter** in OpenTrack; check **axis mapping** in portal vs OpenTrack filter pipeline. |
| **Update banner but Wi‑Fi install won’t start** | Plug into USB if battery low; ensure not in Offline AP; cool down if overheated. Use USB flasher if needed. |

---

## 13. Enclosure (3D-printed shell)

The **integrated PCB** can mount in a **3D-printed enclosure** (battery-friendly internal volume). Open the model on **Onshape** to rotate, measure, or export for printing: [Azimuth enclosure](https://cad.onshape.com/documents/fa78666ff0e219ba32d45d9f/w/679625cc5ba48f8e098cba83/e/0cd4f9cad8e872128d491b46).

---

## 14. Related documentation

| Doc | Contents |
|-----|----------|
| [quickstart.md](quickstart.md) | Shortest Wi‑Fi → OpenTrack path |
| [using-azimuth.md](using-azimuth.md) | Portal sections, RGB pins, extra detail |
| [power-and-thermal.md](power-and-thermal.md) | Modem sleep, portal polling, deep battery notes |
| [parts-list.md](parts-list.md) | BOM, LiPo sourcing, polarity |
| [README.md](../README.md) | Project overview and doc index |

---

*Azimuth is a DIY / open head tracker for [OpenTrack](https://github.com/opentrack/opentrack). No cloud account is required; configuration stays on your LAN and the device.*
