// fun/morse - Tier 1
//
// Blinks the onboard LED in Morse while printing a matching transcript into
// Notepad.
//
// Two reasons this exists. It is a decent LED-timing demo (LED state plus
// deliberate spacing is the whole payload). And morse is a genuinely useful
// covert channel to think about: a payload that only blinks a light in a room
// can carry data at very low bandwidth to someone with line of sight, with no
// network, no filesystem and no log entry anywhere.
//
// LED is on PB1, active low.

#include <duck.h>

#define DOT_MS   220
#define DASH_MS  660
#define GAP_MS   220    // between elements
#define LETTER_MS 660   // between letters
#define WORD_MS   1320  // between words

// 'S' and 'O' = ... --- ... , the SOS everyone already recognises.
static const char *MESSAGE = "SOS";

// Element table: '.' or '-', terminated by 0. ' ' separates letters.
static void blinkChar(char c) {
  ledOn();
  delay(c == '.' ? DOT_MS : DASH_MS);
  ledOff();
  delay(GAP_MS);
}

void setup() {
  duckBegin();

  // Transcript alongside, so a demo has a written record too.
  openRun();
  TYPE("notepad");
  press(KEY_ENTER);
  delay(1200);
  TYPE("Morse demo - the light IS the message.\n\n");

  for (const char *p = MESSAGE; *p; p++) {
    if (*p == ' ') {
      delay(WORD_MS - GAP_MS);
      continue;
    }
    blinkChar(*p);
    if (p[1] != '\0' && p[1] != ' ') delay(LETTER_MS - GAP_MS);
  }

  ledOn();
  delayOrAbort(4000);
  ledOff();
}

void loop() {}
