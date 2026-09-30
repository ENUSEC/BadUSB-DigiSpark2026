// recon/bitlocker - Tier 2
//
// Full-disk encryption state per volume.
//
// ProtectionStatus of 'Off' on a company laptop is the finding. Flag it.
// Get-BitLockerVolume needs elevation on most builds; manage-bde does not.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== GET-BITLOCKERVOLUME ==='"
    "Get-BitLockerVolume -EA 0|Select MountPoint,VolumeStatus,ProtectionStatus,EncryptionPercentage,EncryptionMethod|Format-Table -Auto|Out-String -Width 200"
    "'=== MANAGE-BDE -STATUS ==='"
    "manage-bde -status 2>&1|Out-String -Width 200"
    R_OUT("bitlocker.txt")
    R_SHOW("bitlocker.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
