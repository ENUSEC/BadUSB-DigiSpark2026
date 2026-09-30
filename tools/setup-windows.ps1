<#
.SYNOPSIS
    Build the Digispark toolchain on Windows.

.DESCRIPTION
    Same shape as the Linux and macOS scripts, but three things differ:

    1. arduino-cli's install script is PowerShell, not sh. We just drop the
       .zip on PATH instead.

    2. micronucleus is used as the PREBUILT win binary from the Digistump
       repository (tools/micronucleus-2.0a4-win.zip). Building it from source
       on Windows means standing up MSYS2 and MinGW just for this step, which
       is not worth it - the prebuilt binary is what the Digististump project
       ships and tests against.

    3. arduino-cli on Windows needs the WinUSB/libusb filter driver to talk to
       a Micronucleus bootloader, and access to the USB device can require
       running elevated. If you hit "Access denied" or "device not found",
       that is almost always the cause - not a broken build.

.PARAMETER Port
    Loopback port for the local package index. Default 8765.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\setup-windows.ps1
#>

[CmdletBinding()]
param(
    [int]$Port = 8765,
    [string]$Work = "$env:USERPROFILE\.cache\ds-toolchain"
)

$ErrorActionPreference = 'Stop'

function Say  ($m) { Write-Host "`n==> $m" -ForegroundColor Cyan }
function Warn ($m) { Write-Host "!!  $m" -ForegroundColor Yellow }
function Die  ($m) { Write-Host "xx  $m" -ForegroundColor Red; exit 1 }

New-Item -ItemType Directory -Force -Path "$Work\bin" | Out-Null
$env:PATH = "$Work\bin;$env:PATH"

# ------------------------------------------------------------- arduino-cli
Say "arduino-cli"
if (-not (Get-Command arduino-cli -ErrorAction SilentlyContinue)) {
    Say "downloading arduino-cli for Windows"
    $arch = if ([Environment]::Is64BitOperatingSystem) { "64bit" } else { "32bit" }
    $url  = "https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_${arch}.zip"
    $zip  = "$Work\arduino-cli.zip"
    Invoke-WebRequest -Uri $url -OutFile $zip
    Expand-Archive -Path $zip -DestinationPath "$Work\bin" -Force
    Remove-Item $zip
}
arduino-cli version

# --------------------------------------------------------- Digistump source
Say "cloning DigistumpArduino"
if (-not (Test-Path "$Work\DigistumpArduino\.git")) {
    git clone --depth 1 https://github.com/digistump/DigistumpArduino.git "$Work\DigistumpArduino"
}

Say "packaging platform archive"
# NOTE: the archive MUST contain exactly one top-level directory. arduino-cli
# strips one path component; with two (DigistumpArduino/digistump-avr/...) the
# platform files land one level too deep and install reports success while
# leaving an empty hardware dir.
Push-Location "$Work\DigistumpArduino"
& tar czf "$Work\digistump-avr-1.5.4.tar.gz" digistump-avr
Pop-Location

# ------------------------------------------------------------- micronucleus
Say "installing micronucleus (prebuilt win binary)"
if (-not (Test-Path "$Work\bin\micronucleus.exe")) {
    Push-Location "$Work"
    & tar xzf "$Work\DigistumpArduino\tools\micronucleus-2.0a4-win.zip" -C "$Work\bin"
    Pop-Location
    Warn "if the binary is inside a subfolder, move micronucleus.exe into $Work\bin"
}
Get-ChildItem "$Work\bin" -Filter 'micronucleus*' | Select-Object Name

# ------------------------------------------------------- local package index
Say "generating local Boards Manager index"
$idx = @{
    packages = @(@{
        name       = 'digistump'
        maintainer = 'local'
        websiteURL = 'https://github.com/digistump/DigistumpArduino'
        email      = 'noreply@example.invalid'
        help       = @{ online = 'https://github.com/digistump/DigistumpArduino' }
        platforms  = @(@{
            name           = 'Digispark AVR Boards'
            id             = 'digistump-avr'
            version        = '1.5.4'
            architecture   = 'avr'
            category       = 'Contributed'
            help           = @{ online = 'https://github.com/digistump/DigistumpArduino' }
            url            = "http://127.0.0.1:$Port/digistump-avr-1.5.4.tar.gz"
            archiveFileName = 'digistump-avr-1.5.4.tar.gz'
            size           = "$((Get-Item "$Work\digistump-avr-1.5.4.tar.gz").Length)"
            checksum       = "SHA-256:$((Get-FileHash "$Work\digistump-avr-1.5.4.tar.gz" -Algorithm SHA256).Hash.ToLower())"
            boards         = @(@{ name = 'Digispark (Default - 16.5mhz)' })
        })
    })
}
$idx | ConvertTo-Json -Depth 8 | Set-Content "$Work\digistump_index.json" -Encoding UTF8

# arduino-cli will not install from file:// URLs, so serve over loopback HTTP.
Say "serving artifacts on 127.0.0.1:$Port"
Get-Job | Where-Object Name -eq 'dsindex' | Stop-Job -ErrorAction SilentlyContinue
Start-Process python -ArgumentList '-m','http.server',"$Port",'--bind','127.0.0.1' `
    -WorkingDirectory $Work -WindowStyle Hidden
Start-Sleep -Seconds 3
try {
    Invoke-WebRequest "http://127.0.0.1:$Port/digistump_index.json" -UseBasicParsing | Out-Null
} catch {
    Die "python must be on PATH, or use: python -m http.server $Port --bind 127.0.0.1"
}

# ------------------------------------------------------------------ install
Say "arduino:avr core (provides avr-gcc)"
arduino-cli config init --overwrite | Out-Null
arduino-cli config set directories.data "$Work\data"
arduino-cli config set directories.downloads "$Work\dl"
arduino-cli config set board_manager.additional_urls "http://127.0.0.1:$Port/digistump_index.json"
arduino-cli core update-index | Out-Null
arduino-cli core install arduino:avr

Say "digistump platform"
arduino-cli core install digistump:avr

Say "verifying"
arduino-cli core list

Write-Host "`n==> done" -ForegroundColor Cyan
Write-Host @"

Add to PATH (setx is permanent):
    setx PATH "$env:PATH;$Work\bin"

Then:
    tools\build.sh payloads/tier0/00-smoke
    tools\build.sh payloads/tier0/00-smoke --flash

If flashing fails with 'device not found' or 'Access denied', run the shell
elevated - the Micronucleus bootloader needs USB access that a non-admin token
may not have. See QUICKSTART.md.
"@ -ForegroundColor Cyan
