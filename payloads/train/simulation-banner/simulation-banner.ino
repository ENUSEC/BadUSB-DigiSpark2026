// train/simulation-banner - Tier 3
//
// The close-out payload. Run it immediately after anything else and it states
// plainly that nothing was compromised.
//
// This is the single most important payload in the tier, for a boring reason:
// every awareness exercise is undermined if the audience leaves believing they
// were breached. Be unambiguous. Do not let "it was just a demo" be something
// somebody has to be told by you afterwards.
//
// Keep the numbers in the blank lines below honest for your own environment.
// If you escalate the demo - visible window, Defender alerts, locked
// workstations - say so, so nobody is surprised by the follow-up email.

#include <duck.h>

// A macro, not a function. The call sites read line("text"), and PSTR() has to
// wrap the literal *at the call site* for the compiler to fold it into flash.
// A function taking `const char *` receives a RAM pointer instead, and then
// typeFlash() reads that RAM address with pgm_read_byte() - garbage on screen,
// and the whole string counted against the 512 bytes of SRAM.
#define line(s) do { typeFlash(PSTR(s)); press(KEY_ENTER); } while (0)

void setup() {
  duckBegin();

  // Let the demo payload finish, then take the screen.
  delayOrAbort(1500);

  openRun();
  TYPE("notepad");
  press(KEY_ENTER);
  delay(1500);

  line("############################################################");
  line("#                                                          #");
  line("#        THIS WAS AN AUTHORISED SECURITY EXERCISE.          #");
  line("#                                                          #");
  line("############################################################");
  line("");
  line("Nothing was broken, stolen or changed.");
  line("");
  line("The device you plugged in contained a program that pressed");
  line("keys on this computer. That is all it did. It did not");
  line("install software, read your files, or connect anywhere.");
  line("");
  line("If your screen looked odd, or a window opened by itself,");
  line("that was the demonstration.");
  line("");
  line("WHAT ACTUALLY HAPPENED, IN PLAIN TERMS");
  line("---------------------------------------");
  line("A USB device can pose as a keyboard. The computer cannot tell");
  line("a keyboard from a malicious one by asking. So it lets it");
  line("type. This program then pressed a sequence of keys that opened");
  line("a program and displayed this message.");
  line("");
  line("That is the whole technique. It is simple, it is cheap, and");
  line("it is why 'do not plug in found USB sticks' is real advice.");
  line("");
  line("WHAT TO DO NEXT TIME");
  line("---------------------");
  line("  - Do not plug in a USB device you found or were given by");
  line("    someone you do not know, however urgent the story.");
  line("  - If you have plugged one in already, tell IT. You have");
  line("    nothing to be embarrassed about and it is not a big deal");
  line("    - that is exactly why this exercise exists.");
  line("");
  line("Questions: [name] / [email] / [extension]");

  ledOn();
  delayOrAbort(45000);
  ledOff();
}

void loop() {}
