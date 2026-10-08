@echo off
setlocal
cd /d "%~dp0"
if exist "redesign_method\chips_win10.exe" (
    start "" "redesign_method\chips_win10.exe"
) else if exist "chips_win10.exe" (
    start "" "chips_win10.exe"
) else (
    echo [ERROR] Game executable not found. Please run build.bat first.
    pause
)
