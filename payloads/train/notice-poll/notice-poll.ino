// train/notice-poll - Tier 3
//
// The closing question. Opens a three-question form.
//
// This is the payload that turns an exercise into actual measurement. Every
// awareness deck reports a satisfaction score; almost none report whether the
// technique worked, because asking "did you notice?" usually gets a shrug in
// the room and a rushed answer on the form.
//
// The questions are deliberately the ones whose answers change policy. If most
// people never noticed, the control that matters is the USB allow-list, not
// more training slides. Put this URL somewhere you will actually look at the
// results, and do not let the numbers go stale by only asking once.

#include <duck.h>

// Swap your URL into the middle literal. Adjacent string literals are folded
// together at compile time, so this costs nothing and stays readable.
//
// Start-Process rather than typing the URL into the Run dialog: ':' and '/'
// sit on different keys with different shift states on a UK keyboard, so a
// raw-typed https:// URL comes out mangled. Base64 carries it intact.
static const char SCRIPT[] PROGMEM =
    "Start-Process '"
    "https://forms.example.org/did-you-notice"
    "'";

void setup() {
  duckBegin();

  // Let a preceding demo payload finish before taking over the screen.
  delayOrAbort(1500);

  runPS(SCRIPT, false, 1000);

  // Browsers take a while to start. Sit on the LED so you can tell the
  // payload completed rather than the device dying mid-run.
  ledOn();
  delayOrAbort(15000);
  ledOff();
}

void loop() {}
