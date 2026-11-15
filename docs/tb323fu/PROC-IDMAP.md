# TB323FU native proc id-map lookup companion

Device-specific module for Android 6.12.30 and the already deployed fullfix lkm4ctr.
No kernel image is rebuilt or flashed. Built against the existing matching
common-baseline/out-baseline kernel build and lkm4ctr Module.symvers.

Provides real proc PID dentries for uid_map, gid_map, projid_map and setgroups
by extending missing tgid/tid lookups through native proc_pident_instantiate.
Native permissions, PID dentry revalidation and the vendored namespace map
write handlers remain in use. setgroups write-open checks CAP_SYS_ADMIN in
the target namespace, matching native procfs. All indirect VFS callbacks
retain CFI instrumentation; __nocfi applies to outgoing resolved calls.

The companion is boot-lifetime and intentionally cannot unload: cached proc
inodes retain its file operations. Reboot removes it. No boot autostart added.
Load after lkm4ctr; the updated /data/local/tmp/start-ubuntu26-kde.sh loads it.
The existing installed desktop-helper APK has not been changed.

Verified live on device: stat and exists, Rust cat, relative openat, self,
thread-self, another PID, PID exit invalidation, unshare -Ur, actual UID/GID
map writes in an unmapped child, setgroups=deny. systemd remains running with
zero failed units. Removed the previous GNU cat diversion after verification.

Scope: pathname lookup and file operations. Native proc readdir tables are
unchanged, so directory enumeration alone does not list these extra names.
This is not a claim of complete CONFIG_USER_NS equivalence.

Build the companion against the exact kernel build and lkm4ctr sources used
for the deployed module (including its populated Module.symvers):

```sh
make -C "$KERNEL_SOURCE" O="$KERNEL_OUT" \
 M="$PWD/extra/proc-idmap-compat" ARCH=arm64 LLVM=1 \
 LKM4CTR_ROOT="$LKM4CTR_SOURCE" \
 KBUILD_EXTRA_SYMBOLS="$LKM4CTR_SOURCE/vendor/Module.symvers" modules
```

Use the matching clang and a pahole version supporting the kernel's BTF
features. This device-specific companion is not wired into the multi-KMI
release workflow. Do not deploy it to other kernels based only on the KMI tag.

The live-tested binary SHA-256 is recorded in VERIFIED-MODULE.sha256; the
binary itself is not included. The companion depends on the deployed fullfix
module exposing the vns_* implementations and shadow_hook API. Those earlier
fullfix patches are outside this commit and are not claimed to be supplied here.

Ubuntu service configuration fix: run ubuntu26/fix-services.sh as root inside
the container. It restores the read-only /sys guard for udev and skips TPM
units when no TPM device is exposed. Previous udev overrides are backed up
under /var/backups/droidspaces-compat-20261115. This skips hardware services
that cannot operate in this container; it does not provide TPM hardware.
