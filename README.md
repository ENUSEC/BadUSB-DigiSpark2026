# digispark-badusb-lab

A Digispark (ATtiny85) payload collection, in tiers, for authorised
security testing and awareness work.

Raw Arduino C++ — no DuckyScript preprocessor, no payload generator. Every
sketch is readable on its own, and they share a small helper library so the
interesting parts are the payload logic rather than keystroke plumbing.

```
git clone <this-repo> && cd digispark
./tools/setup-linux.sh          # or setup-macos.sh / setup-windows.ps1
tools/build.sh payloads/tier0/00-smoke
tools/build.sh payloads/tier0/00-smoke --flash
```

Full setup instructions, including the two toolchain problems that will
otherwise cost you an afternoon, are in **[QUICKSTART.md](QUICKSTART.md)**.

---

## Tiers

Ordered by how much they assume. Work down; do not skip Tier 0.

| Tier | Directory | Count | What it is |
|---|---|---|---|
| 0 | `payloads/tier0/` | 1 | Does the device work at all? Enumeration, host policy, layout |
| 1 | `payloads/fun/` | 8 | Benign demos, for testing mechanics and for a room to watch |
| 2 | `payloads/recon/` | 24 | Read-only collection. Writes files, sends nothing |
| 3 | `payloads/train/` | 5 | Awareness material, for the people who were just demonstrated on |
| 4 | `payloads/detect/` | 4 | Detection experiments. Two need wiring; all are unproven |

**42 sketches, all compiling, all under the 6012-byte ceiling.**

### Tier 0 — validate before you trust anything

| Sketch | Checks |
|---|---|
| `00-smoke` | Enumerates as HID; host accepts a foreign keyboard; ASCII and US/UK-differing characters type correctly; the base64 PowerShell path runs |

If `00-smoke` does not pass, nothing else in the repo will work, and the
payload comments will not tell you why. Run it first, every time you change
`duck.h`.

### Tier 1 — benign demos

`banner`, `flip-screen`, `hotkey-tour`, `led-chaser`, `matrix-rain`, `morse`,
`paint-scribble`, `theme-toggle`.

These exist because "it typed my payload" and "I saw it happen" are different
claims, and only the second one convinces a room. None of them persist,
collect, or exfiltrate anything.

### Tier 2 — recon (24, read-only)

Grouped by question:

- **Identity / posture** — `host-trust`, `hostid`, `systeminfo`, `domain`, `av-edr`, `defender-status`, `usb-policy`
- **Exposure** — `wifi-profiles`, `remote-access`, `smb-shares`, `bitlocker`, `firewall-rules`
- **Accounts** — `users-groups`, `last-logon`, `processes`, `software`
- **Live context** — `window-titles`, `clipboard`, `smartcard-reader`, `ps-logging`
- **Persistence / egress** — `persistence-audit`, `egress-beacon`, `idle-lock`, `screenshot`

