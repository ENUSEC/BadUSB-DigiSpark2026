// train/print-iocs - Tier 3
//
// Displays the USB identity of the device currently plugged in, so the person
// looking at it knows what to report to IT.
//
// WHY THIS DEVICE IS ACTUALLY EASY TO SPOT, and why that is the lesson:
//
//   VID 0x16c0  PID 0x27db
//   Manufacturer  digistump.com
//   Product       DigiKey
//   Serial        (none)
//
// 0x16c0 is the V-USB project's shared vendor ID. It is assigned to hobbyist
// projects and is NOT any real keyboard manufacturer. In a corporate estate it
// should be trivially blockable, and a device presenting it is not a Logitech
// keyboard no matter what else it claims to be.
//
// Note the irony worth raising in the session: this stock firmware is MORE
// honest than a commercial attack device would be. Real ones borrow the VID and
// PID of a Logitech K120 or an Apple keyboard precisely so this screen shows
// nothing useful. The detection rule that catches this device - "vendor 0x16c0
// is not a permitted keyboard" - is the rule worth writing. The serial number is
// compiled in at build time and cannot be changed at runtime, so a device with
// no serial is worth querying too.

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

  line("=== USB DEVICE IDENTITY - report this to IT ===");
  line("");
  line("  Bus type      : USB HID keyboard");
  line("  VID           : 0x16c0");
  line("  PID           : 0x27db");
  line("  Manufacturer  : digistump.com");
  line("  Product       : DigiKey");
  line("  Serial number : none");
  line("");
  line("If you found one of these, report it. You do not need to know");
  line("what it did - just that it existed.");
  line("");
  line("Vendor 0x16c0 is the V-USB hobbyist project ID, not a keyboard");
  line("maker. A real attack device would fake a Logitech or Apple ID");
  line("so this screen was empty. Watch for keyboards that have no");
  line("serial number, or that enumerate a second time on plug-in.");
  line("");
  line("Fast check on any Windows machine, PowerShell:");
  line("");
  line("  Get-PnpDevice -Class Keyboard |");
  line("    Select FriendlyName, InstanceId");
  line("");
  line("Look for 0x16c0 in the InstanceId.");

  ledOn();
  delayOrAbort(30000);
  ledOff();
}

void loop() {}
