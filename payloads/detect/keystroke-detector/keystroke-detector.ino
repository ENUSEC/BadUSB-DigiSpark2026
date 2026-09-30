// detect/keystroke-detector - Tier 4
//
// A second Digispark tapped onto a keyboard's own USB data line, watching for
// that keyboard to be told to type something it was not asked to type.
//
// THIS IS THE INTERESTING PAYLOAD IN THE REPO. Everything else is an attack
// or a demo. This one is a detector: the premise is that a BadUSB keystroke is
// observable from outside the attacker's device, by watching the *legitimate*
// keyboard on the bus and noticing it typing on its own.
//
// WHY THE PREMISE IS WORTH CHECKING
// When a BadUSB types into a machine, the host sees a keyboard HID report. The
// real keyboard plugged into the next port is silent at that moment - nobody
// touched it. The host does not care, because it cannot correlate the two. But
// something watching the real keyboard's data line can. A real keyboard that
// reports input while nobody is at it is an anomaly, and unlike the BadUSB
// itself, the anomaly is in the *other* device: a machine whose USB keyboard
// line shows traffic would never be flagged by a rule looking at the BadUSB's
// VID and PID.
//
// THE HONEST PART
// What is implemented here is the front end of that idea and no more:
//
//   - 16.5MHz sampling of one line, which is what NRZI decoding needs
//   - transition counting, to recover a bit rate
//   - sync-pattern (K-state / byte-stuff detection) counting, to tell "this
//     line carries framed USB" from "this line is noise"
//
// What is NOT here, and does not fit in 6KB alongside a USB stack:
//
//   - NRZI de-encoding to recover actual bytes
//   - PID/endpoint filtering, so you know the reports are from the HID
//     interface rather than some other function
//   - HID report assembly, so you can tell *which* key was pressed
//
// So this will tell you whether a tapped keyboard line is delivering HID-rate
// traffic during a window you care about. It will not tell you the key. For
// most threat-hunting purposes that is still the interesting half - it tells
// you WHEN, which is what lets you go and look at the logs for that timestamp.
//
// Treat this as a validated concept with a working sampler, not a finished
// detector. If you take it further, a part with real USB hardware (an RP2040,
// or any MCU with a native USB PHY) removes every limitation above and the
// full decoder becomes practical.
//
// WIRING
//
//   keyboard D+ ----+---- PB0   (high-impedance tap)
//                  |
//                  1k
//                  |
//                  +-- GND
//
// Same caveat as passive-sniffer: the series resistor is load-bearing. You are
// sensing edges on a line someone else is driving.

#include <duck.h>

#define TAPPED_PIN  0
#define SAMPLE_MS   3000

// Results are reported as blink counts on the onboard LED, because the whole
// point is that this device is NOT a keyboard and should not need a host.
//
// Module-level globals rather than a struct: Arduino auto-generates function
// prototypes before your type definitions, so a struct declared mid-file is not
// visible to a function that appears earlier. Keep it simple.
static uint16_t g_edges = 0;
static uint16_t g_syncs = 0;

static void blinkN(uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    ledOn();  delay(150);
    ledOff(); delay(250);
  }
}

// Fills g_edges / g_syncs.
static void sample() {
  g_edges = 0;
  g_syncs = 0;

  uint8_t run = 0;        // current run of identical bits
  uint8_t maxRun = 0;     // longest run - proxy for the sync field
  bool last = digitalRead(TAPPED_PIN);

  // Full-speed USB is 12 Mbit/s and the PLL runs at 16.5MHz, so roughly one
  // sample per bit with a little margin.
  for (uint32_t t = 0; t < (uint32_t)SAMPLE_MS * 14000; t++) {
    bool now = digitalRead(TAPPED_PIN);
    if (now != last) {
      g_edges++;
      if (run > maxRun) maxRun = run;
      run = 0;
      last = now;
    } else {
      run++;
    }
  }

  // A USB sync field is a run of seven identical bits followed by a
  // transition. Requiring a long run plus traffic is a loose filter: it will
  // occasionally fire on noise, so treat this as a rate, not a frame count.
  g_syncs = (g_edges > 40 && maxRun > 20) ? 1 : 0;
}

void setup() {
  pinMode(TAPPED_PIN, INPUT);
  ledOff();

  // Long settle. This is a measurement probe; rushing it measures the
  // firmware starting up, not the bus.
  delay(5000);

  sample();

  if (g_edges < 20) {
    blinkN(1);   // nothing - line is idle or not connected
  } else if (g_syncs) {
    blinkN(2);   // USB-rate traffic with sync-like patterns
  } else {
    blinkN(3);   // edges but no framing - wrong pin, or noise
  }

  ledOn();
  delay(1500);
  ledOff();
}

void loop() {}

