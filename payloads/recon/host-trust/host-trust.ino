// recon/host-trust - Tier 2
//
// Run this FIRST. Answers the only question that matters before anything else.
//
// Zero-forensics note: nothing is written. This is the cheapest possible
// way to find out whether the rest of the tier is worth running at all.
// Read-only, no elevation, no logs touched.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== HID KEYBOARDS THE HOST WILL ACCEPT ==='"
    "Get-PnpDevice -Class Keyboard -EA 0|Select Status,FriendlyName,InstanceId|Format-List"
    "'=== USB ARRIVALS, LAST 24H ==='"
    "Get-WinEvent -FilterHashtable @{LogName='System';ProviderName='Microsoft-Windows-Kernel-PnP';StartTime=(Get-Date).AddHours(-24)} -EA 0|Select -First 15 TimeCreated,Id,Message|Format-List"
    "'=== WRITEPROTECT / LOCKDOWN ON STORAGE ==='"
    "Get-ItemProperty 'HKLM:\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies' -EA 0|Select WriteProtect,SetLockdownPassword"
    "'=== ATTACK SURFACE VERDICT ==='"
    "'If the HID list is empty or blocked, every payload in this repo is inert here.'"
    R_OUT("host-trust.txt")
    R_SHOW("host-trust.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
