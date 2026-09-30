// recon/wifi-profiles - Tier 2
//
// SSIDs this machine has joined. Useful for site survey and physical context.
//
// Deliberate scope limit: this lists PROFILE NAMES only.
// netsh wlan show profile name=X key=clear would extract the actual PSK,
// and that is credential theft, not reconnaissance. It is not implemented
// here on purpose. Add it only if your rules of engagement say so.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== SAVED PROFILES ==='"
    "netsh wlan show profiles 2>&1|Out-String -Width 200"
    "'=== CONNECTED ==='"
    "netsh wlan show interfaces 2>&1|Out-String -Width 200"
    R_OUT("wifi-profiles.txt")
    R_SHOW("wifi-profiles.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
