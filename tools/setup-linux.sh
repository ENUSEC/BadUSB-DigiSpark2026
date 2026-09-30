#!/usr/bin/env bash
# setup-linux.sh - build the Digispark toolchain on Debian/Ubuntu/Fedora.
#
# WHY THIS SCRIPT EXISTS INSTEAD OF `arduino-cli board add digistump`
#   The Digistump Boards Manager package index is dead. digistump.com is gone
#   and there is no live mirror, so `arduino-cli core install digistump:avr`
#   cannot resolve. The platform source itself is still alive on GitHub, so we
#   package it locally and serve a hand-written index that points at it.
#
# KNOWN ARDUINO-CLI BUG THIS WORKS AROUND
#   arduino-cli 1.5.x fails any platform that declares a toolsDependency, even
#   when the tool is in the official index. The error is the misleading
#   "platform is not available for your OS". So the index below deliberately
#   declares NO toolsDependencies; avr-gcc is resolved from the already-installed
#   arduino:avr core, and micronucleus is invoked directly by tools/build.sh
#   rather than through arduino-cli's upload target.

set -euo pipefail

CLI_VERSION="${CLI_VERSION:-latest}"
WORK="${WORK:-$HOME/.cache/ds-toolchain}"
PORT="${PORT:-8765}"
HOST_TRIPLE="${HOST_TRIPLE:-x86_64-linux-gnu}"

say()  { printf '\n\033[1;36m==> %s\033[0m\n' "$*"; }
warn() { printf '\033[1;33m!!  %s\033[0m\n' "$*"; }
die()  { printf '\033[1;31mxx  %s\033[0m\n' "$*" >&2; exit 1; }

mkdir -p "$WORK" "$WORK/bin"

# ---------------------------------------------------------------- arduino-cli
say "arduino-cli"
if ! command -v arduino-cli >/dev/null 2>&1 && [ ! -x "$WORK/bin/arduino-cli" ]; then
  case "$(uname -m)" in
    x86_64)  ARCH=64bit ;;
    aarch64) ARCH=arm64 ;;
    i386|i686) ARCH=32bit ;;
    *) die "unsupported arch $(uname -m)" ;;
  esac
  say "downloading arduino-cli for Linux_${ARCH}"
  ( cd "$WORK" && curl -fsSL -o ac.tar.gz \
      "https://downloads.arduino.cc/arduino-cli/arduino-cli_${CLI_VERSION}_Linux_${ARCH}.tar.gz" \
    && tar xzf ac.tar.gz && cp arduino-cli bin/ )
fi
export PATH="$WORK/bin:$PATH"
arduino-cli version

# ------------------------------------------------------------- system tools
say "system packages"
if command -v dnf >/dev/null 2>&1; then
  echo "    sudo dnf install -y libusb-compat-0.1 libusb1-devel libudev-devel make"
elif command -v apt-get >/dev/null 2>&1; then
  echo "    sudo apt-get install -y libusb-0.1-0 libusb-1.0-0-dev libudev-dev make"
else
  warn "unknown package manager - install libusb 0.1 + 1.0 dev packages by hand"
fi
echo "    (libusb 0.1 is the odd one: the bundled micronucleus is from 2015 and"
echo "     links against libusb-0.1.so.4. Modern distros only ship libusb 1.0.)"

# ------------------------------------------------------------- local index
say "packaging Digistump platform from source"
if [ ! -d "$WORK/DigistumpArduino" ]; then
  git clone --depth 1 https://github.com/digistump/DigistumpArduino.git "$WORK/DigistumpArduino"
fi
( cd "$WORK/DigistumpArduino" && tar czf "$WORK/digistump-avr-1.5.4.tar.gz" digistump-avr )

say "packaging micronucleus"
if [ ! -f "$WORK/micronucleus-2.0a4-linux64.tar.gz" ]; then
  cp "$WORK/DigistumpArduino/tools/micronucleus-2.0a4-linux64.tar.gz" "$WORK/" 2>/dev/null || \
  die "micronucleus tarball not found in DigistumpArduino/tools"
fi
tar xzf "$WORK/micronucleus-2.0a4-linux64.tar.gz" -C "$WORK"
cp "$WORK/micronucleus-2.0a4/micronucleus" "$WORK/micronucleus-2.0a4/launcher" "$WORK/bin/" 2>/dev/null || true
# arm/32-bit hosts need the matching tarball from the repo; copy it in if present.
for f in "$WORK/DigistumpArduino/tools"/micronucleus-*-linux32.tar.gz; do
  [ -e "$f" ] && echo "    note: 32-bit host needs micronucleus-2.0a4-linux32"
done

# --------------------------------------------------------- index generation
say "generating local Boards Manager index"
P_SHA=$(sha256sum "$WORK/digistump-avr-1.5.4.tar.gz" | cut -d' ' -f1)
P_SZ=$(stat -c%s "$WORK/digistump-avr-1.5.4.tar.gz")

if [ ! -f "$WORK/bin/micronucleus" ]; then
  warn "micronucleus binary missing; upload step will not work"
  TOOL_BLOCK='[]'
else
  T_SHA=$(sha256sum "$WORK/micronucleus-2.0a4-linux64.tar.gz" | cut -d' ' -f1)
  T_SZ=$(stat -c%s "$WORK/micronucleus-2.0a4-linux64.tar.gz")
  TOOL_BLOCK="[{\"name\":\"micronucleus\",\"version\":\"2.0a4\",\"systems\":[{\"host\":\"$HOST_TRIPLE\",\"url\":\"http://127.0.0.1:$PORT/micronucleus-2.0a4-linux64.tar.gz\",\"archiveFileName\":\"micronucleus-2.0a4-linux64.tar.gz\",\"size\":\"$T_SZ\",\"checksum\":\"SHA-256:$T_SHA\"}]}]"
fi

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
    }],
    "tools": $TOOL_BLOCK
  }]
}
EOF
python3 -c "import json;json.load(open('$WORK/digistump_index.json'))" || die "generated index is not valid JSON"

# ----------------------------------------------------------- local HTTP server
# arduino-cli will not install from file:// URLs, so serve the artifacts over
# loopback HTTP for the duration of the install.
say "serving artifacts on 127.0.0.1:$PORT"
pkill -f "http.server $PORT" 2>/dev/null || true
( cd "$WORK" && setsid nohup python3 -m http.server "$PORT" --bind 127.0.0.1 >"$WORK/http.log" 2>&1 </dev/null & )
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
arduino-cli board listall digistump 2>/dev/null | head -5 || true

cat <<EOF

$(say "done")

Add to your shell profile:
    export PATH="$WORK/bin:\$PATH"

Then build a payload:
    tools/build.sh payloads/00-smoke            # compile only
    tools/build.sh payloads/00-smoke --flash    # compile and flash

Flash budget: 6012 bytes total. V-USB baseline is 2508, duck.h brings it to
3052, leaving roughly 2950 bytes for your payload text. build.sh fails the
build if a sketch exceeds the ceiling.

EOF
