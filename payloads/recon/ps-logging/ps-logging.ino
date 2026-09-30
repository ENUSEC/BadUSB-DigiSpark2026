// recon/ps-logging - Tier 2
//
// Is PowerShell actually being logged, and has anything been recorded?
//
// This payload runs PowerShell. If script block logging is on, THIS SCRIPT
// IS ALREADY IN THE LOG WITH ITS FULL TEXT. Note the time. It is also a
// live demo of why -EncodedCommand is conspicuous - see train/explain-attack.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== SCRIPTBLOCK LOGGING POLICY ==='"
    "Get-ItemProperty 'HKLM:\\SOFTWARE\\Policies\\Microsoft\\Windows\\PowerShell\\ScriptBlockLogging' -EA 0|Format-List"
    "'=== MODULE LOGGING POLICY ==='"
    "Get-ItemProperty 'HKLM:\\SOFTWARE\\Policies\\Microsoft\\Windows\\PowerShell\\ModuleLogging' -EA 0|Format-List"
    "'=== TRANSCRIPTION POLICY ==='"
    "Get-ItemProperty 'HKLM:\\SOFTWARE\\Policies\\Microsoft\\Windows\\PowerShell\\Transcription' -EA 0|Format-List"
    "'=== EVENT 4104 SAMPLES (IF ANY) ==='"
    "Get-WinEvent -FilterHashtable @{LogName='Microsoft-Windows-PowerShell/Operational';Id=4104} -MaxEvents 3 -EA 0|Select TimeCreated,Message|Format-List|Out-String -Width 250"
    R_OUT("ps-logging.txt")
    R_SHOW("ps-logging.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
