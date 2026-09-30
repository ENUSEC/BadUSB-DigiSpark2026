// recon/users-groups - Tier 2
//
// Local accounts and who is actually an administrator.
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
    "'=== LOCAL USERS ==='"
    "Get-LocalUser -EA 0|Select Name,Enabled,LastLogon,PasswordRequired,PasswordExpires,Description|Format-Table -Auto|Out-String -Width 200"
    "'=== ADMINISTRATORS ==='"
    "Get-LocalGroupMember Administrators -EA 0|Select Name,ObjectClass,PrincipalSource|Format-Table -Auto|Out-String -Width 200"
    "'=== LOCAL GROUPS ==='"
    "Get-LocalGroup|Select Name,Description|Format-Table -Auto|Out-String -Width 200"
    R_OUT("users-groups.txt")
    R_SHOW("users-groups.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
