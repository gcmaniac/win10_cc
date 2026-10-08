@echo off
setlocal enabledelayedexpansion

echo ====================================================================
echo    Chip's Challenge - Direct Method (Compatibility Layer)
echo    Setup Requirements, Tools ^& WineVDM Environment
echo ====================================================================
echo.

cd /d "%~dp0"

:: 1. Verify Original 16-bit Game Files
echo [STEP 1/3] Verifying original Chip's Challenge (16-bit) files...
if not exist "chips_challenge\CHIPS.EXE" (
    echo [ERROR] chips_challenge\CHIPS.EXE not found!
    pause
    exit /b 1
)
if not exist "chips_challenge\CHIPS.DAT" (
    echo [ERROR] chips_challenge\CHIPS.DAT not found!
    pause
    exit /b 1
)
echo [OK] Original CHIPS.EXE and CHIPS.DAT found and verified.

:: 2. Setup WineVDM (otvdm)
echo.
echo [STEP 2/3] Checking Win16 Emulation Tool (WineVDM / otvdm)...
if not exist "tools\otvdm\otvdmw.exe" (
    echo [INFO] Downloading WineVDM portable v0.9.0 via PowerShell...
    powershell -NoProfile -Command "try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $url = 'https://github.com/otya128/winevdm/releases/download/v0.9.0/otvdm-v0.9.0.zip'; $zip = 'tools\otvdm.zip'; New-Item -ItemType Directory -Force -Path 'tools\otvdm' | Out-Null; Write-Host 'Downloading WineVDM...'; (New-Object Net.WebClient).DownloadFile($url, $zip); Write-Host 'Extracting...'; Expand-Archive -Path $zip -DestinationPath 'tools\otvdm' -Force; Remove-Item $zip; $sub = 'tools\otvdm\otvdm-v0.9.0'; if (Test-Path $sub) { Get-ChildItem $sub | Move-Item -Destination 'tools\otvdm' -Force; Remove-Item $sub }; Write-Host 'WineVDM successfully installed!' } catch { Write-Host '[ERROR] Failed to download WineVDM: ' $_.Exception.Message }"
    if not exist "tools\otvdm\otvdmw.exe" (
        echo [ERROR] WineVDM installation failed. Please check your internet connection.
        pause
        exit /b 1
    )
) else (
    echo [OK] WineVDM is already installed in tools\otvdm\.
)

:: 3. Configure Sound & Settings
echo.
echo [STEP 3/3] Synchronizing audio files and configuration...
powershell -NoProfile -Command "Copy-Item 'chips_challenge\POP2.WAV' 'chips_challenge\HIT3.WAV' -Force -ErrorAction SilentlyContinue; Copy-Item 'chips_challenge\CLICK3.WAV' 'chips_challenge\CLICK1.WAV' -Force -ErrorAction SilentlyContinue; if (Test-Path 'C:\Windows\Media\chimes.wav') { Copy-Item 'C:\Windows\Media\chimes.wav' 'chips_challenge\CHIMES.WAV' -Force -ErrorAction SilentlyContinue }; if (Test-Path 'C:\Windows\Media\ding.wav') { Copy-Item 'C:\Windows\Media\ding.wav' 'chips_challenge\BELL.WAV' -Force -ErrorAction SilentlyContinue }; Get-ChildItem 'chips_challenge\*.WAV' | Copy-Item -Destination 'tools\otvdm\WINDOWS\' -Force -ErrorAction SilentlyContinue; Get-ChildItem 'chips_challenge\*.MID' | Copy-Item -Destination 'tools\otvdm\WINDOWS\' -Force -ErrorAction SilentlyContinue"

where gcc >nul 2>&1
if %errorlevel% neq 0 (
    if exist "E:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=E:\msys64\mingw64\bin;!PATH!"
    ) else if exist "C:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=C:\msys64\mingw64\bin;!PATH!"
    )
)

where gcc >nul 2>&1
if %errorlevel% equ 0 (
    if exist "src\launcher.c" (
        windres src\resources.rc -O coff -o src\resources.o 2>nul
        if exist "src\resources.o" (
            gcc -O2 -mwindows src\launcher.c src\resources.o -o play_original.exe 2>nul
            gcc -O2 src\setup_installer.c -o setup_requirements.exe 2>nul
        )
    )
)

echo [OK] Setup completed successfully.
echo.
echo ====================================================================
echo    DIRECT METHOD SETUP COMPLETE!
echo ====================================================================
echo  You can now play the original 16-bit game:
echo    - Double-click: play_original.exe  (GUI Launcher with Icon)
echo    - OR double-click: play_original.bat  (Batch Script)
echo ====================================================================
echo.
pause
