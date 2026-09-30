// fun/hotkey-tour - Tier 1
//
// Walks through the interesting things a keyboard can do to a Windows desktop,
// one keystroke at a time, pausing on each.
//
// This is the best single payload for explaining BadUSB to a non-technical
// audience, because it removes the abstraction: nobody has to imagine what a
// "keystroke injection payload" is, they just watched one walk around their
// own desktop pressing buttons.
//
// Everything used here exists on a stock US and UK layout and is defined by
// DigiKeyboard - no custom HID usage codes needed.

#include <duck.h>

// Long enough to read the caption between steps.
#define STEP_MS 4200

// `caption` is a flash string - pass a PROGMEM array or a TYPE(...) literal.
static void step(const char *caption, uint8_t mod, uint8_t key) {
  typeFlash(caption);
  press(KEY_ENTER);
  delay(900);
  chord(mod, key);
  ledOn();
  delay(STEP_MS);
  ledOff();
}

void setup() {
  duckBegin();
  openRun();
  TYPE("notepad");
  press(KEY_ENTER);
  delay(1200);

  ledPulse(2, 150, 150);

  // Win+E - File Explorer. The least alarming opener.
  step(PSTR("[1] Win+E  - File Explorer"), MOD_GUI_LEFT, KEY_E);

  // Win+D - show desktop.
  step(PSTR("[2] Win+D  - show desktop"), MOD_GUI_LEFT, KEY_D);

  // Win+R - the Run dialog. Every recon payload in this repo starts here.
  step(PSTR("[3] Win+R  - Run dialog (every payload uses this)"), MOD_GUI_LEFT, KEY_R);
  press(DUCK_KEY_ESC);  // tidy up after ourselves
  delay(400);

  // Win+L - lock the workstation.
  step(PSTR("[4] Win+L  - LOCK THIS SESSION"), MOD_GUI_LEFT, KEY_L);

  // Win+X - the power-user menu.
  step(PSTR("[5] Win+X  - power user menu"), MOD_GUI_LEFT, KEY_X);
  press(DUCK_KEY_ESC);
  delay(400);

  // Ctrl+Alt+Del is the secure attention sequence. Do NOT actually send it -
  // you cannot dismiss it from a second keyboard and the host locks up until
  // someone uses the mouse. Noted here so nobody "just tries it".

  TYPE("[6] Ctrl+Alt+Del is deliberately NOT sent - it locks the");
  press(KEY_ENTER);
  TYPE("    desktop and a Digispark cannot dismiss it.");
  press(KEY_ENTER);
  TYPE("");
  press(KEY_ENTER);
  TYPE("Every step above is one keystroke from a device that looks");
  press(KEY_ENTER);
  TYPE("like a USB stick. That is the entire problem.");

  ledOn();
  delayOrAbort(12000);
  ledOff();
}

void loop() {}
