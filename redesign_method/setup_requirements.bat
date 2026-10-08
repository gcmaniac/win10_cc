@echo off
setlocal enabledelayedexpansion

echo ====================================================================
echo    Chip's Challenge - Redesign Method (Native 64-bit C Engine)
echo    Setup Requirements, Toolchain ^& Asset Processing
echo ====================================================================
echo.

cd /d "%~dp0"

:: 1. Check MinGW GCC
echo [STEP 1/4] Checking C Compiler (MinGW-w64 GCC)...
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    if exist "E:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=E:\msys64\mingw64\bin;!PATH!"
        echo [INFO] MinGW GCC found at E:\msys64\mingw64\bin.
    ) else if exist "C:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=C:\msys64\mingw64\bin;!PATH!"
        echo [INFO] MinGW GCC found at C:\msys64\mingw64\bin.
    ) else (
        echo [WARN] GCC not found in PATH. Required for compiling C code.
    )
) else (
    echo [OK] MinGW GCC is ready.
)

:: 2. Check Python
echo.
echo [STEP 2/4] Checking Python Interpreter (for Asset Extraction)...
where python >nul 2>&1
if %errorlevel% neq 0 (
    if exist "C:\Python314\python.exe" (
        set "PATH=C:\Python314;!PATH!"
        echo [INFO] Python found at C:\Python314.
    ) else (
        echo [WARN] Python not detected in PATH.
    )
) else (
    echo [OK] Python is ready.
)

:: 3. Extract Assets (Sprites, Audio, Level Data)
echo.
echo [STEP 3/4] Extracting and preparing game assets...
if exist "scripts\extract_assets.py" (
    where python >nul 2>&1
    if %errorlevel% equ 0 (
        python scripts\extract_assets.py
    ) else (
        echo [SKIP] Asset extraction skipped: Python unavailable.
    )
) else (
    echo [WARN] scripts\extract_assets.py not found.
)

:: 4. Compile Native C Engine
echo.
echo [STEP 4/4] Compiling Native C Game Engine (chips_win10.exe)...
where gcc >nul 2>&1
if %errorlevel% equ 0 (
    echo - Compiling resource icon (windres)...
    windres src\resources.rc -O coff -o src\resources.o 2>nul
    
    echo - Compiling chips_win10.exe...
    if exist "src\resources.o" (
        gcc -O2 -mwindows -Iinclude src\main.c src\renderer.c src\dat_loader.c src\game_logic.c src\audio.c src\resources.o -lgdi32 -lmsimg32 -lwinmm -o chips_win10.exe
    ) else (
        gcc -O2 -mwindows -Iinclude src\main.c src\renderer.c src\dat_loader.c src\game_logic.c src\audio.c -lgdi32 -lmsimg32 -lwinmm -o chips_win10.exe
    )

    if %errorlevel% equ 0 (
        echo   [OK] chips_win10.exe successfully created!
    ) else (
        echo   [FAIL] Compilation of chips_win10.exe failed.
    )

    gcc -O2 src\setup_installer.c -o setup_requirements.exe 2>nul
) else (
    if exist "chips_win10.exe" (
        echo [OK] Using pre-compiled chips_win10.exe.
    ) else (
        echo [WARN] Compilation skipped: GCC unavailable.
    )
)

echo.
echo ====================================================================
echo    REDESIGN METHOD SETUP COMPLETE!
echo ====================================================================
echo  You can now launch the native 64-bit engine:
echo    - Double-click: chips_win10.exe  (Native Application with Icon)
echo    - OR double-click: play_chips.bat
echo ====================================================================
echo.
pause
