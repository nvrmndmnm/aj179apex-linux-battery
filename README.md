# AJAZZ 2.4G 8K Dongle Battery Daemon
A lightweight, high-efficiency C daemon designed to interface with the **Ajazz 2.4G 8K Dongle** (AJ179APEX) on Linux. It retrieves battery telemetry via `libusb` and writes the state to a local raw percentage file.

---

## Features
The official Ajazz driver is Windows-only. This Linux alternative defaults exclusively to the 8K Dongle protocol:

* **Ultra Low Resource Usage**: Uses minimal CPU. It utilizes an event-driven sleep loop and only talks to the mouse when it's actively moving.
* **Raw Output**: Directly outputs the raw battery integer to a file (e.g., `69` instead of bulky JSON blocks) so you can parse it trivially in whichever script or status bar you like.

---

## How to Check Your Mouse IDs

This daemon defaults to the **Ajazz 2.4G 8K Dongle** IDs (`VENDOR_ID = 3151`, `PRODUCT_ID = 5007`).

---

## Installation

### Automatic Installation
The included `install.sh` script automates dependency installation, compilation via `make`, and `systemd` service initialization. 

1.  Grant execution permissions:
    ```bash
    chmod +x install.sh
    ```
2.  Execute with elevated privileges:
    ```bash
    sudo ./install.sh
    ```

---
### Manual Installation

1.  **Install Dependencies:**
Arch:
```Bash
sudo pacman -S libusb gcc make
```
Debian:
```Bash
sudo apt install libusb-1.0-0-dev gcc make
```

2.  **Compile & Deploy:**
    ```bash
    make
    sudo make install
    ```

3. Setup Configuration
```Bash
sudo nano /etc/conf.d/ajazz-battery
```

4. Enable the Daemon (Auto-start)
```Bash
sudo systemctl enable --now ajazz-mouse.service
```

## Why Elevated Privileges (sudo) are Required
* **The Installer (`install.sh`)**: Needs `sudo` because it installs dependencies, copies binaries to global strict-access folders like `/usr/bin` and registers a background daemon with `systemd`.
* **The Daemon (`ajazz_daemon`)**: Communicating directly with USB hardware interfaces (via `libusb`) is locked down by the kernel. The installer automatically applies a `udev` rule giving the daemon permission to ping the dongle without requiring a root override on boot.

---

## Output Integration

The daemon automatically writes the raw battery percentage (e.g. `76`) to `/tmp/ajazz_battery`. 

If the mouse goes to sleep or disconnects, it stops polling, then keeps the last known number until it wakes back up. If it is entirely disconnected internally, it drops a `Disconnected` string to the file.

---
**Original developer:** [Rockeyxx](https://github.com/Rockeyxx)

**License:** MIT
