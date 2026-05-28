# DoggoLights Testing Jig

Python + Flask + Vue based automated test jig that runs on a Raspberry Pi 3B+
with a custom HAT. Tests a freshly-soldered DoggoLights PCB end-to-end:

1. Detect DUT on USB (`/dev/ttyACM*`).
2. Pull boot/rst, flash latest `firmware.bin` via `esptool`.
3. Open the USB-CDC RPC console exposed by the firmware (`rpc.c`) and run:
   - LED test (lux sensors + INA228 current sense per colour/preset).
   - WiFi test (DUT AP `DoggoLights<MAC[3..5]>`, RPi associates via `nmcli`).
   - Button test (`WAIT_BUTTON`).
4. On PASS: send `MFG_PASS`, record results in SQLite, mark device locked.

## Layout

```
TestingJig/
  pyproject.toml               # installable package: `pip install .`
  run.py                       # dev shim -> jig.cli:main
  config.yaml                  # optional local override (cwd)
  scripts/                     # dev shims for flash / fetch
  jig/                         # the package
    cli.py                     # console-script entry points
    runtime.py                 # shared server/flash/fetch impl
    settings.py                # config loader (env / etc / home / pkg default)
    default_config.yaml        # shipped inside the wheel
    state.py / database.py / log.py
    test_runner.py
    app.py                     # Flask app
    firmware/                  # updater + esptool wrapper
    hardware/                  # I2C bus, INA228, lux, GPIO, USB monitor
    dut/                       # RPC client + MAC helpers
    wifi/                      # nmcli helpers
    tests/                     # one file per step
    web/                       # Vue 3 (CDN) single-page UI
  deploy/
    doggojig.service           # systemd unit
    99-doggojig.rules          # udev rule (/dev/doggolights-dut symlink)
  ansible/
    site.yml                   # one-shot provisioning playbook
    inventory.ini
    README.md
```

## Install (on the Pi, automated)

Recommended. See [ansible/README.md](ansible/README.md):

```bash
cd Hardware/TestingJig/ansible
ansible-playbook -i inventory.ini site.yml --ask-become-pass
```

After a few minutes the Pi has the package installed in `/opt/doggojig/venv`,
config at `/etc/doggojig/config.yaml`, results at `/var/lib/doggojig/`, and
`doggojig.service` running on boot. Open `http://<pi>:8080`.

## Install (manual)

```bash
git clone https://github.com/deskpro256/DoggoLights.git
cd DoggoLights/Hardware/TestingJig
python3 -m venv .venv && source .venv/bin/activate
pip install .[pi]              # drop [pi] on a dev machine without RPi.GPIO/pyudev
doggojig                       # serves on http://0.0.0.0:8080
```

## Standalone tools

Both work whether installed or run from a checkout:

```bash
doggojig-fetch                                  # refresh cached firmware.bin
doggojig-flash --port /dev/doggolights-dut      # flash cached firmware
doggojig-flash --port /dev/doggolights-dut --erase

# or from a checkout, without installing:
python scripts/fetch_firmware.py
python scripts/flash.py --port /dev/ttyACM0
```

## Config precedence

`settings.load()` searches, first match wins:

1. `$DOGGOJIG_CONFIG`
2. `./config.yaml` (cwd)
3. `~/.config/doggojig/config.yaml`
4. `/etc/doggojig/config.yaml`
5. Packaged default ([jig/default_config.yaml](jig/default_config.yaml))

Your file is shallow-merged on top of the default, so partial overrides are
fine.
