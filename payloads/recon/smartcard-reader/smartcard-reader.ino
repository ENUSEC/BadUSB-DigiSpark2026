// recon/smartcard-reader - Tier 2
//
// Badge reader presence and any smartcard in the current user's store.
//
// A workforce that badges in makes physical presence a real control. If a
// badge reader exists but is not used, that is a conversation to have - and
// it tells you how long you have before the badge itself shows up next to
// the keyboard.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== SMART CARD PNP DEVICES ==='"
    "Get-CimInstance Win32_PnPEntity -Filter \"PNPClass='SmartCard'\" -EA 0|Select Name,Status,DeviceID|Format-List"
    "'=== SMART CARD SERVICE ==='"
    "Get-Service SCardSvr -EA 0|Select Status,StartType,DisplayName|Format-List"
    "'=== CERTIFICATES IN USER STORE ==='"
    "Get-ChildItem Cert:\\CurrentUser\\My -EA 0|Select Subject,Issuer,Thumbprint,NotAfter|Format-List"
    "'=== SCARD READER EVENTS (last 10) ==='"
    "Get-WinEvent -FilterHashtable @{LogName='Microsoft-Windows-SmartCard/Operational'} -MaxEvents 10 -EA 0|Select TimeCreated,Id,Message|Format-List|Out-String -Width 250"
    R_OUT("smartcard-reader.txt")
    R_SHOW("smartcard-reader.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
