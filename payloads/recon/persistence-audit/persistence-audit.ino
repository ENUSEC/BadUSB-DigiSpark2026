// recon/persistence-audit - Tier 2
//
// Existing autoruns. Finds what is ALREADY there - this payload adds nothing.
//
// Frame this correctly when you report it: an unexpected Winlogon Shell or
// Taskman value means the host is ALREADY compromised and you should stop
// and escalate rather than continue. Do not test the theory in place.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== HKCU RUN ==='"
    "Get-ItemProperty 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run' -EA 0|Format-List"
    "'=== HKLM RUN ==='"
    "Get-ItemProperty 'HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run' -EA 0|Format-List"
    "'=== WOW6432 RUN ==='"
    "Get-ItemProperty 'HKLM:\\Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run' -EA 0|Format-List"
    "'=== STARTUP FOLDERS ==='"
    "Get-ChildItem \"$env:APPDATA\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\",\"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\" -EA 0|Select FullName,LastWriteTime|Format-List"
    "'=== NON-MICROSOFT SCHEDULED TASKS ==='"
    "Get-ScheduledTask -EA 0|Where-Object {$_.TaskPath -notlike '\\Microsoft\\*'}|Select TaskName,TaskPath,State,Author|Format-Table -Auto|Out-String -Width 200"
    "'=== WINLOGON ==='"
    "Get-ItemProperty 'HKLM:\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon' -EA 0|Select Shell,Userinit,Taskman|Format-List"
    R_OUT("persistence-audit.txt")
    R_SHOW("persistence-audit.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
