// train/explain-attack - Tier 3
//
// The technical debrief, for an IT or security audience: exactly what the
// previous payloads did, including what they did NOT do.
//
// Included because BadUSB demos usually stop at "look, it typed something" and
// leave everyone reasoning about a threat model they were never given. This
// closes that loop, including the control that would have stopped it and the
// reason it did not fire.
//
// The last section is deliberately self-incriminating: every Tier 2 collector
// runs powershell -EncodedCommand, a signature EDR watches for. Being able to
// say "and here is what our own tooling flagged" persuades better than any
// amount of hand-waving.
//
// Deliberately terse line-by-line: the flash budget is 6012 bytes total, and
// long prose here is long prose that will not fit.

#include <duck.h>

// A macro, not a function. The call sites read line("text"), and PSTR() has to
// wrap the literal *at the call site* for the compiler to fold it into flash.
// A function taking `const char *` receives a RAM pointer instead, and then
// typeFlash() reads that RAM address with pgm_read_byte() - garbage on screen,
// and the whole string counted against the 512 bytes of SRAM.
#define line(s) do { typeFlash(PSTR(s)); press(KEY_ENTER); } while (0)

void setup() {
  duckBegin();
  openRun();
  TYPE("notepad");
  press(KEY_ENTER);
  delay(1500);

  line("=== BADUSB: WHAT JUST HAPPENED ===");
  line("");
  line("THE DEVICE");
  line("A Digispark is an ATtiny85 in a case shaped like a USB");
  line("stick. The chip has no USB hardware, so it runs V-USB - a");
  line("software USB stack - over two GPIO pins.");
  line("");
  line("On boot the Micronucleus bootloader presents as a programming");
  line("port for about five seconds. After that timeout with no upload");
  line("arriving, it drops off the bus and hands over to firmware.");
  line("");
  line("That firmware describes itself as a USB HID keyboard. The host");
  line("has no way to ask whether that is true. A keyboard appeared.");
  line("");
  line("THE ATTACK");
  line("Keyboards are allowed to send keystrokes, so this one does. It");
  line("presses Win+R and types a command. No prompt, no consent, no");
  line("visible user action. The host cannot distinguish it from you.");
  line("");
  line("KEYBOARD LAYOUT - THE PART EVERYONE GETS WRONG");
  line("HID sends a usage code per key, not a character. Key to");
  line("character mapping belongs to the OS layout, not the device.");
  line("So the key labelled @ on a US layout only produces @ if the");
  line("OS is US. On UK it gives something else entirely.");
  line("");
  line("DigiKeyboard assumes US, so a raw-typed payload silently");
  line("emits garbage. Hence the base64: A-Z a-z 0-9 + / = are in the");
  line("same physical place on US and UK, so the encoded form is");
  line("layout-independent. Plaintext never reaches the keyboard.");
  line("");
  line("WHAT DID NOT HAPPEN");
  line("No malware. No file read. Nothing sent off-box. No");
  line("persistence. No privilege change. Tier 2 wrote text files to");
  line("C:\\Users\\Public\\ds-recon and stopped.");
  line("");
  line("WHAT WOULD HAVE STOPPED IT");
  line(" 1. USB allow-listing, or a block on vendor 0x16c0. Stock");
  line("    firmware announces its VID, so this works.");
  line(" 2. A hardened Winlogon shell or third-party USB filter.");
  line(" 3. Nothing in Windows alone. Blocking mass storage does");
  line("    NOT block keyboards - separate device classes.");
  line("");
  line("THE PART THAT MAKES EDR FIRE");
  line("Every collector uses powershell -EncodedCommand. That flag");
  line("hides command text from naive inspection, which is exactly");
  line("why it is a detection signature. Robust to keyboard layout,");
  line("visible to script block logging. That is the trade itself.");
  line("");
  line("If your EDR flagged this exercise, it was right to.");

  ledOn();
  delayOrAbort(40000);
  ledOff();
}

void loop() {}
