@echo off
setlocal
cd /d "%~dp0"

if not exist "tools\otvdm\otvdmw.exe" (
    echo [ERROR] WineVDM is not installed!
    echo Please run 'setup_requirements.bat' or 'setup_requirements.exe' first.
    pause
    exit /b 1
)

start "" /d "%~dp0chips_challenge" "%~dp0tools\otvdm\otvdmw.exe" "%~dp0chips_challenge\CHIPS.EXE"
