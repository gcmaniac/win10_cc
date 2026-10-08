# Project Plan: Running Chip's Challenge (Win3.1/Win95) on Windows 10
**Repository:** `e:\c_projects\win10_cc`  
**Target:** Run 16-bit Chip's Challenge smoothly, stably, and authentically on modern 64-bit Windows 10/11.

---

## 1. Problem Analysis (Root Cause)

1. **16-bit Binary Format (NE - New Executable):**  
   The binary header of `CHIPS.EXE` at offset `0x400` identifies it as an `NE` (16-bit Windows 3.x Executable), not a `PE` (Portable Executable 32/64-bit).
2. **Absence of NTVDM in 64-bit Windows:**  
   64-bit Windows 10 does not support 16-bit instructions directly because x86-64 processors in Long Mode cannot execute Virtual 8086 mode. Attempting to run it natively causes the OS error: *"This app can't run on your PC"*.
3. **Asset Inventory:**
   - `CHIPS.DAT`: Binary database of 149 levels (32x32 maps, passwords, time limits, hints).
   - `CHIPS.EXE`: Contains Win16 game logic and 7 bitmap resources (tileset, Chip, monsters, UI).
   - Audio files: Sound effects (`*.WAV`) and background MIDI soundtracks (`CHIP01.MID`, `CHIP02.MID`).
   - `WEP4UTIL.DLL`: Microsoft Entertainment Pack 4 support library.

---

## 2. Implemented Approaches

This repository implements two distinct and complete methods, each separated into its own self-contained directory:

| Aspect | Direct Method (`direct_method/`) | Redesign Method (`redesign_method/`) |
| :--- | :--- | :--- |
| **Concept** | Compatibility layer (WineVDM/otvdm) running the original 16-bit binary | Modern native 64-bit C engine port reimplemented from scratch |
| **Runtime** | Lightweight user-mode thunking layer (~4 MB) | Native 64-bit PE executable with zero external dependencies |
| **Authenticity** | 100% byte-for-byte original 1991 Microsoft executable | Authentic Microsoft ruleset (MSCC) behavior in native C99 |
| **Customization** | Configuration via `entpack.ini` | Fully editable C source code (`src/`, `include/`) |

---

## 3. Directory Layout

- **[`direct_method/`](file:///e:/c_projects/win10_cc/direct_method):** Contains WineVDM compatibility layer, original 16-bit files, launchers (`play_original.exe`, `play_original.bat`), and setup utilities.
- **[`redesign_method/`](file:///e:/c_projects/win10_cc/redesign_method):** Contains complete C source code, asset extractors, build scripts (`build.bat`, `Makefile`), and native 64-bit binary (`chips_win10.exe`).

---

## 4. Verification & Testing

Both methods have been verified on Windows 10:
- Level progression from Level 1 ("LESSON 1") through Level 9 ("KCRE") and beyond.
- Full monster AI movement running on 250ms (4 Hz) movement ticks across all levels.
- Full audio support including balloon pop bomb explosion sound (`pop2.wav`), sockets (`chimes.wav`), tools, doors, and MIDI playback.
