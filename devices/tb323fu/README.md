# Lenovo TB323FU: stock Android 16 / 6.12.30

Device-tested support for `6.12.30-android16-5-g1750f757fabe-ab13938768-4k`.
The device kernel was not rebuilt or flashed. These are device-specific builds;
matching Android and kernel major versions alone do not establish compatibility.
No automatic boot loading is added by this directory.

## Changes

`lkm4ctr-v1.0.1-tb323fu.patch` applies to the **release source archive**, which
omits the four patches shipped alongside it. It includes those four upstream
patches, enables the proc inode accessor against the matching 6.12 kernel
headers (fixing cached `/proc/1/ns/pid`), and resolves eight unexported functions
through the existing resolver. Outgoing resolved calls use `__nocfi`; missing
functions abort initialization.

`pidfd-compat/` changes only EOPNOTSUPP results for PID/user/IPC namespace
pidfd ioctls to ENOTTY. This lets systemd fall back to the working procfs
namespace interface. Access-denied errors are not changed.

`capability-compat/` handles PR_CAPBSET_DROP in child user namespaces using
`cred->user_ns` rather than the native CONFIG_USER_NS=n constant. It requires
namespace-local CAP_SETPCAP, validates the capability, copies credentials and
commits the reduced bounding set. Initial-namespace calls and other prctl
operations retain their native implementation; the outer LSM chain remains.
It depends on the matching lkm4ctr module's exported hook engine.

## Build and source provenance

Download [v1.0.1 source-20261114.tar.gz](https://github.com/misaka5202/droidspaces-lkm/releases/download/v1.0.1/source-20261114.tar.gz).
The build script verifies the pinned SHA-256, extracts a fresh tree and applies
the cumulative patch. Do not apply patches/0001-0004 again.

Prepare matching Android common 6.12.30 source at revision `1750f757fabe`,
with the included `kernel.config`, generated headers and Module.symvers.
The tested prepared output was the local stock-feature baseline with
Clang r536225 (19.0.1), Rust 1.82.0 and a compatible pahole. Set PATH,
LD_LIBRARY_PATH and PAHOLE for your environment as needed. Building the
prepared kernel on a workstation does not replace the device kernel.

```sh
KDIR=/absolute/path/to/prepared/kernel-output bash build.sh source-20261114.tar.gz
```

The original source archive includes its upstream copying.txt and license
notices. Preserve them when redistributing the patched source. Vendored Linux
code remains GPL-2.0; new companion sources carry their provenance notices and
use MODULE_LICENSE("GPL") for kernel export compatibility.

## Manual load

Verify `prebuilt/SHA256SUMS` before copying the three modules to the device.
Load from a root shell, in this order, after a clean boot:

```sh
insmod lkm4ctr-tb323fu-fullfix.ko
insmod ds_pidfd_compat.ko
insmod ds_userns_cap_compat.ko
```

Do not load an additional lkm4ctr instance. Reboot to replace an older loaded
build; do not force-unload modules referenced by namespaces. Keep a recovery
path available when testing kernel modules. This does not enable the optional
CPU/pids emulators, macvlan, IPv6 NAT, devtmpfs or vkms.

## Validation

- QEMU: PID/user namespaces, SysV IPC, POSIX mqueue and seccomp probes pass.
- Device: stock Droidspaces 7.2.0 start/show/run/stop lifecycle passes.
- Ubuntu 26.04.1 ARM64 KDE: systemd, NAT/DHCP, DNS and HTTPS work; KWin and
  Plasma desktop visually verified through Anland 5.23 with SELinux enforcing.
- UPower starts with its original PrivateUsers and capability restrictions;
  battery and USB power supplies are enumerated.
- `test-capabilities.py` passes both outside and inside `unshare -Ur`:
  valid bounding-bit drop, EINVAL for invalid capability, EPERM after removing
  effective CAP_SETPCAP. It changes only its own process credentials.

These checks do not establish complete namespace compatibility or long-term
stability. No claim of universal GKI support is made. Device results are in
the adjacent text logs. Run the capability test inside a disposable container:

```sh
python3 test-capabilities.py
unshare -Ur python3 test-capabilities.py
```
