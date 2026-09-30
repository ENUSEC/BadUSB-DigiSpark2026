// recon/last-logon - Tier 2
//
// Who is logged in right now, and how. Times your engagement window.
//
// LogonType 2 = interactive, 7 = unlock, 10 = remote interactive, 11 =
// cached interactive. Type 2 with a session you did not expect means
// someone is watching. Check before you fire anything visible.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== CONSOLE USER ==='"
    "query user 2>&1|Out-String"
    "'=== INTERACTIVE LOGON SESSIONS (types 2,7,10,11) ==='"
    "Get-CimInstance Win32_LogonSession -EA 0|Where-Object {$_.LogonType -in 2,7,10,11}|Select LogonId,LogonType,StartTime|Format-Table -Auto|Out-String -Width 200"
    "'=== EXPLORER SESSION IDS ==='"
    "Get-Process explorer -IncludeUserName -EA 0|Select Id,SessionId,UserName|Format-Table -Auto|Out-String -Width 200"
    R_OUT("last-logon.txt")
    R_SHOW("last-logon.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
