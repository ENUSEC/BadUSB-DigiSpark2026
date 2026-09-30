// fun/banner - Tier 1
//
// Types an ASCII-art banner into Notepad, then leaves it on screen.
//
// Deliberately benign and unmissable: this is the awareness-training payload.
// Run it in a room where people can see the screen and watch them work out on
// their own that a keyboard typed it. The pause at the end is the whole point -
// do not cut it short.
//
// Uses only unshifted-safe ASCII so it renders identically on US and UK
// layouts. See lib/duck/duck.h for why that matters.

#include <duck.h>

static const char BANNER[] PROGMEM =
    "##############################################################\n"
    "#                                                            #\n"
    "#   THIS TEXT WAS TYPED BY A USB KEYBOARD.                  #\n"
    "#                                                            #\n"
    "#   Nothing was infected. Nothing was copied.               #\n"
    "#   A device that looks like a flash drive can              #\n"
    "#   type ANY key, in ANY order, with no permission.         #\n"
    "#                                                            #\n"
    "#   If this surprised you, plug-in devices are worth        #\n"
    "#   a second look. Ask IT.                                  #\n"
    "#                                                            #\n"
    "##############################################################\n";

// Long enough that a passer-by has time to read it. Bump freely.
#define HOLD_MS 20000

void setup() {
  duckBegin();
  ledPulse(2, 150, 150);

  openRun();
  TYPE("notepad");
  press(KEY_ENTER);
  delay(1500);

  typeFlash(BANNER);

  // Sit there. This is the payload's actual deliverable.
  ledOn();
  delayOrAbort(HOLD_MS);
  ledOff();
}

void loop() {}
