// recon/firewall-rules - Tier 2
//
// Firewall profiles and the enabled rules.
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
    "'=== PROFILES ==='"
    "Get-NetFirewallProfile|Select Name,Enabled,DefaultInboundAction,DefaultOutboundAction,LogAllowed,LogBlocked|Format-Table -Auto|Out-String -Width 200"
    "'=== ENABLED RULES ==='"
    "Get-NetFirewallRule -EA 0|Where-Object {$_.Enabled -eq 'True'}|Select -First 60 DisplayName,Direction,Action,Profile|Format-Table -Auto|Out-String -Width 200"
    "'=== ALLOW ANY ==='"
    "Get-NetFirewallRule -EA 0|Where-Object {$_.Enabled -eq 'True' -and $_.Action -eq 'Allow'}|Select -First 40 DisplayName,Direction,Profile|Format-Table -Auto|Out-String -Width 200"
    R_OUT("firewall-rules.txt")
    R_SHOW("firewall-rules.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
