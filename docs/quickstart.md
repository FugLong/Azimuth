# Azimuth quick start

Short path from a fresh flash to head tracking in **OpenTrack** over Wi‑Fi.

**Full manual (recommended):** [**User Guide**](user-guide.md)

Assumes firmware is already installed (web flasher supports both hardware targets; `DIY` is the default selection). **Hardware paths** and which build to flash: [**README**](../README.md), [**hardware-profiles.md**](hardware-profiles.md). Deeper portal and OpenTrack detail: [**using-azimuth.md**](using-azimuth.md).

## 1. Join your home Wi‑Fi (first setup)

1. Power the board on. On your phone or computer, connect to the **Azimuth-Tracker** Wi‑Fi network (**Offline Mode**).
2. A setup page should open automatically, or open **`http://192.168.4.1/`** in a browser.
3. Enter your **home Wi‑Fi name** and **password**.
4. **Scroll to the bottom** of the page and **save** (apply). The board will connect to your network and restart.

## 2. Tell the tracker where OpenTrack runs

Do this on the **same computer** where you use **OpenTrack**, on your **home Wi‑Fi** (not on **Azimuth-Tracker** Offline Mode).

1. Open **`http://azimuth.local:8080/`** in a browser.  
   If that fails, use **`http://<device-ip>:8080`** (find the device in your router’s list). The address must end in **`:8080`**.
2. Turn **UDP to PC** **on** if it is off.
3. Click **Fill address** (the page fills in this PC’s address for you). If you don’t see that button, type this PC’s **IPv4** (e.g. `192.168.1.x`) in **UDP address** instead.
4. **Scroll down** and **save**.

## 3. OpenTrack

1. Input: **UDP over network** (wording may vary slightly by version).
2. Port: match the **UDP port** in the portal (default **4242** unless you changed it).
3. If **Windows** asks, allow the port through the firewall.

Move the tracker—you should see motion. **Recenter** in OpenTrack after things settle.

## Firmware updates

On **home Wi‑Fi**, the portal may show a banner if a **newer** version is published on GitHub Pages (about one check per boot). You can:

- **Install over Wi‑Fi** from the portal (**Device** section or banner), or **hold FUNC ~2 s** on the Azimuth PCB (see [**User Guide → Updates**](user-guide.md#9-firmware-updates)), or
- Reflash over **USB** with the same [**web flasher**](https://fuglong.github.io/Azimuth/) (Chrome or Edge).

Wireless install is blocked in **Offline mode**, when the chip is **too hot**, or when **battery is very low** off USB — use USB in those cases.

## More help

- **[User Guide](user-guide.md)** — complete manual (Wi‑Fi, portal, FUNC, battery, troubleshooting).
- **[README](../README.md)** — project overview and doc index.
- **[Using Azimuth](using-azimuth.md)** — portal details, OpenTrack (USB Hatire + UDP), tips.
- **[Development](development.md)** — build from source, CI, versioning, repo layout.
- **Browser flasher:** [https://fuglong.github.io/Azimuth/](https://fuglong.github.io/Azimuth/) (Chrome or Edge).
