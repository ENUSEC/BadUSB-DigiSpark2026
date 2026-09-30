// detect/passive-sniffer - Tier 4
//
// Counts USB Start-Of-Frame patterns on a tapped data line, as a cheap
// "is there traffic on this wire" probe.
//
// SCOPE - what this is and is not
// This counts sync patterns. It does NOT decode USB packets. A full decoder
// needs NRZI de-coding, bit unstuffing, CRC16 and endpoint tracking, and the
// useful part of that does not fit in 6KB alongside a USB stack. Claiming
// otherwise in a report would be a good way to lose credibility.
//
// What fits comfortably is enough for a genuinely useful question: is anything
// talking on this line, and at what rate?
//
// Why it is still worth building. USB full-speed frames every 1ms, and each
// frame begins with a SOF whose sync field is a 7-bit pattern 0000001. Count
// SOFs, count milliseconds of USB activity. A keyboard sends one interrupt
// packet per keystroke; a mouse one per motion; an idle bus sends almost
// nothing. The difference between "human typing" and "idle" is obvious from
// the frame count alone - you do not need to know which keys.
//
// WIRING - this is a high-impedance tap, not a connection
//
//   target D+  ----+----  PB0 on the sniff Digispark
//                 |
//                1k
//                 |
//                 +---GND   (target ground, common reference required)
//
// The 1k series resistor matters. Connecting D+ straight to a GPIO risks
// fighting the pull-up on the target and corrupting real traffic. You are
// borrowing a few microamps to sense an edge, not driving the line. Do this on
// hardware you own.
//
// PB0 is also the D- pin of the sniff device's OWN usb link. As shipped the
// Digispark does not enumerate as anything once firmware runs, so the pin is
// free. If you need it to keep enumerating, move the tap to a free pin and
// change TAPPED_PIN.
//
// RESULT ENCODING (no serial output, so it works when unplugged from a host)
//   solid LED      - traffic seen, rate above threshold
//   slow blink     - traffic seen, rate low
//   double blink   - no traffic detected
//   LED off        - never sampled

#include <duck.h>

#define TAPPED_PIN   0      // PB0
#define SAMPLE_MS    2000   // sampling window
#define HIGH_RATE    12     // SOFs per window considered "active"

// Count edges and sync-like runs on the tapped line over one window.
//
// Deliberately dumb: this is not a packet decoder. It answers "is anything
// driving this line, and does it look like USB-rate traffic". D+ idles high and
// the bus is bursty, so edge count alone is meaningful - it separates an active
// keyboard from an idle one - while the sync test is a loose filter that will
// occasionally fire on noise.
static uint16_t g_edges = 0;
static uint16_t g_runs = 0;

static void sample() {
  g_edges = 0;
  g_runs = 0;

  uint8_t run = 0;
  uint8_t maxRun = 0;
  bool last = digitalRead(TAPPED_PIN);

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

  // A run of seven or more identical bits terminated by a transition is
  // sync-field shaped. Count runs, not frames - this over-reports.
  if (g_edges > 40 && maxRun > 20) g_runs++;
}

static void blinkN(uint8_t n, uint16_t onMs, uint16_t gapMs) {
  for (uint8_t i = 0; i < n; i++) {
    ledOn();  delay(onMs);
    ledOff(); delay(gapMs);
  }
}

void setup() {
  pinMode(TAPPED_PIN, INPUT);

  // Boot with the LED off so "off" unambiguously means no traffic sampled.
  ledOff();

  // Let the target bus come up before measuring it.
  delay(3000);

  sample();

  if (g_edges < 20) {
    blinkN(2, 500, 300);   // no traffic
  } else if (g_runs) {
    blinkN(1, 1200, 200);  // USB-rate traffic seen
  } else {
    blinkN(3, 200, 200);   // edges but no framing - wrong pin, or noise
  }
}

void loop() {}
