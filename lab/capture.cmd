@echo off
rem capture.cmd - pull a test run off a Windows target.
rem
rem Run this on the TARGET after a recon payload. Copies C:\Users\Public\ds-recon
rem next to a timestamped folder, so two runs can be diffed.
rem
rem Reads and copies only. Does not modify the target.

setlocal enabledelayedexpansion

set "SRC=%USERPROFILE%\Public\ds-recon"
if "%~1"=="" (
  set "DEST=%CD%\runs"
) else (
  set "DEST=%~1"
)

rem wmic is deprecated but still the only timestamp source guaranteed present
for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMddTHHmmssZ"') do set "STAMP=%%i"
if "%STAMP%"=="" set "STAMP=%RANDOM%"

set "OUT=%DEST%\%STAMP%"
mkdir "%OUT%" 2>nul

echo ==^> run %STAMP% -^> %OUT%

if exist "%SRC%" (
  xcopy /E /I /Q /Y "%SRC%" "%OUT%\recon" >nul
  echo ==^> copied recon output
) else (
  echo     %SRC% not found ^(no recon payload ran, or it failed^)
)

rem Enumeration state: did the host accept a foreign HID keyboard? This is the
rem difference between "the payload ran" and "the payload was allowed to run".
echo ==^> enumeration snapshot > "%OUT%\enumeration.txt"
pnputil /enum-devices /connected > "%OUT%\enumeration.txt" 2>nul
if exist "%OUT%\enumeration.txt" (
  findstr /i "VID_16C0^&PID_27DB" "%OUT%\enumeration.txt%" >nul
  if errorlevel 1 (
    echo     Digispark 16c0:27db not found in the connected-device list.
    echo     Expected if the device was already unplugged.
  ) else (
    echo ==^> Digispark enumerated as 16c0:27db
  )
)

(
  echo run       %STAMP%
  echo recon dir %SRC%
) > "%OUT%\MANIFEST.txt"

echo.
echo ==^> done. Compare with:
echo     fc /n "%OUT%\recon\*" ^<other-run^>\recon\*^>

endlocal
