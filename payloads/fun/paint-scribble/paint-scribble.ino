// fun/paint-scribble - Tier 1
//
// Opens Paint and draws a crude robot with the keyboard.
//
// The point is not the drawing. The point is that it uses only arrow keys and
// Enter - no mouse, no coordinates, no API. If a "harmless-looking" USB device
// can drive a GUI application by brute-force key repetition, then anything you
// can do by hand in front of a screen, it can do while nobody is watching.
//
// Which is also the honest caveat on every payload in this repo: capability
// that equals your own, aimed at the moment you are not looking.

#include <duck.h>

// Press and HOLD a key, so Paint draws a continuous line instead of nudging
// the cursor one pixel at a time.
//
// Note the API difference that matters here: sendKeyStroke() is press-then-
// release back to back, so it cannot hold anything. sendKeyPress() sends only
// the press and leaves the key logically down until you send a 0. Without this
// distinction the scribble below produces a dotted line.
//
// Key repeat rate is a host setting, not ours - at the slowest repeat setting
// these holds draw gaps instead of strokes.
static void hold(uint8_t key, unsigned int ms) {
  DigiKeyboard.sendKeyPress(key);
  delay(ms);
  DigiKeyboard.sendKeyStroke(0);  // zero report = release every key
  delay(60);
}

static void moveTo(uint8_t dir, unsigned int ms) {
  hold(dir, ms);
}

// Fixed route. Tuned by eye on 1080p; will not be pixel-exact elsewhere.
static void drawFigure() {
  // Left side up.
  moveTo(KEY_ARROW_DOWN, 2600);   // head
  moveTo(KEY_ARROW_LEFT, 1500);   // jaw line
  moveTo(KEY_ARROW_UP, 1500);
  moveTo(KEY_ARROW_RIGHT, 2600);  // back up

  moveTo(KEY_ARROW_LEFT, 600);
  moveTo(KEY_ARROW_DOWN, 1900);   // neck
  moveTo(KEY_ARROW_RIGHT, 2200);  // shoulders
  moveTo(KEY_ARROW_LEFT, 2200);
  moveTo(KEY_ARROW_UP, 1900);
  moveTo(KEY_ARROW_RIGHT, 600);

  moveTo(KEY_ARROW_LEFT, 2600);   // legs apart
  moveTo(KEY_ARROW_DOWN, 1500);
  moveTo(KEY_ARROW_RIGHT, 2600);
  moveTo(KEY_ARROW_UP, 1500);
  moveTo(KEY_ARROW_LEFT, 1500);

  // Antenna.
  moveTo(KEY_ARROW_UP, 900);
  moveTo(KEY_ARROW_RIGHT, 500);
  moveTo(KEY_ARROW_LEFT, 500);
  moveTo(KEY_ARROW_DOWN, 900);
}

void setup() {
  duckBegin();
  ledPulse(2, 150, 150);

  // mspaint is the classic; msbrush on newer Windows.
  openRun();
  TYPE("mspaint");
  press(KEY_ENTER);
  delay(3500);

  // Maximise so coordinates roughly match the tuned timings.
  gui(KEY_ARROW_UP);  // maximise
  delay(1500);

  // Pick black from the default palette with Enter (foreground colour is
  // already selected on launch).
  press(KEY_ENTER);
  delay(500);

  ledOn();
  drawFigure();
  delay(500);
  ledOff();

  // Leave the canvas up. Someone should see what happened.
  delayOrAbort(12000);
}

void loop() {}
