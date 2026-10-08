@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

where gcc >nul 2>&1
if %errorlevel% neq 0 (
    if exist "E:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=E:\msys64\mingw64\bin;!PATH!"
    ) else if exist "C:\msys64\mingw64\bin\gcc.exe" (
        set "PATH=C:\msys64\mingw64\bin;!PATH!"
    )
)

echo [BUILD] Compiling resource icon (src\resources.rc)...
windres src\resources.rc -O coff -o src\resources.o

echo [BUILD] Compiling chips_win10.exe (with Chip's Challenge Icon)...
gcc -O2 -mwindows -Iinclude src\main.c src\renderer.c src\dat_loader.c src\game_logic.c src\audio.c src\resources.o -lgdi32 -lmsimg32 -lwinmm -o chips_win10.exe
if %errorlevel% equ 0 (
    echo [SUCCESS] chips_win10.exe built successfully!
) else (
    echo [ERROR] Failed to compile chips_win10.exe
    pause
    exit /b 1
)

echo [BUILD] Compiling play_original.exe...
gcc -O2 -mwindows src\launcher.c src\resources.o -o play_original.exe
if %errorlevel% equ 0 (
    echo [SUCCESS] play_original.exe built successfully!
)
