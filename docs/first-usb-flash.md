# First-time USB flash (copy & paste)

Use this guide when you need to put Azimuth firmware on the board over **USB** using our flash script.

**You need this if:** your **custom Azimuth PCB** came from assembly with **no firmware** (the web browser flasher usually will not work until after this one-time flash). You can also use it for **DIY (XIAO + breakout)** anytime.

**After this works once:** use the normal [**web flasher**](https://fuglong.github.io/Azimuth/) or **Wi‑Fi updates** in the portal — you do not need this script every time.

---

## Before you start

Check all of these:

| | |
|---|---|
| ☐ | **Data USB cable** (charges phones *and* transfers data — not charge-only) |
| ☐ | **Computer** (Mac or Windows; Linux works too — same commands as Mac) |
| ☐ | **Python 3** installed ([python.org/downloads](https://www.python.org/downloads/) — on Windows, check **“Add python to PATH”**) |
| ☐ | You know which board you have (see next section) |

Plug the board into the computer with USB **before** you run the flash command (or plug in when the script asks you to pick a port).

---

## Step 1 — Which board do you have?

Pick **one** path below. Do not run both commands.

| Your hardware | Use this path |
|---------------|----------------|
| **Azimuth custom PCB** (the integrated Azimuth board from the kit/PCB) | [**Custom PCB**](#step-2-flash-custom-pcb) |
| **DIY** — Seeed **XIAO ESP32-C3** + BNO08x breakout on a breadboard | [**DIY XIAO**](#step-2-flash-diy-xiao--bno08x) |

---

## Step 2 — Flash

### Mac or Linux

1. Open **Terminal**  
   - Mac: Spotlight (⌘ Space) → type `Terminal` → Enter  
2. **Copy** the whole block for your board (one click in the box, ⌘C / Ctrl+C).  
3. **Paste** into Terminal (⌘V / Ctrl+V) and press **Enter**.  
4. Wait. First run may install a small tool (`esptool`) — that is normal.  
5. When it says **Flash complete**, you are done. Unplug and plug USB once if the board does not reboot on its own.

#### Flash — Custom PCB

```bash
curl -fsSL https://raw.githubusercontent.com/FugLong/Azimuth/main/scripts/azimuth-flash.sh | bash
```

#### Flash — DIY (XIAO + BNO08x)

```bash
curl -fsSL https://raw.githubusercontent.com/FugLong/Azimuth/main/scripts/azimuth-flash.sh | bash -s diy
```

---

### Windows

1. Install **Python 3** from [python.org/downloads](https://www.python.org/downloads/) if you do not have it.  
   - On the installer screen, turn on **“Add python.exe to PATH”**, then **Install Now**.  
2. Open **PowerShell**  
   - Start menu → type `PowerShell` → **Windows PowerShell**  
3. Plug in the board with a **data** USB cable.  
4. **Copy** the block for your board, **paste** into PowerShell, press **Enter**.  
5. Wait until you see **Flash complete**.

#### Flash — Custom PCB

```powershell
curl.exe -fsSL https://raw.githubusercontent.com/FugLong/Azimuth/main/scripts/azimuth-flash.py -o azimuth-flash.py
python azimuth-flash.py pcb
```

#### Flash — DIY (XIAO + BNO08x)

```powershell
curl.exe -fsSL https://raw.githubusercontent.com/FugLong/Azimuth/main/scripts/azimuth-flash.py -o azimuth-flash.py
python azimuth-flash.py diy
```

If Windows says it cannot find the USB port, open **Device Manager** → **Ports (COM & LPT)** → note something like **COM3**, then run (change `COM3` if yours is different):

```powershell
python azimuth-flash.py pcb --port COM3
```

(or `diy` instead of `pcb` for the XIAO build)

---

## Step 3 — What you should see

While it runs, the window will show things like:

- `Downloading…` / `Firmware 0.x.x ready`  
- `Using port …` (or it asks you to pick `[1]` / `[2]` if several USB devices are plugged in)  
- `Erasing flash…`  
- `Writing firmware…`  
- **`Flash complete`**

Then:

1. Power the board (USB is fine).  
2. On your phone or PC Wi‑Fi list, look for **`Azimuth-Tracker`**.  
3. Connect and open **http://192.168.4.1** to enter your home Wi‑Fi.  
4. Full setup: [**quick start**](quickstart.md) · [**user guide**](user-guide.md)

---

## Stuck?

### `python3: command not found` (Mac)

Install Python from [python.org/downloads](https://www.python.org/downloads/), then try the command again.

### `python is not recognized` (Windows)

Re-run the Python installer and enable **Add to PATH**, or use `py` instead of `python`:

```powershell
py azimuth-flash.py pcb
```

### No serial port / flash failed / keeps retrying “bootloader”

**Custom PCB:** press **RST** once, then run the flash command again **right away**.

**DIY XIAO:** hold the **BOOT** button, tap **RST**, release **RST**, keep holding **BOOT**, run the command again.

Also try: different USB cable, different USB port, unplug other serial devices, run the command again right after plugging in.

### Several USB devices — which one?

Run with “ask me” (Mac/Linux, from a cloned repo only):

```bash
./scripts/azimuth-flash.sh --ask
```

Or on Windows, list ports in Device Manager and use `--port COMx` as shown above.

### I already have the Azimuth repo on my computer

Open Terminal in the repo folder and run:

```bash
./scripts/azimuth-flash.sh          # custom PCB
./scripts/azimuth-flash.sh diy      # XIAO DIY
```

---

## Technical notes (optional)

- Downloads the same official firmware as [fuglong.github.io/Azimuth](https://fuglong.github.io/Azimuth/).  
- Caches files under `~/.cache/azimuth-flash/` (Mac/Linux).  
- Developers building their own firmware: `./scripts/flash.sh` in the repo.
