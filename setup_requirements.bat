@echo off
setlocal enabledelayedexpansion

echo ====================================================================
echo    Chip's Challenge - Setup Tools, Requirements ^& Build System
echo ====================================================================
echo.

cd /d "%~dp0"

:: 1. Check MinGW GCC
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    if exist "E:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=E:\msys64\mingw64\bin;!PATH!"
        echo [INFO] MinGW GCC found at E:\msys64\mingw64\bin and added to PATH.
    ) else if exist "C:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=C:\msys64\mingw64\bin;!PATH!"
        echo [INFO] MinGW GCC found at C:\msys64\mingw64\bin and added to PATH.
    ) else (
        echo [WARN] GCC not found in PATH. Install MinGW-w64 to recompile C code.
    )
) else (
    echo [OK] MinGW GCC is ready.
)

:: 2. Check Python
where python >nul 2>&1
if %errorlevel% neq 0 (
    if exist "C:\Python314\python.exe" (
        set "PATH=C:\Python314;!PATH!"
        echo [INFO] Python found at C:\Python314 and added to PATH.
    ) else (
        echo [WARN] Python not detected in PATH.
    )
) else (
    echo [OK] Python is ready.
)

:: 3. Setup WineVDM (otvdm)
if not exist "direct_method\tools\otvdm\otvdmw.exe" (
    echo.
    echo [STEP 1/3] Preparing WineVDM (otvdm) for 16-bit emulation...
    call "direct_method\setup_requirements.bat"
) else (
    echo [OK] WineVDM is already installed in direct_method\tools\otvdm.
)

:: 4. Extract Assets
echo.
echo [STEP 2/3] Extracting and preparing game assets...
if exist "scripts\extract_assets.py" (
    python scripts\extract_assets.py
)

:: 5. Compile C Engine and Launchers
echo.
echo [STEP 3/3] Compiling Native C Engine and Launchers...
where gcc >nul 2>&1
if %errorlevel% equ 0 (
    call build.bat
) else (
    echo [SKIP] Compilation skipped: GCC not found in PATH.
)

echo.
echo ====================================================================
echo    SETUP COMPLETE! READY TO PLAY ON WINDOWS 10
echo ====================================================================
echo  You have two ways to play:
echo.
echo  1. Direct Method - Original 16-bit Version (Authentic 1991):
echo     - Double-click: play_original.bat  or  direct_method\play_original.exe
echo.
echo  2. Redesign Method - Native 64-bit Version (Modern C Port):
echo     - Double-click: play_chips.bat     or  redesign_method\chips_win10.exe
echo ====================================================================
echo.
pause
