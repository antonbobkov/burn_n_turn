# File I/O usage in game/

This document lists every place where files are read or written for game logic:
direct uses of `std::ifstream` / `std::ofstream`, and indirect uses via
`SavableVariable<T>`.

---

## 1. Direct stream usage

### 1.1 game/core.cpp

| Code location   | Stream   | File(s) read/written | Purpose |
|-----------------|----------|----------------------|---------|
| game/core.cpp:118 | `ifstream` | `sFontPath + sFontName + ".txt"` | **Read.** In `NumberDrawer` constructor: loads character-to-index mapping for the font. At call site (game/core.cpp:944–945) the path is `dragonfont\dragonfont.txt`. |
| game/core.cpp:317 | `ifstream` | `"high.txt"` | **Read.** In `TwrGlobalController` constructor: loads high score. |
| game/core.cpp:862 | `ifstream` | `sFullScreenPath` (`"fullscreen.txt"`) | **Read.** In `AreWeFullScreen()`: reads fullscreen preference for `GetProgramInfo()`. |
| game/core.cpp:922 | `ifstream` | `"config.txt"` | **Read.** In `TowerDataWrap` constructor: loads config (e.g. `FilePath fp`); required (throws if missing). |

### 1.2 game/level.cpp

| Code location     | Stream   | File(s) read/written | Purpose |
|-------------------|----------|----------------------|---------|
| game/level.cpp:318 | `ifstream` | `sFile` (argument to `ReadLevels`) | **Read.** Reads level definitions. Call sites in game/core.cpp:950, :952, :955 use: `fp.sPath + "levels_small.txt"`, `fp.sPath + "levels.txt"`, or `fp.sPath + "levels_trial.txt"`. |

### 1.3 game/screen_controllers.cpp

| Code location              | Stream   | File(s) read/written | Purpose |
|----------------------------|----------|----------------------|---------|
| game/screen_controllers.cpp:595 | `ofstream` | `"high.txt"` | **Write.** In `DragonScoreController::Update()`: saves new high score when current score exceeds it. |

### 1.4 game/main.cpp

| Code location   | Stream   | File(s) read/written | Purpose |
|-----------------|----------|----------------------|---------|
| game/main.cpp:147 | `ofstream` | `"error.txt"` | **Write.** In `catch (MyException &me)`: writes exception description to disk. |

---

## 2. Indirect file I/O: `SavableVariable<T>` (game/common.h)

The template `SavableVariable<T>` in **game/common.h** (lines 147–179) does all
of its file I/O internally:

- **Read:** game/common.h:157–162 (constructor): `std::ifstream ifs(sFileName.c_str()); ifs >> var;` (uses default if read fails).
- **Write:** game/common.h:165–168 (`Save()`): `std::ofstream ofs(sFileName.c_str()); ofs << var;`
- **When write happens:** `Set(new_var, bSave)` calls `Save()` when `bSave` is true (default). game/common.h:182 `BoolToggle(SavableVariable<bool>&)` calls `Set(!Get())`, so it also triggers a write.

### 2.1 Where `SavableVariable` instances are declared and written

All are members of `TwrGlobalController` declared in **game/game.h** (lines 66–73).
Initialized (file names) in **game/core.cpp** (TwrGlobalController ctor, lines 308–314).

| Member              | Type                    | Declared at           | File name   | Written at (code location) |
|---------------------|-------------------------|------------------------|-------------|----------------------------|
| `snProgress`        | `SavableVariable<int>`  | game/game.h:66         | `stuff.txt` | game/core.cpp:837 `snProgress.Set(i)` |
| `sbSoundOn`         | `SavableVariable<bool>` | game/game.h:68         | `soundon.txt` | game/screen_controllers.cpp:353 `sbSoundOn.Set(...)` |
| `sbMusicOn`         | `SavableVariable<bool>` | game/game.h:69         | `musicon.txt` | game/screen_controllers.cpp:347 `sbMusicOn.Set(...)` |
| `sbTutorialOn`      | `SavableVariable<bool>` | game/game.h:70         | `tutorial_on.txt` | game/screen_controllers.cpp:358 `BoolToggle(sbTutorialOn)` |
| `sbFullScreen`      | `SavableVariable<bool>` | game/game.h:71         | `fullscreen.txt` | game/screen_controllers.cpp:363 `BoolToggle(sbFullScreen)` |
| `sbCheatsOn`        | `SavableVariable<bool>` | game/game.h:72         | `cheat.txt` | game/screen_controllers.cpp:368 `BoolToggle(sbCheatsOn)` |
| `sbCheatsUnlocked`  | `SavableVariable<bool>` | game/game.h:73         | `more_stuff.txt` | No write call site found in game/ |

### 2.2 Summary of files used by SavableVariable

| File name         | Role                          |
|-------------------|-------------------------------|
| `stuff.txt`       | Progress (int)                |
| `soundon.txt`     | Sound on (bool)               |
| `musicon.txt`     | Music on (bool)               |
| `tutorial_on.txt` | Tutorial on (bool)            |
| `fullscreen.txt`  | Fullscreen (bool)             |
| `cheat.txt`       | Cheats on (bool)              |
| `more_stuff.txt`  | Cheats unlocked (bool)        |

---

## 3. Quick reference: all file names used in game/

**Read only:**  
`config.txt` (game/core.cpp:922), `dragonfont\dragonfont.txt` (game/core.cpp:118), `fullscreen.txt` (game/core.cpp:862), `high.txt` (game/core.cpp:317),  
`levels.txt` / `levels_small.txt` / `levels_trial.txt` (game/level.cpp:318; call sites game/core.cpp:950, :952, :955).

**Write only:**  
`error.txt` (game/main.cpp:147), `high.txt` (game/screen_controllers.cpp:595).

**Read and write (SavableVariable):**  
`stuff.txt`, `soundon.txt`, `musicon.txt`, `tutorial_on.txt`,  
`fullscreen.txt`, `cheat.txt`, `more_stuff.txt` — read/write in game/common.h:157, :165–168; instance locations in section 2.1.

**Note:** `high.txt` is read at game/core.cpp:317 and written at game/screen_controllers.cpp:595.  
`fullscreen.txt` is read directly in `AreWeFullScreen()` (game/core.cpp:862) and also used by  
`SavableVariable` for `sbFullScreen` (game/common.h:157, :166).
