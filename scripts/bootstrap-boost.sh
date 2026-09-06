#!/bin/sh
# Build a pinned Boost without modifying global package installations.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
deps="$repo_root/build/deps"
prefix="$deps/boost-1.83.0"
archive="$deps/boost_1_83_0.tar.bz2"
mkdir -p "$deps"
if [ ! -f "$archive" ]; then
    curl --fail --location --retry 2 \
      https://archives.boost.io/release/1.83.0/source/boost_1_83_0.tar.bz2 \
      --output "$archive.part"
    mv "$archive.part" "$archive"
fi
# SHA-256 published in the archive's adjacent .json metadata.
checksum=6478edfe2f3305127cffe8caf73ea0176c53769f4bf1585be237eb30798c3b8e
if command -v shasum >/dev/null 2>&1; then
    actual=$(shasum -a 256 "$archive" | cut -d ' ' -f 1)
else
    actual=$(sha256sum "$archive" | cut -d ' ' -f 1)
fi
[ "$actual" = "$checksum" ] || { echo 'Boost checksum mismatch' >&2; exit 1; }
if [ ! -d "$deps/boost_1_83_0" ]; then tar -xjf "$archive" -C "$deps"; fi
cd "$deps/boost_1_83_0"
./bootstrap.sh --with-libraries=thread,system,json --prefix="$prefix"
if [ "$(uname -s)" = Darwin ]; then
    ./b2 cxxstd=11 link=static threading=multi cxxflags=-Wno-enum-constexpr-conversion -j4 install
else
    ./b2 cxxstd=11 link=static threading=multi -j4 install
fi
printf '\nBoost installed in %s\n' "$prefix"
