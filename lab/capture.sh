#!/usr/bin/env bash
# capture.sh - pull a test run off the target for comparison.
#
#   ./lab/capture.sh                    -> ./runs/<timestamp>/
#   ./lab/capture.sh /mnt/shared/runs   -> /mnt/shared/runs/<timestamp>/
#
# The point is diffing two runs. "What did the payload collect?" is a weaker
# question than "what did it miss this time?", and only the second one shows
# up if you keep the artefacts.
#
# Nothing destructive: reads the recon directory, copies it, and adds
# enumeration state. Safe to run mid-exercise.

set -euo pipefail

DEST="${1:-$PWD/runs}"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
OUT="$DEST/$STAMP"

# Where the recon payloads write. Override if you changed it in a payload.
RECON="${RECON:-/mnt/share/ds-recon}"

say() { printf '\033[1;36m==> %s\033[0m\n' "$*"; }

mkdir -p "$OUT"
say "run $STAMP -> $OUT"

# ------------------------------------------------- payload output, if reachable
if [ -d "$RECON" ]; then
  cp -r "$RECON" "$OUT/recon"
  say "collected $(find "$OUT/recon" -type f | wc -l) file(s) from $RECON"
else
  printf '     %s not reachable (mount the target share, or set RECON=)\n' "$RECON" >&2
fi

# ------------------------------------------------------- host-side enumeration
# Best effort, and it degrades quietly. The point is to record what the host
# thought the device was, which is the difference between "the payload ran" and
# "the payload was allowed to run".
if command -v lsusb >/dev/null 2>&1; then
  lsusb > "$OUT/lsusb.txt" 2>&1 || true
fi
if command -v system_profiler >/dev/null 2>&1; then   # macOS
  system_profiler SPUSBDataType > "$OUT/usb.txt" 2>&1 || true
elif [ -d /sys/bus/usb/devices ]; then                 # Linux
  for d in /sys/bus/usb/devices/*/; do
    [ -f "$d/idVendor" ] || continue
    printf '%s %s %s\n' \
      "$(cat "$d/idVendor")" "$(cat "$d/idProduct")" "$(cat "$d/product" 2>/dev/null)"
  done > "$OUT/lsusb.txt" 2>/dev/null || true
fi

# A 16c0:27db line here means the host accepted the HID keyboard. Its absence
# next to a successful recon capture means something else allowed it through.
if [ -f "$OUT/lsusb.txt" ]; then
  if grep -qi '16c0.*27db' "$OUT/lsusb.txt" 2>/dev/null; then
    say "Digispark enumerated as 16c0:27db"
  else
    printf '     Digispark not present in enumeration snapshot (expected if\n' >&2
    printf '     the run already finished and the device was unplugged)\n' >&2
  fi
fi

# ------------------------------------------------------------------- manifest
cat > "$OUT/MANIFEST.txt" <<EOF
run        $STAMP
recon dir  $RECON
files      $(find "$OUT" -type f ! -name MANIFEST.txt | wc -l)
EOF

say "done - compare with:"
printf '    diff -r %s <other-run>\n' "$OUT"
