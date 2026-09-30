// 00-smoke - Tier 0
//
// The one sketch that validates everything else. Run this before trusting any
// payload in this repo.
//
// It checks, in order:
//   1. The device enumerates as a USB HID keyboard at all
//   2. The OS accepts a foreign HID keyboard (no block policy, no setup prompt)
//   3. ASCII types correctly, INCLUDING the characters that differ between
//      US and UK layouts - that is the whole reason duck.h base64-encodes
//      PowerShell rather than typing it raw
//
// A clean run ends with the LED going solid then off. Anything else means stop
// and fix this before moving to Tier 1+.

#include <duck.h>

// Characters that are safe unshifted on both US and UK layouts, plus the
// awkward ones. If the awkward ones come out wrong, duck.h's layout is wrong.
//
// PROGMEM arrays, not `const char *`. A pointer initialised to a literal is a
// pointer in SRAM, and the literal is copied there at startup - which is how
// this sketch used to report 433 bytes of globals on a 512-byte chip. These get
// read back with pgm_read_byte() by typeFlash(), so they must live in flash.
static const char ASCII_PROBE[] PROGMEM = "abcXYZ019 -_=[]\;',./";
static const char US_ONLY[]    PROGMEM = "@#$^&*()~`|{}<>?";

// Also PROGMEM. runPS() reads its argument with pgm_read_byte() too, so a bare
// literal here is both a ~200-byte SRAM hit and a garbled command line - and
// this is the one sketch whose whole job is to prove the encoding works.
static const char PS_PROBE[] PROGMEM =
    "New-Item -Path 'C:\\Users\\Public\\ds-smoke' -ItemType Directory -Force | Out-Null;"
    "'DIGISPARK SMOKE OK @#\\\" ok' | Out-File 'C:\\Users\\Public\\ds-smoke\\result.txt';"
    "'done'";

void setup() {
  if (!duckBegin()) {
    return;  // aborted at plug-in
  }

  ledPulse(3, 120, 120);  // payload starting

  openRun();
  TYPE("cmd");
  press(KEY_ENTER);
  delay(800);

  TYPE("echo ");
  TYPE("L:");
  typeFlash(US_ONLY);
  TYPE(" ");
  typeFlash(ASCII_PROBE);
  press(KEY_ENTER);
  delay(900);

  // A short, deliberately-awkward PowerShell one-liner that contains the
  // characters a raw-typed payload would corrupt. If this prints the path
  // cleanly, the base64 path is sound.
  runPS(PS_PROBE, false, 2500);

  // Verification marker. Easy to eyeball on the target.
  openRun();
  TYPE("notepad C:\\Users\\Public\\ds-smoke\\result.txt");
  press(KEY_ENTER);

  ledPulse(4, 150, 150);
  delayOrAbort(2000);
  ledOff();
}

void loop() {}
