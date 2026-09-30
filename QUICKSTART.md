# QUICKSTART

Getting a Digispark from "I have the board" to "I have typed something" on
Linux, macOS or Windows.

---

## Contents

- [What you need](#what-you-need)
- [Linux](#linux) · [macOS](#macos) · [Windows](#windows)
- [Build your first payload](#build-your-first-payload)
- [Flashing](#flashing)
- [The five-second window](#the-five-second-window-you-will-lose-an-hour-to)
- [Flash budget](#flash-budget)
- [When things break](#when-things-break)

---

## What you need

| | |
|---|---|
| Board | Digispark ATTiny85 (Rev 3 recommended) |
| Toolchain | `arduino-cli`, installed automatically |
| Debug output | Serial monitor (optional, `payloads/detect/usb-enum-logger`) |
| Host | Windows 10/11 for the *targets*; any of the three below to *build* |

Two things before you start, both of which will save you an afternoon:

**The Digistump Boards Manager index is dead.** `digistump.com` is gone and there
is no live mirror, so `arduino-cli core install digistump:avr` cannot resolve
and never will. The platform *source* is still alive on GitHub, so every setup
script here packages it locally and serves a hand-written index pointing at it.
Expect to think of Digispark as "a board with no Boards Manager entry".

**arduino-cli 1.5.x will not install a platform that declares a tool
dependency.** Any `toolsDependencies` entry — even pointing at a tool in the
official Arduino index — fails with the misleading message
`platform is not available for your OS`. The setup scripts therefore declare
**none**: `avr-gcc` is resolved from the separately-installed `arduino:avr`
core, and `micronucleus` is invoked directly by `tools/build.sh` rather than
through arduino-cli's upload target. If you ever add a dependency to that index
and the install starts failing with that exact wording, this is why.

---

## Linux

Tested on Fedora 43. Works the same on Debian/Ubuntu — the package names differ.

```bash
git clone <this-repo> && cd digispark
chmod +x tools/*.sh
./tools/setup-linux.sh
```

That script installs `arduino-cli`, fetches the Digistump platform source,
builds a local package index, serves it on loopback, and installs both cores.
It will print the two packages it needs from your package manager:

```bash
# Fedora
sudo dnf install -y libusb-compat-0.1 libusb1-devel libudev-devel make

# Debian / Ubuntu
sudo apt-get install -y libusb-0.1-0 libusb-1.0-0-dev libudev-dev make
```

**`libusb-compat-0.1` is the one that looks wrong and is not.** The
`micronucleus` binary shipped with Digistump is from 2015 and links against
`libusb-0.1.so.4`. Modern distributions only ship libusb 1.0, so without the
compatibility package you get:

```
micronucleus: error while loading shared libraries: libusb-0.1.so.4: cannot open shared object file
```

Fedora still packages it as `libusb-compat-0.1`. Some Debian derivatives have
dropped it; if yours has, build `micronucleus` from source against libusb 1.0
instead (`git clone https://github.com/micronucleus/micronucleus && cd
commandline && make`).

### Non-root USB access

Flashing talks to a USB device directly. Add a udev rule once:

```bash
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="16c0", MODE="0666"' | \
  sudo tee /etc/udev/rules.d/99-digispark.rules
sudo udevadm control --reload-rules
```

Without it you get `Access denied (insufficient permissions)`. Reboot or
unplug/replug to apply.

---

## macOS

```bash
cd digispark
chmod +x tools/*.sh
./tools/setup-macos.sh
```

Homebrew prerequisites, printed by the script:

```bash
brew install libusb pkg-config
```

macOS differs from Linux in one way that matters: **there is no libusb 0.1 in
modern Homebrew**, so the prebuilt 2015 `micronucleus` will not link. The
script builds `micronucleus` from source instead, which its Makefile handles
natively on Darwin.

If you would rather not build anything:

```bash
brew install libusb-compat    # provides libusb 0.1
# then the prebuilt binary from DigistumpArduino/tools also works
```

### USB access on macOS

macOS has no udev. If flashing fails with a permission error, either run with
`sudo`, or install the official
[macOS libusb driver](https://github.com/michalgrejczyk/DigisparkCDC) and
accept that `micronucleus` will want to be run as root.

---

## Windows

Run PowerShell. If your policy blocks scripts, the `Bypass` flag is enough:

```powershell
git clone <this-repo>
cd digispark
powershell -ExecutionPolicy Bypass -File tools\setup-windows.ps1
```

Requirements: `arduino-cli` (installed by the script), `git`, `tar` (built into
recent Windows), and **Python 3** — the script uses `python -m http.server` to
serve the local package index over loopback, because arduino-cli refuses
`file://` URLs.

Windows uses the **prebuilt** `micronucleus-2.0a4-win.zip` from the Digistump
repository rather than building from source, which would mean standing up
MSYS2/MinGW for one binary.

Make the tools available in future shells:

```powershell
setx PATH "$env:PATH;$env:USERPROFILE\.cache\ds-toolchain\bin"
```

### If flashing fails

`device not found` or `Access denied` on Windows is nearly always about USB
permissions, not a broken build. Run the shell **elevated** — the Micronucleus
bootloader needs USB access a non-admin token may not have.

---

## Build your first payload

```bash
tools/build.sh payloads/tier0/00-smoke
```

Output looks like this:

```
==> 00-smoke
     flash   4392 / 6012 bytes  (73%)   headroom 1620
     SRAM      81 / 512 bytes
 ok  00-smoke compiled -> .build/00-smoke/00-smoke.hex
```

`build.sh` **fails the build** if a payload exceeds the 6012-byte ceiling, so
you find out at compile time rather than at flash time. Compile everything:

```bash
tools/build.sh all
```

---

## Flashing

```bash
tools/build.sh payloads/tier0/00-smoke --flash
```

Then **plug the Digispark in when it tells you to.** The script waits 60
seconds; you have about 5.

If flashing does not start, the reset trick gets you into the bootloader: brief
the two `RST` pins together with a jumper, connect, then release. That forces
the bootloader to run regardless of firmware state.

---

## The five-second window you will lose an hour to

This is the single most common BadUSB-on-Digispark confusion, so read it.

The Digispark boots into **Micronucleus**, a bootloader that presents itself as
a *programming* interface. It waits about **5 seconds** for an upload. If none
arrives, it drops off the USB bus and hands over to your firmware — which then
enumerates as a HID keyboard.

Consequences:

- **Plug in, wait 5s, then look.** The device is not a keyboard for those
  first five seconds. `lsusb` shows "Digistump Micronucleus bootloader".
- **`lsusb` shows nothing at all?** Either the bootloader already timed out, or
  the board is not detected. Unplug and retry.
- **Nothing enumerates after the timeout?** Your firmware probably crashed, or
  it does not fit, or the VID/PID is something the host rejects. Check the size
  report from `build.sh` first — 6012 bytes is a hard ceiling.
- **Windows shows "press Win to search" or an unknown-device warning?** That is
  the removable-device prompt for a non-storage device. Some hardened configs
  refuse it outright — which is exactly what `recon/host-trust` measures.

The bootloader does not need the device to be unplugged to switch: it
disconnects and reconnects itself, so the host sees a device disappear and a
different device appear on the same physical plug.

---

## Flash budget

The real constraint, and it is tighter than it first looks.

```
6012 bytes   total (digispark-tiny.upload.maximum_size)
-2508        V-USB + DigiKeyboard baseline
- 544        duck.h helpers, LED, chords, abort interlock
--------------
~2960        for your payload text
```

`build.sh` reports this for every sketch. A few notes that will save you a
failed build:

- **SRAM is 512 bytes total.** A plain C string literal is copied into SRAM at
  startup. A 700-byte banner compiled naively reports `841 bytes` of globals
  and silently overruns the chip. Every string in this repo is `PROGMEM`, and
  `duck.h` provides `TYPE("...")` / `typeFlash()` for that reason.
- **One concern per payload.** Twenty-four recon collectors in one sketch does
  not fit. Twenty-four one-liners do.
- **Base64 costs 8/3 at type time but nothing extra in flash.** The plaintext
  lives in flash and is encoded on-device. A 600-character PowerShell script is
  ~600 bytes of flash and ~1600 keystrokes.

---

## When things break

| Symptom | Cause | Fix |
|---|---|---|
| `libusb-0.1.so.4` missing | 2015 micronucleus, libusb 0.1 | install `libusb-compat-0.1`, or build micronucleus from source |
| `platform is not available for your OS` | arduino-cli 1.5 rejects any `toolsDependencies` | our index declares none; if you edited it, remove them |
| `Platform 'digistump:avr' not found` after a "successful" install | archive had two nesting levels, files landed one level deep | re-tar with a single top-level dir: `tar czf x.tar.gz digistump-avr` |
| Access denied / device not found | USB permissions | udev rule (Linux), `sudo` (macOS), elevated shell (Windows) |
| `text section exceeds available space` | payload over 6012 bytes | shorten it; `build.sh` reports exact size |
| Device enumerates but types nothing | layout mismatch, or host blocked HID | run `payloads/tier0/00-smoke`; then `recon/host-trust` |
| Garbled characters, especially `@ " \` | DigiKeyboard assumes US layout | use `runPS()`/base64, never raw-typed paths |
| Nothing appears in `lsusb` at all | bootloader already timed out | unplug, replug, flash within 5s |

---

## Scope

Every payload here is for hardware you own, machines you are authorised to
test, or your own lab VM. The recon tier is read-only and writes text files to
`C:\Users\Public\ds-recon`; it installs nothing, changes no settings and sends
nothing off-box. `recon/wifi-profiles` lists saved SSID *names* only and
deliberately does not extract keys. `recon/screenshot` and `recon/clipboard`
read data rather than system state — check your rules of engagement before
using those two anywhere but a lab.

See `README.md` for the payload catalogue and `lab/README.md` for how to run a
repeatable test loop.
