// recon/processes - Tier 2
//
// Running processes with full command lines.
//
// CommandLine is where the interesting things hide: an analyst's tooling,
// an IR tool, a reverse shell. This is how you spot a hostile machine
// before you leave anything behind on it.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "Get-CimInstance Win32_Process -EA 0|Select ProcessId,ParentProcessId,Name,ExecutablePath,CommandLine|Sort Name|Format-Table -Auto|Out-String -Width 250"
    "'=== PROCESS COUNT ==='"
    "(Get-Process).Count"
    "'=== LISTENERS ==='"
    "Get-NetTCPConnection -State Listen -EA 0|Select LocalAddress,LocalPort,OwningProcess|Sort LocalPort|Format-Table -Auto|Out-String -Width 200"
    R_OUT("processes.txt")
    R_SHOW("processes.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
