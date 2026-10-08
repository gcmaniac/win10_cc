@echo off
setlocal
cd /d "%~dp0"
if not exist "chips_win10.exe" (
    echo [ERROR] Game native belum terkompilasi!
    echo Silakan jalankan 'setup_requirements.bat' terlebih dahulu.
    pause
    exit /b 1
)
start "" "%~dp0chips_win10.exe"
