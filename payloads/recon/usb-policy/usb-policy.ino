// recon/usb-policy - Tier 2
//
// Is untrusted USB actually blocked, and does this host have USB mass storage?
//
// USBSTOR is the storage stack, not HID. A host can block storage while
// still accepting keyboards, which is the usual state and why BadUSB
// still works on 'locked down' machines. host-trust is the real test.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== REMOVABLE DEVICE POLICY ==='"
    "Get-ItemProperty 'HKLM:\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableDevices' -EA 0|Format-List"
    "'=== DEVICE INSTALL RESTRICTIONS ==='"
    "Get-ItemProperty 'HKLM:\\SOFTWARE\\Policies\\Microsoft\\Windows\\DeviceInstall\\Restrictions' -EA 0|Format-List"
    "'=== WRITE PROTECT ==='"
    "Get-ItemProperty 'HKLM:\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies' -EA 0|Select WriteProtect,SetLockdownPassword"
    "'=== USBSTOR DEVICES SEEN (last 30) ==='"
    "(Get-Item 'HKLM:\\SYSTEM\\CurrentControlSet\\Enum\\USBSTOR' -EA 0).GetValueNames()|Select -First 30"
    R_OUT("usb-policy.txt")
    R_SHOW("usb-policy.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
