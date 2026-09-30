# Tier 2 — recon

24 read-only collectors. One concern per sketch, because the flash ceiling is
6012 bytes and they do not fit together.

Every collector here:

- writes plain text to `C:\Users\Public\ds-recon\`
- changes no settings, installs nothing, adds no persistence
- sends nothing off the machine
- returns the keyboard to the host when done

Read the **Does not** line in a sketch before using it anywhere but a lab.
That second half is what you brief a client on.

---

## Identity and posture

| Sketch | Answers | Does **not** |
|---|---|---|
| `host-trust` | Does the host allow a foreign HID keyboard at all? | Change the policy |
| `hostid` | Machine name, domain, BIOS serial | — |
| `systeminfo` | OS build, CPU, RAM, uptime, last boot | — |
| `domain` | Domain membership, trusts, functional level | Enumerate AD beyond the local query |
| `av-edr` | Which security products are installed | Disable or interact with them |
| `defender-status` | Real-time protection, tamper protection, policy | Turn anything off |
| `usb-policy` | Registry USB storage/ HID restrictions, device allow-lists | Modify any of them |

`host-trust` is the one to run first. If the host blocks foreign HID
keyboards, every other collector here is a no-op, and you have a finding
rather than a gap.

## Exposure

| Sketch | Answers | Does **not** |
|---|---|---|
| `wifi-profiles` | Saved SSID names, signal, auth type | **Extract PSKs** |
| `remote-access` | Installed remote-access agents, listening ports | Connect to anything |
| `smb-shares` | Admin shares, `net view`, open handles | Touch the shares |
| `bitlocker` | Volume encryption state, recovery-key count | Dump or escrow keys |
| `firewall-rules` | Enabled profiles, active rules, listening ports | Modify rules |

`wifi-profiles` stops at SSID names on purpose. Key extraction is a different
activity with a different authorisation requirement, and the payload is
deliberately not that.

## Accounts

| Sketch | Answers | Does **not** |
|---|---|---|
| `users-groups` | Local admins, group membership, last-logon | — |
| `last-logon` | Interactive and remote logon history, failed attempts | — |
| `processes` | Running processes, listening sockets, loaded drivers | Kill or inject |
| `software` | Installed programs, versions, patch level | Uninstall or update |

## Live context

Read the comment on these two before running them outside a lab. They touch
user data rather than system state, and "we did not anticipate that" is worth
more in a rules-of-engagement conversation than a slightly lower risk rating.

| Sketch | Answers | Does **not** |
|---|---|---|
| `window-titles` | Titles of open windows — what the user is working on | Read file contents |
| `clipboard` | Current clipboard text | Capture a history |
| `smartcard-reader` | Connected smart-card readers, CSP PIN status | Use a card or a credential |
| `ps-logging` | Whether PowerShell script-block logging is on, and events | Enable it |

## Persistence and egress

| Sketch | Answers | Does **not** |
|---|---|---|
| `persistence-audit` | Run keys, startup folders, scheduled tasks, services | Create or remove any |
| `egress-beacon` | Reachability of a listener, DNS resolution, proxy config | **Exfiltrate data** |
| `idle-lock` | Console session state and lock status | Lock or unlock |
| `screenshot` | Saves one screenshot for the exercise | Upload it anywhere |

`egress-beacon` confirms whether a path out exists — which is what you need to
scope the exercise. It transmits no collected data, and the URL in it is a
placeholder you must point somewhere you control.

---

## Running them

```bash
tools/build.sh payloads/recon/systeminfo --flash
```

Output lands in `C:\Users\Public\ds-recon\`. Collect it with `lab/capture.cmd`
on the target, or `lab/capture.sh` from a shared folder, and diff two runs.

`00-smoke` must pass first. If it does not, recon failures will be
enumeration problems rather than payload problems.
