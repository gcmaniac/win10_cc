# Redesign Method Guide: Native 64-bit C Engine Port

This directory contains the complete implementation of the **Redesign Method (Native 64-bit C Engine Port)** for Chip's Challenge on Windows 10 without any emulation layer.

---

## 1. Architecture & Design

* **Native 64-bit Architecture:**
  - Written in portable C99 using native Win32 GDI and WinMM APIs.
  - Zero external dependencies: no third-party DLLs, no emulators, zero installation overhead.
  - High performance, sub-millisecond input response, smooth double-buffered rendering.
* **Asset Integration:**
  - `scripts/extract_assets.py` extracts original sprite bitmaps from the 16-bit `CHIPS.EXE` NE resource header into standard BMP tiles.
  - `dat_loader.c` parses the official binary level format `CHIPS.DAT` (149 levels, monster spawn tables, 32x32 maps, passwords, hints).
  - Audio engine (`audio.c`) plays WAV sound effects and MIDI soundtrack (`CHIP01.MID`, `CHIP02.MID`) directly via the Windows Multimedia sequencer.
* **Authentic Microsoft Ruleset (MSCC):**
  - Full movement logic for all 9 creature types (Bug, Fireball, Pink Ball, Tank, Glider, Teeth, Walker, Blob, Paramecium) running on a authentic 250ms (4 Hz) movement timer.
  - Authentic tile interactions: Dirt block pushing, water filling, bomb detonation (popping sound), ice sliding, teleports, trap buttons, and burglar encounters.
  - Password and level jump dialog (`Ctrl+G` / `F3`).

---

## 2. Directory Contents

| File / Folder | Description |
| :--- | :--- |
| `chips_win10.exe` | **Primary 64-bit native game executable** with embedded Chip's Challenge icon. |
| `play_chips.bat` | Batch script launcher shortcut. |
| `build.bat` | Fast build script using MinGW GCC. |
| `Makefile` | Standard GNU Makefile for compilation. |
| `setup_requirements.exe` | One-click toolchain and asset preparation executable. |
| `setup_requirements.bat` | Automated setup script for toolchain checks and compilation. |
| `include/` | Header files (`chips_types.h`, `game_logic.h`, `dat_loader.h`, `renderer.h`, `audio.h`). |
| `src/` | Source code files. |
| `assets/` | Extracted sprites, audio WAV/MID, and level data `CHIPS.DAT`. |
| `scripts/` | Python extraction scripts. |
| `tests/` | Unit tests for game logic and creature movement. |
| `REDESIGN_METHOD_GUIDE.md` | This technical guide. |

---

## 3. How to Build & Run

### Running Directly:
1. Double-click **`chips_win10.exe`** or run **`play_chips.bat`**.

### Rebuilding from Source:
1. Run `build.bat` or `make`.
2. The compiler produces `chips_win10.exe` linked with standard Windows libraries (`-lgdi32 -lmsimg32 -lwinmm`).

### Game Controls:
* **Movement:** Arrow Keys or `W`, `A`, `S`, `D`
* **Go to Level / Password Dialog:** `Ctrl+G` or `F3`
* **Restart Level:** `F2` or `R`
* **Toggle Sound & Music:** `M`
* **Show Hint:** `H`
* **Advance After Victory:** `Space` or `Enter`
