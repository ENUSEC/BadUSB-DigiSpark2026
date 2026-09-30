// detect/usb-enum-logger - Tier 4
//
// A drop-and-forget evidence logger. This is the Digispark running as a CDC
// serial device rather than as a keyboard, logging its own presence over time.
//
// SCOPE - read this before you wire anything up
// A Digispark cannot see other devices on the USB bus. There is no bus
// sniffer for the enumeration phase available here: D+/D- on this chip are the
// same two pins it transmits on, and while it is transmitting its own USB link
// it cannot also listen to the traffic next to it. Anyone claiming a $3 ATtiny85
// enumerates every device on a hub is mistaken about the hardware.
//
// What IS achievable, and is what this does:
//   - a serial log of when the device was inserted
//   - a marker you can stamp from the device itself while on site
//   - a liveness signal, so you can tell "ran and logged" from "dead on arrival"
//
// This is a forensic marker, not a detector. It is useful as evidence that a
// specific device was present, and for answering "was it running the whole
// time?" - which matters, because a payload that ran while the machine was
// locked did considerably less than one that ran at an unattended console.
//
// NO WIRING NEEDED
// Plug it in and read the port:
//
//   ls /dev/hidraw*                      # Linux, after the bootloader exits
//   cat /dev/hidraw0                     # raw, control chars and all
//   screen /dev/hidraw0 115200           # friendlier
//   minicom -D /dev/hidraw0 -b 115200
//
// On macOS the device shows under /dev/tty.usbmodem* or as a "USB Serial"
// device; on Windows it needs the Digistump CDC driver from
// https://github.com/michalgrejczyk/DigisparkCDC.
//
// NOTE ON THE LIBRARY, because it will bite you
// The keyboard stack (DigiKeyboard) and the serial stack (DigiCDC) are
// MUTUALLY EXCLUSIVE on this chip - both drive the same two pins. Including
// both headers in one sketch does not give you a keyboard that also logs; it
// gives you whichever V-USB endpoint the linker kept. This sketch is serial
// only, so nothing here can type.

#include <DigiCDC.h>

#define LED PB1
#define BTN PB2  // on-board button

static void mark(const char *what) {
  SerialUSB.println(what);
  SerialUSB.flush();
  digitalWrite(LED, LOW);
  delay(120);
  digitalWrite(LED, HIGH);
  delay(300);
}

// Blink patterns are the fallback channel: if nobody has the serial port open,
// a short blink still proves the device ran.
static void blinkSos() {
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(LED, LOW);  delay(200);
    digitalWrite(LED, HIGH); delay(200);
    digitalWrite(LED, LOW);  delay(200);
    digitalWrite(LED, HIGH); delay(600);
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(BTN, INPUT_PULLUP);

  SerialUSB.begin();

  mark("=== digispark drop-logger ===");
  SerialUSB.println("Press the button to stamp a marker.");
  SerialUSB.println("Unplug to stop.");
  SerialUSB.flush();

  blinkSos();

  // The Micronucleus bootloader hands the bus to this firmware after its ~5s
  // timeout. Reaching this point means the device is genuinely running and
  // enumerating - not sitting in the bootloader.
  SerialUSB.print("running, millis=");
  SerialUSB.println(millis());
  SerialUSB.flush();
}

void loop() {
  static bool wasDown = false;

  // The CDC stack is polled cooperatively. Without this the host sees a
  // stalled device and drops the port after a second or two.
  SerialUSB.task();

  bool down = (digitalRead(BTN) == LOW);
  if (down && !wasDown) {
    SerialUSB.print("=== marker millis=");
    SerialUSB.print(millis());
    SerialUSB.print(" uptime_s=");
    SerialUSB.print(millis() / 1000);
    SerialUSB.println(" ===");
    SerialUSB.flush();
    digitalWrite(LED, LOW);
    delay(150);
    digitalWrite(LED, HIGH);
  }
  wasDown = down;

  delay(50);
}
