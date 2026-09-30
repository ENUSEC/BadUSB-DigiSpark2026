// recon/hostid - Tier 2
//
// Machine and user identity: the first thing you want from any target.
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
    "$cs=Get-CimInstance Win32_ComputerSystem;$os=Get-CimInstance Win32_OperatingSystem;$b=Get-CimInstance Win32_BIOS"
    "[pscustomobject]@{ComputerName=$env:COMPUTERNAME;User=\"$env:USERDOMAIN\\$env:USERNAME\";Manufacturer=$cs.Manufacturer;Model=$cs.Model;Serial=$b.SerialNumber;SKU=$b.SystemSKU;OS=$os.Caption;Version=$os.Version;Build=$os.BuildNumber;InstallDate=$os.InstallDate;LastBoot=$os.LastBootUpTime;RAM_MB=[int]($cs.TotalPhysicalMemory/1MB)}|Format-List"
    R_OUT("hostid.txt")
    R_SHOW("hostid.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