Every one of these writes plain text to `C:\Users\Public\ds-recon\`. Nothing
installs, nothing modifies settings, nothing leaves the machine. Read
`payloads/recon/README.md` before running them on anything you care about.

Two are worth reading the comment on before you run them anywhere but a lab:
`screenshot` and `clipboard` read user data rather than system state, and
`wifi-profiles` lists saved SSID *names* but deliberately does not extract
keys.

### Tier 3 — training

`print-iocs`, `simulation-banner`, `explain-attack`, `prefill-ticket`,
`notice-poll`.

`explain-attack` is the one worth the flash: it types out what the previous
payloads did, what they did not do, what would have stopped them, and — the
useful part — that `-EncodedCommand` is an EDR signature. Being able to say
"our own tooling flagged us" lands better than any amount of hand-waving.

`notice-poll` is how you turn a demo into a measurement. Ask whether people
noticed, not whether they enjoyed it.

### Tier 4 — detection (unproven, needs hardware)

| Sketch | Needs | Status |
|---|---|---|
| `usb-enum-logger` | nothing | Usable. CDC serial; blinks if you never open the port |
| `passive-sniffer` | D+/D− tap | Counting only, **not** a packet decoder |
| `keystroke-detector` | D+/D− tap | Sync-pattern match, **not** a full decoder |
| `touch-latch` | wire to a target pin | Works, but tells you little on its own |

Read the SCOPE comment at the top of each before wiring anything. In
particular: **a Digispark cannot sniff the USB bus.** The two pins it
transmits on are the two pins it would have to listen on, and it cannot do
both. The D+/D− tap sketches observe an *adjacent* bus segment on a host with
an OTG-capable port; they are not a general enumerator.

---

## Layout

```
lib/duck/duck.h          shared helpers (see below)
tools/setup-linux.sh     toolchain: Linux
tools/setup-macos.sh     toolchain: macOS
tools/setup-windows.ps1  toolchain: Windows
tools/build.sh           compile / flash / size-enforce / lint
payloads/                the 42 sketches, one directory each
lab/                     test procedure and a capture helper
QUICKSTART.md            per-OS setup, first flash, troubleshooting
.build/                  hex output (generated, gitignored)
```

---

## `lib/duck/duck.h`

| Helper | Purpose |
|---|---|
| `duckBegin()` | LED start-up blink + PB2 abort interlock. Returns false if aborted |
| `TYPE("...")` | Type a literal. Compiles to flash — **not** a bare `DigiKeyboard.print` |
| `typeFlash(S)` | Type a `PROGMEM` array |
| `runPS(S, hidden, ms)` | Win+R → `powershell -nop -w h -enc <base64>` |
| `recon(S)` | Standard collector wrapper, opens `C:\Users\Public\ds-recon` |
| `press(k)` / `chord()` | Keystroke + modifier helpers |
| `delayOrAbort(ms)` | Delay that the PB2 button can cut short |
| `R_DIR` `R_MKDIR` `R_OUT` `R_SHOW` | Ready-made PowerShell path fragments |

### Two things that will bite you

**1. String literals live in SRAM, and there are only 512 bytes.**

On AVR a `const char *x = "text";` puts a pointer in SRAM and copies the
literal there at startup. A 700-byte banner compiled that way reports
`841 bytes` of globals and silently overruns the chip. Everything in this
repo uses `PROGMEM` arrays and `TYPE()`.

The subtler version of this bug *compiles cleanly and then types garbage*,
because `typeFlash()` and `runPS()` read their argument with
`pgm_read_byte()` — which reads flash. Hand either one a bare literal and it
reads a RAM address. `tools/build.sh` lints for exactly this and fails the
build:

```
lint  42:  typeFlash("@#$^");
FAIL  00-smoke.ino: flash-reading function called with a bare literal.
```

**2. `DigiKeyboard` assumes a US layout, whatever the host is using.**

HID sends a usage code per key; the character comes from the *OS* layout. So
the key labelled `@` on a US keyboard only produces `@` on a US host, and
something else entirely on UK. Every raw-typed path, quote, backslash or
colon in a payload is therefore a coin flip on a non-US machine.

`runPS()` is the fix: Base64 of UTF-16LE, typed as `A-Z a-z 0-9 + / =`, which
are in the same physical place on US and UK. The plaintext never becomes
keystrokes, and the command line survives intact. It also costs nothing extra
in flash — the plaintext is stored as a `PROGMEM` array and encoded on-device.

This is the single most common "it worked on my laptop" failure in BadUSB
work, and it is why `00-smoke` prints a character probe including `@ # " \ { }`.

---

## Flash budget

```
6012 bytes   digispark-tiny total
-2508        V-USB + DigiKeyboard baseline (measured, bare library)
- 544        duck.h: LED, chords, abort interlock, base64 encoder
--------------
~2960        available for your payload text
```

Measured across all 42 sketches: **3182 min, 5772 max, 4449 average.** The
ceiling is real and `build.sh` treats an overflow as a build failure rather
than letting you find out at flash time.

Practical consequences:

- **One concern per sketch.** The 24 recon collectors are 24 separate one-liner
  sketches, not one combined sketch, precisely because they do not fit
  together.
- **Prose is expensive.** `explain-attack` is 96% of budget. Training content
  is a design constraint, not an afterthought.
- **`-enc` costs nothing in flash.** A 600-character script is 600 bytes of
  `PROGMEM` and ~1600 keystrokes at run time.

When a sketch will not fit, arduino-cli's only complaint is
`text section exceeds available space`, with no byte count. `build.sh`
translates that into something actionable.

---

## Testing

Nothing in this repo has been run against real hardware. The toolchain and
all 42 compiles are verified; enumeration, typing and flashing are not.

`lab/README.md` has the procedure, and `lab/capture.sh` pulls the target-side
output into a timestamped directory for comparison across runs.

The first hardware session should be: `00-smoke`, then two or three Tier 1
demos with a person watching, then `recon/host-trust` (does the host even allow
this keyboard?), and only then the rest of Tier 2.

---

## Scope

For hardware you own, systems you are authorised to test, and your own lab
VMs. The recon tier is read-only by design. `payloads/recon/README.md`
documents what each collector does *and does not* do; that second half matters
as much as the first when you are briefing a client on what was collected.

If a control blocked something, that is a finding — record it in
`lab/results/`, including the ones that make the exercise look less
impressive.
