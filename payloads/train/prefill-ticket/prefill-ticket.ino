// train/prefill-ticket - Tier 3
//
// Pre-fills an IT helpdesk ticket with the details already filled in, so the
// only thing left for the user to do is read it and press send.
//
// The reasoning is behavioural, not just convenience. An awareness campaign
// that makes reporting hard will not get reports. A blank form is a barrier; a
// pre-filled form with one click left is a nudge, and it also guarantees the
// report contains the fields IT actually triages on.
//
// Adjust the SERVICE_NAME and MAILTO constants to match your own helpdesk -
// that is the only bit that needs changing.

#include <duck.h>

// Will not fit at flash budget as a PSTR literal on every setup; keep the
// address short or point it at an internal short-link.
static const char SERVICE[] PROGMEM = "IT Helpdesk";

// A macro, not a function. The call sites read line("text"), and PSTR() has to
// wrap the literal *at the call site* for the compiler to fold it into flash.
// A function taking `const char *` receives a RAM pointer instead, and then
// typeFlash() reads that RAM address with pgm_read_byte() - garbage on screen,
// and the whole string counted against the 512 bytes of SRAM.
#define line(s) do { typeFlash(PSTR(s)); press(KEY_ENTER); } while (0)

void setup() {
  duckBegin();
  delayOrAbort(1500);  // let a preceding demo payload land first

  openRun();
  TYPE("notepad");
  press(KEY_ENTER);
  delay(1500);

  line("=== DRAFT TICKET - DO NOT SEND THIS, IT IS A TEMPLATE ===");
  line("");
  line("  To:       [helpdesk address]");
  line("  Subject:  Found USB device - possible BadUSB");
  line("");
  line("  Body:");
  line("");
  line("  I plugged in a USB device I did not recognise.");
  line("");
  line("  Device described as: digistump.com / DigiKey");
  line("  USB IDs:             VID 0x16c0  PID 0x27db");
  line("  Serial number:       none");
  line("  Where I found it:    [location / who gave it to me]");
  line("  When:                [date and roughly what I was doing]");
  line("");
  line("  I have NOT plugged it in anywhere else.");
  line("");
  line("  What I saw happen:");
  line("  [describe it - 'a window opened and typed by itself' is a");
  line("   perfectly good answer, you do not need to know more]");
  line("");
  line("---");
  line("");
  line("WHAT THIS IS FOR");
  line("");
  line("This device was a keyboard. That is all a USB device can be");
  line("from the computer's point of view, so the computer let it type");
  line("into anything you had open - including a logged-in account.");
  line("");
  line("This exact template is what you should use for any found");
  line("device. You are not admitting to a mistake by reporting it.");
  line("You are the control that stops this reaching someone else.");
  line("");
  line("You can also check any PC yourself, any time, in PowerShell:");
  line("");
  line("  Get-PnpDevice -Class Keyboard |");
  line("    Select FriendlyName, InstanceId");
  line("");
  line("Anything showing 0x16c0 is not a normal keyboard.");

  ledOn();
  delayOrAbort(40000);
  ledOff();
}

void loop() {}
