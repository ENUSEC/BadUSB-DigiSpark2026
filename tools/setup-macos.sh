#!/usr/bin/env bash
# setup-macos.sh - build the Digispark toolchain on macOS (Intel and Apple Silicon).
#
# Differences from Linux, and why:
#
#  1. There is no libusb 0.1 in modern Homebrew, so the 2015 prebuilt
#     micronucleus will not link. We build micronucleus from source instead,
#     which is what its Makefile expects on Darwin. If you already have
#     libusb-compat installed the prebuilt binary works and you can skip it:
#         brew install libusb-compat
#
#  2. Homebrew does not install libusb's pkg-config file in a predictable
#     location, so micronucleus' own Makefile hunts for it via `brew list`.
#     Installing libusb via brew first is what makes that work.
#
#  3. arduino-cli's install.sh is the same on macOS, but it wants an explicit
#     BINDIR.
#
# Everything else - the dead Digistump package index, the local index we serve
# over loopback, and the arduino-cli 1.5 toolsDependencies bug - is the same as
# on Linux. See QUICKSTART.md for the full explanation.

set -euo pipefail

WORK="${WORK:-$HOME/.cache/ds-toolchain}"
PORT="${PORT:-8765}"

say()  { printf '\n\033[1;36m==> %s\033[0m\n' "$*"; }
warn() { printf '\033[1;33m!!  %s\033[0m\n' "$*"; }
die()  { printf '\033[1;31mxx  %s\033[0m\n' "$*" >&2; exit 1; }

command -v brew >/dev/null 2>&1 || die "Homebrew required: https://brew.sh"
[ "$(uname -s)" = "Darwin" ] || die "this script is for macOS - use tools/setup-linux.sh"

mkdir -p "$WORK" "$WORK/bin"

# ---------------------------------------------------------------- arduino-cli
say "arduino-cli"
if [ ! -x "$WORK/bin/arduino-cli" ]; then
  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh \
    | BINDIR="$WORK/bin" sh
fi
export PATH="$WORK/bin:$PATH"
arduino-cli version

# ------------------------------------------------------------- system tools
say "host dependencies"
echo "    brew install libusb pkg-config"
echo "    (skip if already installed)"
brew list libusb >/dev/null 2>&1 || brew install libusb pkg-config

# ------------------------------------------------------------- micronucleus
say "building micronucleus from source (no libusb 0.1 on macOS)"
if [ ! -x "$WORK/bin/micronucleus" ]; then
  [ -d "$WORK/micronucleus" ] || git clone --depth 1 \
    https://github.com/micronucleus/micronucleus.git "$WORK/micronucleus"
  ( cd "$WORK/micronucleus/commandline" && make )
  cp "$WORK/micronucleus/commandline/micronucleus" "$WORK/bin/"
fi
"$WORK/bin/micronucleus" 2>&1 | head -2 || true

# ------------------------------------------------------------- local index
say "packaging Digistump platform from source"
[ -d "$WORK/DigistumpArduino" ] || git clone --depth 1 \
  https://github.com/digistump/DigistumpArduino.git "$WORK/DigistumpArduino"
( cd "$WORK/DigistumpArduino" && tar czf "$WORK/digistump-avr-1.5.4.tar.gz" digistump-avr )

say "generating local Boards Manager index"
case "$(uname -m)" in
  arm64) TRIPLE="aarch64-apple-darwin" ;;
  *)     TRIPLE="x86_64-apple-darwin" ;;
esac

P_SHA=$(shasum -a 256 "$WORK/digistump-avr-1.5.4.tar.gz" | cut -d' ' -f1)
P_SZ=$(stat -f%z "$WORK/digistump-avr-1.5.4.tar.gz")

cat > "$WORK/digistump_index.json" <<EOF
{
  "packages": [{
    "name": "digistump",
    "maintainer": "local",
    "websiteURL": "https://github.com/digistump/DigistumpArduino",
    "email": "noreply@example.invalid",
    "help": { "online": "https://github.com/digistump/DigistumpArduino" },
    "platforms": [{
      "name": "Digispark AVR Boards",
      "id": "digistump-avr",
      "version": "1.5.4",
      "architecture": "avr",
      "category": "Contributed",
      "help": { "online": "https://github.com/digistump/DigistumpArduino" },
      "url": "http://127.0.0.1:$PORT/digistump-avr-1.5.4.tar.gz",
      "archiveFileName": "digistump-avr-1.5.4.tar.gz",
      "size": "$P_SZ",
      "checksum": "SHA-256:$P_SHA",
      "boards": [{ "name": "Digispark (Default - 16.5mhz)" }]
    }]
  }]
}
EOF
python3 -c "import json;json.load(open('$WORK/digistump_index.json'))" \
  || die "generated index is not valid JSON"

# ----------------------------------------------------------- local HTTP server
# arduino-cli will not install from file:// URLs.
say "serving artifacts on 127.0.0.1:$PORT"
pkill -f "http.server $PORT" 2>/dev/null || true
( cd "$WORK" && nohup python3 -m http.server "$PORT" --bind 127.0.0.1 >"$WORK/http.log" 2>&1 </dev/null & )
sleep 2
curl -fsS -o /dev/null "http://127.0.0.1:$PORT/digistump_index.json" || die "local index not reachable"

# ------------------------------------------------------------------ install
say "arduino:avr core (provides avr-gcc)"
arduino-cli config init --overwrite >/dev/null 2>&1 || true
arduino-cli config set directories.data "$WORK/data" >/dev/null
arduino-cli config set directories.downloads "$WORK/dl" >/dev/null
arduino-cli config set board_manager.additional_urls "http://127.0.0.1:$PORT/digistump_index.json" >/dev/null
arduino-cli core update-index >/dev/null
arduino-cli core install arduino:avr 2>&1 | tail -2

say "digistump platform"
arduino-cli core install digistump:avr 2>&1 | tail -2

say "verifying"
arduino-cli core list

cat <<EOF

$(say "done")

Add to your shell profile:
    export PATH="$WORK/bin:\$PATH"

Then:
    tools/build.sh payloads/tier0/00-smoke           # compile
    tools/build.sh payloads/tier0/00-smoke --flash   # flash

EOF
