// fun/theme-toggle - Tier 1
//
// Flips Windows between light and dark mode.
//
// This is a small, fully reversible cosmetic change, which is exactly why it
// is filed under "fun" rather than recon - but it is also a legitimate
// persistence/mechanism proof: it demonstrates that the payload can reach into
// a per-user registry hive (HKCU) without elevation, from a device that
// presents as storage.
//
// If you want to flip it programmatically rather than by hunting for the
// settings pane, HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize
// has AppsUseLightTheme / SystemUsesLightTheme as DWORDs - but that needs
// base64'd PowerShell and logs the user out to apply, so the UI route below is
// the honest demo.

#include <duck.h>

void setup() {
  duckBegin();
  ledPulse(2, 150, 150);

  // Win+A opens Quick Settings in Windows 10/11.
  gui(KEY_A);
  delay(1800);

  // The accent/theme swatches sit below the brightness slider. Step down to
  // the "light / dark" pair and toggle.
  press(KEY_ARROW_DOWN);
  delay(600);
  press(KEY_ARROW_DOWN);
  delay(600);
  press(KEY_ARROW_RIGHT);
  delay(600);
  press(KEY_ENTER);

  ledOn();
  delayOrAbort(6000);

  press(DUCK_KEY_ESC);
  delay(600);
  ledOff();
}

void loop() {}
