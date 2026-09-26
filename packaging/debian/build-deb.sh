#!/bin/sh
# Builds a Debian package (.deb) of mac68k-asm with dpkg-deb - no debhelper needed.
#   packaging/debian/build-deb.sh [version]      -> mac68k-asm_<version>_<arch>.deb
set -eu
HERE=$(cd "$(dirname "$0")" && pwd); ROOT=$(cd "$HERE/../.." && pwd)
VERSION=${1:-$(sed -n 's/^#define MAC68K_VERSION "\([0-9.]*\)".*/\1/p' "$ROOT/src/main.c")}
ARCH=$(dpkg --print-architecture)
PKG=$(mktemp -d)
trap 'rm -rf "$PKG"' EXIT
make -C "$ROOT" clean all
make -C "$ROOT" install PREFIX=/usr DESTDIR="$PKG"
mkdir -p "$PKG/usr/share/doc/mac68k-asm" "$PKG/DEBIAN"
cp "$ROOT/README.md" "$PKG/usr/share/doc/mac68k-asm/"
cat > "$PKG/DEBIAN/control" <<CTL
Package: mac68k-asm
Version: $VERSION
Section: devel
Priority: optional
Architecture: $ARCH
Maintainer: Tobias Bircher <mac68k@bircher.ai>
Depends: libc6
Recommends: mac68k-disk
Homepage: https://github.com/bircher988/mac68k-asm
Description: 68k assembler toolchain for the classic Macintosh
 Assembler, linker and resource compiler that build applications for the
 first Macintosh models (64K ROM, System 3.x) from 68000 assembler sources.
 Reads MDS-style projects (.Asm, .Link, .R, .Job) and writes MacBinary files.
CTL
dpkg-deb --build --root-owner-group "$PKG" "$ROOT/mac68k-asm_${VERSION}_${ARCH}.deb"
echo "built $ROOT/mac68k-asm_${VERSION}_${ARCH}.deb"
