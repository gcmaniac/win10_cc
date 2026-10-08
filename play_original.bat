@echo off
setlocal
cd /d "%~dp0"
if not exist "direct_method\tools\otvdm\otvdmw.exe" (
    echo [ERROR] WineVDM not found!
    echo Please run 'setup_requirements.bat' first.
    pause
    exit /b 1
)
start "" /d "%~dp0direct_method\chips_challenge" "%~dp0direct_method\tools\otvdm\otvdmw.exe" "%~dp0direct_method\chips_challenge\CHIPS.EXE"
