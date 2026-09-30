// fun/flip-screen - Tier 1
//
// Rotates the display 180 degrees (Ctrl+Alt+Down). Reversible, zero
// persistence, and the single most visible "wait, what" reaction of any
// payload in this repo - which makes it a good confidence check.
//
// Windows-only key combo. On macOS the equivalent is
// Option+Command+Left/Right for the mirrored variants, but Flip needs a driver
// so it is Windows in practice.

#include <duck.h>

#define FLIP_HOLD_MS 12000

void setup() {
  duckBegin();
  ledPulse(2, 150, 150);

  // Ctrl+Alt+Down = flip 180 degrees.
  chord(MOD_CONTROL_LEFT | MOD_ALT_LEFT, KEY_ARROW_DOWN);

  ledOn();
  delayOrAbort(FLIP_HOLD_MS);

  // Put it back the way we found it. Always leave the scene clean.
  chord(MOD_CONTROL_LEFT | MOD_ALT_LEFT, KEY_ARROW_DOWN);
  delay(800);
  ledOff();
}

void loop() {}
