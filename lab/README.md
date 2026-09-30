# lab

Procedure and helpers for hardware testing.

**Nothing here has been run against real hardware yet.** The toolchain and all
42 compiles are verified; enumeration, keystrokes and flashing are not. Fill
in `results/` as you go — the first session is mostly about finding out which
of the documented gotchas you hit.

---

## First session, in order

Do not skip ahead. Each step depends on the previous one passing.

| # | Payload | Passes when |
|---|---|---|
| 1 | `tier0/00-smoke` | `C:\Users\Public\ds-smoke\result.txt` shows `DIGISPARK SMOKE OK @#" ok` |
| 2 | `fun/led-chaser` | LED blinks. Proves HID output with no keyboard involvement |
| 3 | `fun/banner` | Text appears in notepad, layout correct |
| 4 | `fun/hotkey-tour` | Someone can watch all five steps and understand them |
| 5 | `recon/host-trust` | Reports whether the host allows a foreign HID keyboard at all |
| 6 | `recon/systeminfo` | Files appear in `C:\Users\Public\ds-recon\` |
| 7 | `train/explain-attack` | The technical debrief reads correctly, no garbled characters |

Step 1 is the gate. If the character probe comes out wrong, fix `duck.h` before
running anything else — otherwise you will misread 40 other failures as
payload bugs.

### A target worth using

A Windows 10/11 VM, snapshot before each run. Snapshots matter more than they
sound: recon scripts are read-only, but a demo payload that opens Run and
types commands has a way of leaving a Run-dialog entry or a notepad session
behind, and you want a clean baseline for the next run.

---

## Recording a run

```bash
# on the target
lab\capture.cmd
```

or on Linux/macOS against a shared folder:

```bash
./lab/capture.sh /mnt/shared/runs
```

Writes a timestamped directory with the contents of `C:\Users\Public\ds-recon`
plus `lsusb`-style enumeration output. Compare two runs to show what a
collection gap looks like, which is usually the most persuasive artefact in
the whole exercise.

---

## What to actually measure

A demo is not a measurement. These are the questions worth answering, and the
ones people skip:

1. **Did the payload run at all?** Often the honest answer. A device that ran
   while the machine was locked did far less than one that ran at an
   unattended console, and that distinction is the real lesson.
2. **Did anyone notice?** `train/notice-poll` exists for this. Ask whether they
   noticed, not whether they enjoyed it — the latter always scores high.
3. **What stopped it, if anything?** USB allow-listing, hardened Winlogon
   shell, EDR script-block logging, or nothing. Record the controls that fired
   *and* the ones that did not.
4. **How long from plug-in to keystrokes?** Includes the ~5s bootloader
   timeout. Real-world delivery is slower than demos suggest, and knowing the
   actual number is useful.

Record negative results. "The allow-list blocked it and we never saw an
enumeration" is a finding, and a more valuable one than a clean run.

---

## Detecting it yourself

The Tier 4 sketches are experiments, not products. Before you spend time on
them, try the boring detection first — on a host you control:

```powershell
# Script block logging. The -EncodedCommand flag is the signal, and it is a
# reliable one: there is no benign reason to base64 a PowerShell command.
Get-EventLog Security -InstanceId 4104 | Select -ExpandProperty Message |
  Select-String -Pattern '-enc'

# USB device allow-listing. The Digispark announces VID 0x16c0 PID 0x27db.
Get-PnpDevice | Where-Object InstanceId -match 'VID_16C0&PID_27DB'

# Unknown-device events during the bootloader window - the bootloader
# presents as a programming device first.
Get-WinEvent -FilterHashtable @{LogName='System';Id=20001,20003} -MaxEvents 20
```

A commercial USB allow-list (or a hardened Winlogon shell) blocks this
outright. Note that blocking *mass storage* does **not** block a keyboard —
separate device classes, and the keyboard class is the one that is allowed by
default everywhere.

---

## Tier 4 hardware notes

Two sketches need a tap, and both need to be read before wiring.

### Cannot be done: USB bus sniffing

**A Digispark cannot enumerate other devices on the bus.** D+/D− on this chip
are the same two pins it transmits on, and it cannot transmit its own link and
listen to the adjacent one at the same time. There is no $3 bus enumerator;
anyone who says otherwise is describing a different device.

### What `passive-sniffer` and `keystroke-detector` actually do

They observe an **adjacent** bus segment — useful when the host has an OTG or
dual-role port and you can put the tap on a segment the Digispark is not
driving. They count edges and match the sync-field shape. They are **not**
decoders, and they will not recover a keystroke stream.

Wiring: tap D+ and D− of the segment being observed, common ground to the
Digispark GND, and expect the LED patterns to mean roughly what the comments
say and no more. Validate the tap with a USB traffic generator before
believing a null result — "no activity" and "tap wired wrong" look identical
from the LED.

`usb-enum-logger` needs no wiring and is the only Tier 4 sketch that is
genuinely useful today. It runs as CDC serial rather than as a keyboard (the
two stacks are mutually exclusive on this chip — they drive the same pins), so
it is a drop-and-forget evidence marker: it logs insertion time, accepts button
presses as timestamped markers, and blinks even if nobody opens the port.
