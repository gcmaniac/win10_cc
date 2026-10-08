# Troubleshooting & Engineering Journey: Chip's Challenge on Windows 10

This document records all architectural challenges, bugs, root-cause analyses, and resolutions encountered while restoring the 1991 16-bit classic **Chip's Challenge** for modern 64-bit Windows 10/11.

---

## Table of Contents
1. [Issue 1: 16-bit Win16 Binary Incompatibility (NTVDM Absence)](#issue-1-16-bit-win16-binary-incompatibility-ntvdm-absence)
2. [Issue 2: Dirt Block Submersion & Floor Transformation in Water](#issue-2-dirt-block-submersion--floor-transformation-in-water)
3. [Issue 3: Missing Password & Level Jump Interface](#issue-3-missing-password--level-jump-interface)
4. [Issue 4: Application Icon Extraction & Embedding](#issue-4-application-icon-extraction--embedding)
5. [Issue 5: Immobile Monsters & Authentic AI Ruleset Implementation](#issue-5-immobile-monsters--authentic-ai-ruleset-implementation)
6. [Issue 6: Missing Bomb Detonation Sound (Balloon Pop) in Level 9](#issue-6-missing-bomb-detonation-sound-balloon-pop-in-level-9)
7. [Issue 7: Architectural Separation & English Localization](#issue-7-architectural-separation--english-localization)
8. [Summary of Test Levels & Open Challenges for GitHub Community](#summary-of-test-levels--open-challenges-for-github-community)

---

### Issue 1: 16-bit Win16 Binary Incompatibility (NTVDM Absence)

* **Symptom:**  
  Attempting to run `CHIPS.EXE` directly on 64-bit Windows 10 fails with the operating system error:  
  `"This app can't run on your PC. To find a version for your PC, check with the software publisher."`
* **Root Cause Analysis:**  
  The original `CHIPS.EXE` header begins with `NE` (New Executable, 16-bit Windows 3.1). In 64-bit Long Mode on x86-64 CPUs, the hardware Virtual 8086 mode is unavailable. Microsoft removed the NTVDM (WOW16) subsystem from 64-bit versions of Windows.
* **Resolution:**  
  Implemented two independent pathways:
  1. **Direct Method (`direct_method/`):** Utilized WineVDM (`otvdm`), an open-source user-mode thunking layer that translates Win16 API calls (`USER.EXE`, `GDI.EXE`, `MMSYSTEM.DLL`) into modern Win32/Win64 calls in real time.
  2. **Redesign Method (`redesign_method/`):** Engineered a native 64-bit C99 engine port using Win32 GDI and WinMM, running natively without any emulation layer.

---

### Issue 2: Dirt Block Submersion & Floor Transformation in Water

* **Symptom:**  
  In Level 2 ("LESSON 2"), pushing dirt blocks into water caused the blocks to freeze or resist movement instead of bridging across the water.
* **Root Cause Analysis:**  
  The tile collision check treated water tiles containing blocks ambiguously, failing to simultaneously update both the top tile layer and bottom floor layer according to Microsoft ruleset specifications.
* **Resolution:**  
  Updated block collision logic in `game_logic.c`:
  - When a block enters a water tile, it sinks: the water tile is consumed and replaced with dirt (`FC_DIRT`).
  - When Chip steps onto the newly created dirt tile, it behaves like dirt and converts permanently to a walk-through floor tile (`FC_EMPTY`).

---

### Issue 3: Missing Password & Level Jump Interface

* **Symptom:**  
  Players could not enter level passwords to skip levels or jump directly to a target level among the 149 available levels.
* **Root Cause Analysis:**  
  The initial native C prototype only supported linear level progression (`game_next_level` and `game_prev_level`).
* **Resolution:**  
  1. Added a Win32 Menu Bar (`&Game`, `&Help`) with shortcuts:
     - `Restart Level` (`F2` / `R`)
     - `Go to Level / Password...` (`Ctrl+G` / `F3`)
  2. Built a Win32 modal dialog (`PasswordDlgProc`) allowing input of either:
     - 4-letter uppercase level passwords (e.g., `BDHP`, `JXMJ`, `KCRE`).
     - Numeric level index (`1` to `149`).
  3. Integrated password lookup in `game_goto_password()`.

---

### Issue 4: Application Icon Extraction & Embedding

* **Symptom:**  
  Both the native game executable and launcher showed generic default Windows executable icons.
* **Root Cause Analysis:**  
  16-bit NE executable resources cannot be linked directly into modern PE 64-bit binaries without conversion.
* **Resolution:**  
  1. Wrote an extraction script to retrieve original icon bitmaps from `CHIPS.EXE`.
  2. Formatted the bitmap into a standard Windows `.ico` container (`assets/chips.ico`).
  3. Created `resources.rc` and compiled it using GNU `windres` into a COFF object (`resources.o`).
  4. Linked `resources.o` during compilation into `chips_win10.exe`, `play_original.exe`, and setup binaries.

---

### Issue 5: Immobile Monsters & Authentic AI Ruleset Implementation

* **Symptom:**  
  In Level 2 and subsequent levels, creatures (Bugs, Fireballs, etc.) remained stationary on the grid.
* **Root Cause Analysis:**  
  1. The initial parser ignored **Field 10** in `CHIPS.DAT`, which dictates the official initial turn order for active creatures.
  2. The main loop lacked a periodic creature movement timer.
* **Resolution:**  
  1. **Field 10 Parsing:** Updated `dat_loader.c` to parse creature coordinates from Field 10, then scanned the grid for remaining creatures.
  2. **Movement Timer:** Added `TIMER_CREATURE` running at 250ms (4 Hz), reflecting authentic MSCC game speed.
  3. **Monster AI Implementations:**
     - **Bug:** Left-edge follower (`Left -> Straight -> Right -> Reverse`).
     - **Paramecium:** Right-edge follower (`Right -> Straight -> Left -> Reverse`).
     - **Fireball:** `Straight -> Right -> Left -> Reverse` (Immune to fire).
     - **Glider:** `Straight -> Left -> Right -> Reverse` (Navigates over water).
     - **Pink Ball:** `Straight -> Reverse` (Bounces off walls).
     - **Tank:** Moves straight until blocked; reverses direction when blue buttons are triggered.
     - **Teeth:** Actively tracks Chip's coordinates; moves every 2 creature ticks.
     - **Walker:** Straight; if blocked, randomly picks an open direction.
     - **Blob:** Random direction selector; moves every 2 creature ticks.
  4. **Hazard Handling:** Monsters properly drown in water (except Glider), burn in fire (except Fireball), detonate bombs, and press floor buttons.

---

### Issue 6: Missing Bomb Detonation Sound (Balloon Pop) in Level 9

* **Symptom:**  
  In Level 9 ("KCRE"), pushing dirt blocks into bombs destroyed the bomb, but no sound effect was heard.
* **Root Cause Analysis:**  
  In the 16-bit Microsoft Entertainment Pack version, `entpack.ini` mapped:
  ```ini
  BombSound=hit3.wav
  ```
  However, `hit3.wav` was missing from the game directory. The authentic sound associated with bomb detonations is `POP2.WAV` (a distinct popping balloon sound).
* **Resolution:**  
  1. Updated `direct_method/tools/otvdm/WINDOWS/entpack.ini` to explicitly set `BombSound=pop2.wav`.
  2. Generated a copy of `POP2.WAV` as `HIT3.WAV` in `chips_challenge/` and `tools/otvdm/WINDOWS/` for backward compatibility.
  3. Restored missing sound effects:
     - `SocketSound` & `SoundOnSound` (`CHIMES.WAV` - chime sound when socket opens).
     - `ChipDeathByTimeSound` (`BELL.WAV` - bell sound when time expires).
     - `TickSound` (`CLICK1.WAV` - timer tick sound).
  4. Updated `redesign_method/src/audio.c` to bind `SFX_BOMB` to `POP2.WAV`.

---

### Issue 7: Architectural Separation & English Localization

* **Symptom:**  
  The repository initially merged the emulation tool and the native port in a single directory with Indonesian filenames and scripts, making modular usage and international collaboration difficult.
* **Resolution:**  
  1. Reorganized into two dedicated directories:
     - `direct_method/`: Self-contained compatibility layer with pre-compiled 64-bit launcher and original assets.
     - `redesign_method/`: Native 64-bit C99 engine port with modular source code, build scripts, and tests.
  2. Provided dedicated `setup_requirements.bat` and `setup_requirements.exe` inside each directory.
  3. Localized all scripts, UI dialogs, compiler messages, guides, and documentation into standard English.

---

## Summary of Test Levels & Open Challenges for GitHub Community

The game includes all **149 original levels** stored in `CHIPS.DAT`. Key milestone levels tested during this engineering process include:

| Level # | Password | Title | Verified Mechanics |
| :---: | :---: | :--- | :--- |
| **1** | `BDHP` | LESSON 1 | Movement, key pickups, doors, chip collection, socket opening. |
| **2** | `JXMJ` | LESSON 2 | Bug movement (left-edge AI), dirt block water pushing, water bridging. |
| **3** | `ECBQ` | LESSON 3 | Fire boots, fire tiles, glider interactions. |
| **4** | `YMCJ` | LESSON 4 | Ice skates, ice corner slides, inertia momentum. |
| **5** | `TQKB` | LESSON 5 | Suction boots, conveyor belts (force floors), direction logic. |
| **8** | `NHAG` | LESSON 8 | Thief encounters, stripping player inventory boots. |
| **9** | `KCRE` | NUTS AND BOLTS | Bomb detonation via dirt block, balloon popping sound effect, maze navigation. |

### Open Challenge for Contributors:
- Test higher levels (Levels 10 through 149) to discover and submit patches for rare edge-case MSCC rules (e.g., precise clone machine timing, teleport priority loops, and speedrun glitch parity).
- Enjoy preserving computing history!
