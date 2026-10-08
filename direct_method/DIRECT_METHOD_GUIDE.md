# Direct Method Guide: Running 16-bit Chip's Challenge on Windows 10

This directory contains the complete implementation of the **Direct Method (Compatibility Layer)** for running the original 16-bit Windows 3.1/Windows 95 binary (`CHIPS.EXE`) natively on 64-bit Windows 10.

---

## 1. Overview & Architecture

### The Problem
* **16-bit NE (New Executable) Format:** The original `chips_challenge\CHIPS.EXE` was compiled for 16-bit Windows 3.1 in 1991.
* **Absence of NTVDM in 64-bit Windows:** When 64-bit x86-64 processors operate in Long Mode, hardware Virtual 8086 mode is unavailable. Consequently, Microsoft removed the NTVDM (WOW16) subsystem in 64-bit Windows 10/11. Launching the 16-bit executable directly results in the OS error: *"This app can't run on your PC"*.

### The Solution: WineVDM (otvdm) Compatibility Layer
Instead of heavy, slow PC emulators (like DOSBox or VirtualBox with a full Windows 3.1 installation), this method uses **WineVDM (otvdm)**:
* **User-Mode API Thunking:** Intercepts 16-bit Windows API calls (`USER.EXE`, `GDI.EXE`, `MMSYSTEM.DLL`) and translates them directly to modern Win32/Win64 Windows 10 APIs in real time.
* **Desktop Integration:** The game window appears directly on the Windows 10 desktop alongside modern applications.
* **Authentic Audio:** Sound effects (WAV) and MIDI background music (`CHIP01.MID`, `CHIP02.MID`) play through Windows Multimedia (`winmm.dll`).

---

## 2. Directory Contents

| File / Folder | Description |
| :--- | :--- |
| `play_original.exe` | **Primary 64-bit launcher** embedded with the authentic Chip's Challenge icon. Double-click to play immediately. |
| `play_original.bat` | Script launcher alternative. |
| `setup_requirements.exe` | One-click setup utility to check environment and tools. |
| `setup_requirements.bat` | Automated setup script for tool verification and WineVDM deployment. |
| `chips_challenge/` | Original game directory containing `CHIPS.EXE`, `CHIPS.DAT`, `WEP4UTIL.DLL`, sound files (`*.WAV`), and music (`*.MID`). |
| `tools/otvdm/` | Standalone portable WineVDM runtime (~4 MB, no drivers or system install required). |
| `src/` | Source code for launcher and setup utilities. |
| `DIRECT_METHOD_GUIDE.md` | This technical guide. |

---

## 3. Audio & Sound Configuration

In the original Microsoft Entertainment Pack release, sound configuration is stored in `entpack.ini`.

### Sound Effect Mappings:
* **Bomb Explosion:** `pop2.wav` (Authentic popping balloon sound triggered when dirt blocks or monsters hit a bomb). Both `pop2.wav` and `hit3.wav` are linked to this sound.
* **Tool / Item Pickup:** `blip2.wav`
* **Chip Pickup:** `click3.wav`
* **Chip Socket Opening:** `chimes.wav`
* **Door Opening:** `door.wav`
* **Blocked Move / Wall Bump:** `oof3.wav`
* **Thief (Steals Boots):** `strike.wav`
* **Switch / Green Button:** `pop2.wav`
* **Water Splash:** `water2.wav`
* **Teleport:** `teleport.wav`
* **Chip Death:** `bummer.wav`
* **Level Complete:** `ditty1.wav`
* **Time Out Death:** `bell.wav`
* **Timer Tick:** `click1.wav`
* **Background Music:** `chip01.mid` and `chip02.mid`

All sound and music files are pre-configured in `tools\otvdm\WINDOWS\entpack.ini` and available in `chips_challenge\`.

---

## 4. How to Play

1. Open the `direct_method` folder.
2. Double-click **`play_original.exe`** (or **`play_original.bat`**).
3. The original 1991 Microsoft Chip's Challenge window opens instantly with full graphics, sounds, and music.

### In-Game Controls:
* **Move Chip:** Arrow Keys (Up, Down, Left, Right)
* **Restart Level:** `F2` or Game menu -> New
* **Go to Level / Password:** Game menu -> Go to Level
* **Toggle Sound:** Options menu -> Sound / Music
