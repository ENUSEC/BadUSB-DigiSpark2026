// detect/touch-latch - Tier 4
//
// Logs whether anyone physically touched the device, via a capacitive pad on a
// spare pin.
//
// WHY THIS EXISTS
// The hardest question in a physical engagement is "was the device even
// noticed?" Every other Tier 4 payload measures the bus. This measures the
// person. A touch event is evidence that someone handled the device - which is
// the difference between a payload that ran silently into a locked screen and
// one that a human actually saw and failed to report.
//
// Used alongside recon/clipboard or recon/window-titles it gives you a
// timeline: device handled at T, analyst was mid-something at T. That pairing
// is often the whole engagement.
//
// WHY SIMPLE GPIO, NOT CAPACITIVE SENSING
// A proper capacitive sensor needs charge-transfer timing and a fairly tight
// loop, which competes with the USB stack for the same 6KB and 512 bytes.
// Instead this watches a plain GPIO for the transition that a human touch
// through a solder point or a scrap of foil will produce. It is less sensitive
// and more false-positive-prone than a real sensor, and that is an honest
// trade for something that fits.
//
// If you have a TTP223 or similar module, feed its OUT pin into the same GPIO
// and it becomes a proper touch sensor with no code change.
//
// WIRING
//
//   D2 (PB2) ----+---- 3M3 pull-down to GND
//               |
//              touch pad / foil, insulated with tape or Kapton
//
// Optionally: TTP223 OUT -> D2, module powered from the USB rail.
//
// The internal pull-up in buttonSetup() fights the 3M3 pull-down. If you wire an
// external one, define DUCK_BTN_PIN to a free pin and do NOT call buttonSetup -
// see the commented alternative below.

#include <duck.h>

#define TOUCH_PIN 3   // PB3. Free on the Digispark; PB2 is the on-board button.

// Debounce. A real finger on a foil pad bounces for tens of milliseconds.
#define DEBOUNCE_MS 60

static bool lastState = false;

static bool readTouch() {
  // Falling edge = touched, with the external pull-down holding the idle
  // state at 0. Reverse the comparison if your wiring inverts.
  return digitalRead(TOUCH_PIN) == LOW;
}

static void blink(uint8_t n, uint16_t onMs, uint16_t gapMs) {
  for (uint8_t i = 0; i < n; i++) {
    ledOn();  delay(onMs);
    ledOff(); delay(gapMs);
  }
}

void setup() {
  pinMode(TOUCH_PIN, INPUT);
  ledOff();

  // Baseline sample. A pad that reads "touched" at rest is miswired or
  // floating, and reporting touches from here would be pure noise.
  delay(500);
  if (readTouch()) {
    // 6 slow blinks = no usable baseline. Stop here rather than log garbage.
    blink(6, 600, 400);
    return;
  }

  blink(2, 200, 300);  // armed

  for (;;) {
    bool now = readTouch();

    if (now != lastState) {
      // Settle before believing it.
      delay(DEBOUNCE_MS);
      if (readTouch() == now) {
        if (now) {
          // Touched. Blink count so far is the log - there is no storage.
          // Count the elapsed time in coarse blocks instead of pretending
          // to have per-event timestamps.
          blink(1, 120, 120);
        } else {
          blink(2, 80, 80);  // released
        }
      }
    }
    lastState = now;

    delay(10);
  }
}

void loop() {}
