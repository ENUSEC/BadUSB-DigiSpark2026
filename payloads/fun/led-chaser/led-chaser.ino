// fun/led-chaser - Tier 1
//
// The only payload here that types NOTHING. It just runs the onboard LED.
//
// Worth knowing why that matters: on a hardened host that blocks foreign HID
// keyboards, every other payload in this repo is inert - but the LED still
// runs, because it never touches the USB HID endpoint. So a green chase means
// "device is alive and executing firmware"; a dead chase means the Digispark
// itself is faulty or not flashed, which is a different problem than "the host
// refused to accept the keyboard".
//
// Useful as a first flash to a new board: confirms the build and the device
// without depending on any target-side cooperation.

#include <duck.h>

#define STEPS 40
#define ON_MS 60
#define OFF_MS 160

void setup() {
  duckBegin();

  // Breathe once so you can tell this firmware from the stock blink.
  ledPulse(2, 400, 200);

  // Classic chase: brighten one LED, dim the other, walk down.
  for (int i = 0; i < STEPS; i++) {
    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);
    digitalWrite(0, (i & 1) ? LOW : HIGH);
    digitalWrite(1, (i & 1) ? HIGH : LOW);
    delay(ON_MS);
  }

  // Solid green-ish, then off. "Payload completed" tell.
  ledOn();
  delay(1200);
  ledOff();
}

void loop() {}
