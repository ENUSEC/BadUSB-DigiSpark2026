// fun/matrix-rain - Tier 1
//
// A Matrix-rain console effect, via PowerShell.
//
// This is the one "fun" payload that uses the base64 path, because the script
// is full of $, |, { } and quotes that a raw-typed payload would corrupt on a
// UK layout. It is also the payload most likely to trip Defender, which makes
// it a useful accidental demo of why EDR exists - see payloads/train/.
//
// Note the effect is console-only and self-terminating. It does not persist,
// and it does not touch any setting.

#include <duck.h>

static const char SCRIPT[] PROGMEM =
    "$c=New-Object System.Console;"
    "$w=$Host.UI.RawUI.WindowSize;"
    "Add-Type -Name W -Namespace N -MemberDefinition '[DllImport(\"user32.dll\")]public static extern int SetConsoleTitle(string t);';"
    "$cs=@('%','#','*','+','-','.','0','1','2','3','4','5','6','7','8','9');"
    "$end=$w.Height;"
    "while($end -gt 0){"
    "$y=$w.CursorTop;"
    "$x=$w.CursorLeft;"
    "$w.SetBufferContents([System.Console]::GetBufferContents([System.Console]::BufferArea),0,[System.Console]::BufferArea);"
    "for($i=0;$i -lt 40;$i++){"
    "$w.SetCursorPosition((Get-Random -Maximum $w.BufferWindow.Width),((Get-Random -Maximum $end)));"
    "Write-Host -NoNewline ($cs[(Get-Random -Maximum $cs.Length)]) -ForegroundColor Green};"
    "Start-Sleep -Milliseconds 90};"
    "Write-Host 'Wake up.' -ForegroundColor Green";

void setup() {
  duckBegin();
  ledPulse(2, 150, 150);

  // Visible, not hidden: the point is that a person watches this happen.
  // It also self-terminates, so we do not need to kill anything afterwards.
  runPS(SCRIPT, false, 1200);

  ledOn();
  delayOrAbort(20000);
  ledOff();
}

void loop() {}
