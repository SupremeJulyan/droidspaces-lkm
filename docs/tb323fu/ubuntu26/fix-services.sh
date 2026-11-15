#!/bin/bash
set -eu
backup=/var/backups/droidspaces-compat-20261115
mkdir -p "$backup"
for unit in systemd-udevd.service systemd-udevd-control.socket systemd-udevd-kernel.socket; do
 dir=/etc/systemd/system/$unit.d
 mkdir -p "$dir"
 if [ -f "$dir/99-readonly-fix.conf" ]; then cp -a "$dir/99-readonly-fix.conf" "$backup/$unit-readonly.conf"; fi
 printf '[Unit]\nConditionPathIsReadWrite=\nConditionPathIsReadWrite=/sys\n' > "$dir/99-readonly-fix.conf"
done
for unit in tpm-udev.path tpm-udev.service; do
 mkdir -p /etc/systemd/system/$unit.d
 printf '[Unit]\nConditionPathExistsGlob=/dev/tpm[0-9]*\n' > /etc/systemd/system/$unit.d/99-droidspaces-hardware.conf
done
systemctl stop systemd-udevd-control.socket systemd-udevd-kernel.socket tpm-udev.path
systemctl daemon-reload
systemctl reset-failed systemd-udevd.service systemd-udevd-control.socket systemd-udevd-kernel.socket tpm-udev.path tpm-udev.service
systemctl start systemd-udevd-control.socket systemd-udevd-kernel.socket tpm-udev.path
systemctl --failed --no-pager
/usr/bin/gnucat /proc/self/uid_map
