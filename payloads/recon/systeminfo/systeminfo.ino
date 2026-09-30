// recon/systeminfo - Tier 2
//
// Full OS build, hotfix level, locale and uptime.
//
// (no notes)
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "systeminfo /FO LIST | Out-String -Width 200"
    "'=== HOTFIXES ==='"
    "Get-HotFix -EA 0|Select -First 25 HotFixID,Description,InstalledOn|Format-Table -Auto|Out-String -Width 200"
    "'=== UPTIME ==='"
    "((Get-Date)-(Get-CimInstance Win32_OperatingSystem).LastBootUpTime)|Out-String"
    R_OUT("systeminfo.txt")
    R_SHOW("systeminfo.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
