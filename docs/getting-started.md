# Getting started

How to build and flash the firmware in this repository with PlatformIO.

- [What changed and why](#what-changed-and-why)
- [1. Install PlatformIO Core](#1-install-platformio-core)
- [2. USB driver](#2-usb-driver)
- [3. Clone and first build](#3-clone-and-first-build)
- [4. Build, upload and monitor](#4-build-upload-and-monitor)
- [5. Optional: VS Code or CLion](#5-optional-vs-code-or-clion)
- [Troubleshooting](#troubleshooting)

---

## What changed and why

The project moved from the Arduino IDE to PlatformIO because we need to build several
firmwares (master, slave and practice sketches) from one repository with a shared
library. All build settings, including the pinned ESP32 platform version, now live in
`platformio.ini`, so every team member builds with the same configuration. The Arduino
IDE is no longer used; `.ino` sketches are now `main.cpp` files under `src/`.

---

## 1. Install PlatformIO Core

PlatformIO Core is the `pio` command-line tool. You need it even if you plan to use an
editor plugin later.

### Windows

1. Install Python from <https://www.python.org/downloads/>. In the installer, tick
   **Add python.exe to PATH**.
2. In PowerShell:

   ```powershell
   py -m pip install --user platformio
   py -c "import site; print(site.getuserbase() + '\Scripts')"
   ```

3. The second command prints a folder, for example
   `C:\Users\<you>\AppData\Roaming\Python\Python312\Scripts`. Add that folder to your
   `PATH`: Start → *Edit environment variables for your account* → **Path** → **Edit** →
   **New** → paste the folder → **OK**.
4. Close and reopen PowerShell.

### macOS — Homebrew (recommended)

```bash
brew install platformio
```

### macOS — pip fallback

Use this if you do not have Homebrew.

```bash
python3 -m pip install --user platformio
echo 'export PATH="$(python3 -m site --user-base)/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

The second line adds pip's user `bin` folder to your `PATH`, so zsh can find `pio`.

If pip fails with `externally-managed-environment`, your Python comes from Homebrew and
does not allow global installs. Use pipx instead:

```bash
brew install pipx
pipx ensurepath
pipx install platformio
```

Then open a new terminal.

### Verify

```bash
pio --version
```

Expected output: `PlatformIO Core, version 6.x.x`. If you get `'pio' is not recognized`
(Windows) or `command not found` (macOS), see [Troubleshooting](#troubleshooting).

---

## 2. USB driver

The ESP32 board talks to your computer through a USB-to-serial chip. Most boards use one
of two chips:

| Chip | How to recognise it | Driver |
|---|---|---|
| **CH340** (WCH) | Small rectangular chip near the USB port, marked `CH340C`, `CH340G` or `CH340K` | Windows: *CH341SER* from <https://www.wch-ic.com>. macOS: usually built in |
| **CP210x** (Silicon Labs) | Small square chip near the USB port, marked `CP2102` or `CP2104` (often with the SiLabs logo) | Windows: *CP210x Universal Windows Driver* from <https://www.silabs.com> (often installed automatically by Windows Update). macOS: usually built in |

The marking is tiny; use your phone camera to zoom in.

### Check that the port is visible

**Windows** — plug the board in, then open **Device Manager** → **Ports (COM & LPT)**.
You should see something like `USB-SERIAL CH340 (COM3)` or
`Silicon Labs CP210x USB to UART Bridge (COM4)`. A device with a yellow warning triangle
under *Other devices* means the driver is missing.

**macOS** — plug the board in, then:

```bash
ls /dev/cu.*
```

Look for a new entry such as `/dev/cu.usbserial-0001`, `/dev/cu.wchusbserial1420` or
`/dev/cu.SLAB_USBtoUART`. Ignore `cu.Bluetooth-Incoming-Port` and `cu.debug-console`.
Not sure which one is the board? Run the command with the board unplugged and again with
it plugged in, and compare.

**Both** — PlatformIO can list ports too:

```bash
pio device list
```

---

## 3. Clone and first build

```bash
git clone https://github.com/ahmad-sy1/slimme-verkeersregeling.git
cd slimme-verkeersregeling
pio run -e blink
```

`pio run` only compiles; **no board needs to be connected**. This is the quickest way to
check that your setup works.

The first build downloads the ESP32 toolchain and framework (a few hundred MB) into
`%USERPROFILE%\.platformio` (Windows) or `~/.platformio` (macOS). That takes a few
minutes; later builds take seconds.

Build every environment at once:

```bash
pio run
```

A working setup ends with:

```
Environment       Status    Duration
----------------  --------  ------------
master            SUCCESS   ...
slave             SUCCESS   ...
blink             SUCCESS   ...
mac_address       SUCCESS   ...
espnow_master     SUCCESS   ...
espnow_slave      SUCCESS   ...
signal_head_test  SUCCESS   ...
all_on            SUCCESS   ...
pin_scan          SUCCESS   ...
led_voltage       SUCCESS   ...
========================= 10 succeeded in ... =========================
```

Build output goes to `.pio/`, which is in `.gitignore`.

---

## 4. Build, upload and monitor

Each environment in `platformio.ini` builds one folder under `src/`. Pass the environment
name with `-e`.

| Environment | Source folder | What it is for |
|---|---|---|
| `master` | `src/master/` | Master controller: runs the fixed-timing cycle and broadcasts it to the slaves |
| `slave` | `src/slave/` | Slave controller, for both slaves: picks its pin table by MAC address and switches its LEDs |
| `blink` | `src/basics/blink/` | Practice: blinks the built-in LED on GPIO 2. Use it to check that uploading works |
| `mac_address` | `src/basics/mac_address/` | Practice: prints this board's MAC address on the serial monitor |
| `espnow_master` / `espnow_slave` | `src/basics/espnow_*/` | Practice: ESP-NOW greeting between two boards |
| `signal_head_test` | `src/basics/signal_head_test/` | Hardware test: walks every signal head of a slave (red, orange, green) |
| `all_on` | `src/basics/all_on/` | Hardware test: switches all 12 slave LEDs on |
| `pin_scan` | `src/basics/pin_scan/` | Hardware test: drives chosen pins through serial commands |
| `led_voltage` | `src/basics/led_voltage/` | Hardware test: measures the voltage over each LED |

### Commands

```bash
pio run -e <env>                          # compile only
pio run -e <env> -t upload                # compile and upload to the board
pio device monitor -e <env>               # open the serial monitor (exit: Ctrl+C)
pio run -e <env> -t upload -t monitor     # upload, then open the monitor
```

Example: read the MAC address of a board.

```bash
pio run -e mac_address -t upload -t monitor
```

The monitor uses `monitor_speed = 115200` from `platformio.ini`, so you do not need to
set the baud rate. The MAC address is printed once at start-up; press the **EN** (reset)
button on the board to print it again.

`blink` and `all_on` only show their result on the LEDs; the other environments print to
the serial monitor. See [`wiring-check.md`](wiring-check.md) for the hardware test tools.

### More than one board connected

PlatformIO picks a port automatically. With several boards connected, name the port:

```bash
pio run -e blink -t upload --upload-port COM3                     # Windows
pio run -e blink -t upload --upload-port /dev/cu.usbserial-0001   # macOS
pio device monitor -e mac_address --port COM3
```

### Clean build

```bash
pio run -e <env> -t clean
```

---

## 5. Optional: VS Code or CLion

Both editors use the same `platformio.ini`, so the environments and commands above stay
the same. Editor-generated folders (`.vscode/`, `.idea/`) are in `.gitignore`; do not
commit them.

### VS Code

1. Install the **PlatformIO IDE** extension from the Extensions panel.
2. **File → Open Folder…** and choose the `slimme-verkeersregeling` folder (the one that
   contains `platformio.ini`).
3. Pick the environment in the blue status bar at the bottom (the `env:` switcher), then
   use the status bar icons: ✓ build, → upload, plug icon for the serial monitor.

### CLion

1. **Settings → Plugins** → install **PlatformIO for CLion**. It uses the PlatformIO Core
   you installed in step 1.
2. **File → Open…** and choose the `slimme-verkeersregeling` folder.
3. Build and upload through the PlatformIO tool window or the run configurations the
   plugin creates for each environment.

If a GUI action does not work, run the same command in the terminal; the error message is
usually clearer there.

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `'pio' is not recognized` (Windows) or `pio: command not found` (macOS) | The folder that contains `pio` is not on your `PATH` | Redo the `PATH` step in [Install PlatformIO Core](#1-install-platformio-core), then open a **new** terminal |
| `error: externally-managed-environment` when running pip | Homebrew's Python does not allow `pip install` | Use `brew install platformio` or `pipx install platformio` |
| `Access is denied` / `could not open port` (Windows) or `Resource busy` (macOS) | Another program has the serial port open, usually a leftover Arduino IDE Serial Monitor, or a second `pio device monitor` in another terminal | Close the Arduino IDE and any other serial monitor, then try again |
| Upload stuck on `Connecting........_____....` and then fails | The ESP32 did not enter download mode automatically | Run the upload again; when `Connecting...` appears, **hold the BOOT button** until the upload percentage starts, then release it |
| No port visible in Device Manager or `ls /dev/cu.*` | Charge-only USB cable (no data wires), or missing driver | Try a different cable you know transfers data; check the [USB driver](#2-usb-driver) section |
| `UnknownEnvNamesError` / `Unknown environment names 'X'` | The name after `-e` does not match an `[env:...]` in `platformio.ini` (typo, or `-` instead of `_`) | Use one of the names in [Build, upload and monitor](#4-build-upload-and-monitor). The error message lists the valid names |
| First build is very slow | PlatformIO is downloading the ESP32 toolchain | Wait; this only happens once |
| Serial monitor shows unreadable characters | Baud rate does not match the sketch | Open the monitor with `-e <env>` so it uses `monitor_speed` from `platformio.ini` |
