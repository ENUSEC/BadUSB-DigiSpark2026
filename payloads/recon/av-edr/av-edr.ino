// recon/av-edr - Tier 2
//
// Which endpoint security is watching this host, and is it actually running.
//
// Product-state bitmask in SecurityCenter2 is 0x10(16) for disabled,
// 0x00 for on and up to date. Worth knowing before you trust a blank.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== REGISTERED AV (SecurityCenter2) ==='"
    "Get-CimInstance -Namespace root/SecurityCenter2 -ClassName AntiVirusProduct -EA 0|Select displayName,productState,pathToSignedProductExe|Format-List"
    "'=== CANDIDATE EDR SERVICES ==='"
    "Get-Service -EA 0|Where-Object {$_.DisplayName -match 'defender|sentinel|carbon|crowdstrike|falcon|sophos|cylance|mcafee|norton|kaspersky|eset|trend micro|avast|bitdefender|wdfilter|fireeye|panther|falcon|attackiq'}|Select Status,StartType,DisplayName|Format-Table -Auto|Out-String -Width 200"
    "'=== CANDIDATE EDR PROCESSES ==='"
    "Get-Process -EA 0|Where-Object {$_.ProcessName -match 'MsMpEng|WdFilter|SentinelOne|CSfLAgent|SentinelAgent|carbon|CylanceSvc|SophosUI|avp|ccSvc|ekrn|fshoster|CylanceSvc'}|Select ProcessName,Id|Format-Table -Auto|Out-String"
    R_OUT("av-edr.txt")
    R_SHOW("av-edr.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
