# Ansible provisioning for the DoggoLights testing jig Raspberry Pi.

## One-time controller setup (your laptop)

```bash
pip install ansible
```

Edit [inventory.ini](inventory.ini) - set the Pi's hostname/IP and login user.

## Provision a fresh Pi

```bash
ssh-copy-id pi@doggojig-pi.local           # passwordless sudo target
ansible-playbook -i inventory.ini site.yml --ask-become-pass
```

This is idempotent. After it finishes you can open
`http://doggojig-pi.local:8080` and the service will already be running.

## What it does

1. Installs OS packages: `git`, `python3-venv`, `network-manager`, `i2c-tools`.
2. Enables the I2C interface (via `raspi-config nonint do_i2c 0`).
3. Creates a `doggojig` system user in `dialout`, `gpio`, `i2c`.
4. Clones the repo to `/opt/doggojig/src`, creates a venv at
   `/opt/doggojig/venv`, installs the package with `[pi]` extras (pulls
   `pyudev` + `RPi.GPIO`).
5. Drops a default `/etc/doggojig/config.yaml` (only if missing - re-runs
   never clobber your tweaks).
6. Installs the udev rule (stable `/dev/doggolights-dut` symlink) and the
   systemd unit, then enables + starts `doggojig.service`.

## Updating

Bumping the repo or config:

```bash
ansible-playbook -i inventory.ini site.yml --ask-become-pass --tags update
# or just:
ansible-playbook -i inventory.ini site.yml --ask-become-pass
```

The `git` + `pip install` steps detect changes and the handler restarts the
service automatically.

## SD card failure recovery

1. Flash a fresh Raspberry Pi OS Lite image, enable SSH, set hostname.
2. `ssh-copy-id` to it.
3. Re-run the playbook.

Total recovery time once the Pi boots: a few minutes plus apt download time.
