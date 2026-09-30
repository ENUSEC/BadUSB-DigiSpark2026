#!/usr/bin/env bash
# build.sh - compile (and optionally flash) a payload.
#
#   tools/build.sh payloads/00-smoke          compile only, print size report
#   tools/build.sh payloads/00-smoke --flash  compile, then flash via micronucleus
#   tools/build.sh all                       every payload in payloads/
#   tools/build.sh --flash all               compile all, flash only the last
#
# Enforces the 6012-byte ceiling: a sketch that does not fit is a build error,
# not a surprise at flash time.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORK="${WORK:-$HOME/.cache/ds-toolchain}"
export PATH="$WORK/bin:$PATH"

FQBN="digistump:avr:digispark-tiny"
MAX_BYTES=6012
BUILD_OUT="$ROOT/.build"
LIB="$ROOT/lib/duck"

say()  { printf '\n\033[1;36m==> %s\033[0m\n' "$*"; }
ok()   { printf '\033[1;32m ok\033[0m  %s\n' "$*"; }
fail() { printf '\033[1;31mFAIL\033[0m  %s\n' "$*"; exit 1; }

command -v arduino-cli >/dev/null 2>&1 || fail "arduino-cli not on PATH. Run tools/setup-$(uname -s | tr 'A-Z' 'a-z').sh first."

DO_FLASH=0
TARGETS=()
for arg in "$@"; do
  case "$arg" in
    --flash) DO_FLASH=1 ;;
    # Not `mapfile`: that is bash 4+, and macOS still ships bash 3.2 as /bin/bash.
    all)     while IFS= read -r d; do TARGETS+=("$d"); done < <(find "$ROOT/payloads" -mindepth 2 -maxdepth 2 -type d | sort) ;;
    *)       TARGETS+=("$arg") ;;
  esac
done
[ ${#TARGETS[@]} -eq 0 ] && fail "usage: build.sh [--flash] <sketch-dir>|all"

mkdir -p "$BUILD_OUT"

# ------------------------------------------------------------------ lint
# A plain `const char *x = "text";` is a RAM pointer on AVR, and the literal is
# copied into SRAM at startup - on a 512-byte chip that is usually fatal. The
# subtler version passes compilation cleanly and then types garbage, because
# typeFlash() reads its argument with pgm_read_byte(). So: flag every
# typeFlash() call whose argument is a bare string literal, which is never
# correct. PROGMEM arrays and PSTR()/TYPE() are the supported forms.
lint_flash() {
  local src="$1" hits
  hits=$(grep -nE '(typeFlash|typeEncoded|runPS|recon)\(\s*"' "$src" || true)
  if [ -n "$hits" ]; then
    printf '\033[1;33mlint\033[0m  %s\n' "$hits" | sed 's/^/      /'
    fail "$(basename "$src"): flash-reading function called with a bare literal.
       typeFlash/typeEncoded/runPS/recon all read with pgm_read_byte(), but a
       bare literal lives in SRAM - it will type garbage, and it counts against
       the 512-byte SRAM budget. Use TYPE(\"...\") for a literal, or
       'static const char S[] PROGMEM = \"...\"' for anything longer."
  fi
}

for t in "${TARGETS[@]}"; do
  SKETCH="${t%/}"
  NAME="$(basename "$SKETCH")"
  if [ -f "$SKETCH/$NAME.ino" ]; then lint_flash "$SKETCH/$NAME.ino"; fi
done

for t in "${TARGETS[@]}"; do
  SKETCH="${t%/}"
  NAME="$(basename "$SKETCH")"
  # arduino-cli requires the main .ino to be named after its folder.
  if [ ! -f "$SKETCH/$NAME.ino" ]; then
    printf '\033[1;33m--  %s\033[0m  skipped (expected %s/%s.ino)\n' "$NAME" "$NAME" "$NAME"
    continue
  fi
  say "$NAME"

  if ! LOG=$(arduino-cli compile \
        -b "$FQBN" \
        --library "$LIB" \
        --build-path "$BUILD_OUT/$NAME" \
        --output-dir "$BUILD_OUT/$NAME" \
        "$SKETCH" 2>&1); then
    # arduino-cli reports an oversized sketch only as "text section exceeds
    # available space" - no byte count, no sketch name. Worth translating,
    # because the fix (shorten it) is not obvious from that message.
    if printf '%s' "$LOG" | grep -q 'text section exceeds available space'; then
      printf '%s\n' "$LOG" | tail -5
      fail "$NAME does not fit in $MAX_BYTES bytes.
       arduino-cli does not report the size on overflow. Shorten the payload
       (see README.md 'Flash budget') - the common cause is too many recon
       commands in one sketch, or a training sketch with too much prose."
    fi
    printf '%s\n' "$LOG" | tail -25
    fail "$NAME did not compile"
  fi

  USED=$(printf '%s' "$LOG" | grep -oE 'Sketch uses [0-9]+ bytes' | grep -oE '[0-9]+' || echo 0)
  RAM=$(printf '%s' "$LOG" | grep -oE 'Global variables use [0-9]+ bytes' | grep -oE '[0-9]+' || echo 0)

  if [ "${USED:-0}" -gt "$MAX_BYTES" ]; then
    printf '%s\n' "$LOG" | tail -5
    fail "$NAME is $USED bytes, over the $MAX_BYTES ceiling by $((USED - MAX_BYTES))"
  fi

  PCT=$(( USED * 100 / MAX_BYTES ))
  printf '     flash  %5d / %d bytes  (%d%%)   headroom %d\n' "$USED" "$MAX_BYTES" "$PCT" "$((MAX_BYTES - USED))"
  printf '     SRAM   %5d / 512 bytes\n' "$RAM"
  ok "$NAME compiled -> $BUILD_OUT/$NAME/$NAME.hex"

  if [ "$DO_FLASH" = "1" ]; then
    command -v micronucleus >/dev/null 2>&1 || fail "micronucleus not found. Install libusb-compat-0.1 (see QUICKSTART.md)."
    say "flash $NAME - plug the Digispark in NOW, it only listens ~5s"
    # -cdigispark retries across re-enumeration; --run leaves the sketch running.
    micronucleus --run -cdigispark --timeout 60 "$BUILD_OUT/$NAME/$NAME.hex"
    ok "$NAME flashed"
  fi
done

say "done"
