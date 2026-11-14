#!/usr/bin/env bash
# Public Domain since creation (AI-generated; no copyright subsists).
set -euo pipefail
here=$(cd -- "$(dirname -- "$0")" && pwd)
: "${KDIR:?Set KDIR to the prepared matching 6.12.30 kernel output directory}"
archive=${1:?Usage: build.sh source-20261114.tar.gz}
echo "020d8cb5e83c1f2622f421a89d8720289c2eccfbd9a83facece85cd96b202c33  $archive" | sha256sum -c -
work=${WORKDIR:-"$here/build"}
if [[ -e "$work" ]]; then echo "WORKDIR must not exist: $work" >&2; exit 1; fi
mkdir -p "$work"
tar -xzf "$archive" -C "$work" ./lkm4ctr
patch -d "$work/lkm4ctr" -p1 < "$here/lkm4ctr-v1.0.1-tb323fu.patch"
cp -a "$here/pidfd-compat" "$here/capability-compat" "$work/"
make -C "$KDIR" ARCH=arm64 LLVM=1 M="$work/lkm4ctr/vendor" modules
make -C "$KDIR" ARCH=arm64 LLVM=1 M="$work/pidfd-compat" modules
make -C "$KDIR" ARCH=arm64 LLVM=1 M="$work/capability-compat" \
 KBUILD_EXTRA_SYMBOLS="$work/lkm4ctr/vendor/Module.symvers" modules
