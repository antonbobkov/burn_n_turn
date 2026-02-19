---
name: claude encapsulate DragonGameController
overview: Make all public member variables of DragonGameController private and expose narrow accessor methods, following the principles in public_variable_burndown.md.
todos:
  - id: header
    content: "Update dragon_game_controller.h: move all variables to private, add ~30 accessor method declarations"
    status: pending
  - id: impl
    content: Implement new accessor methods in dragon_game_controller.cc
    status: pending
  - id: runner
    content: Update dragon_game_runner.cc and simulation.cc to use new API
    status: pending
  - id: level
    content: Update level_controller.cc to use new API
    status: pending
  - id: menu
    content: Update menu_controller.cc to use new API
    status: pending
  - id: basic
    content: Update basic_controllers.cc and buy_now_controller.cc to use new API
    status: pending
  - id: entities
    content: Update critters.cc, critter_generators.cc, dragon.cc, fireball.cc, entities.cc to use new API
    status: pending
  - id: build-test
    content: Build project and run simulation_test to verify correctness
    status: pending
isProject: false
---

# Claude: Encapsulate DragonGameController Public Variables

## Goal

Move all 25 public data members of `DragonGameController` to private and add the minimum accessor API needed. Follows the principles in [public_variable_burndown.md](docs/public_variable_burndown.md): no public variables, narrow access functions, read-only getters where possible.

## Changes to [dragon_game_controller.h](code/game/controller/dragon_game_controller.h)

Move **all** member variables below `private:`. Add these accessor methods:

### Controller stack (`vCnt`, `nActive`)

- `GameController* GetActiveController()` -- returns `vCnt[nActive].get()` (runner delegates input/update)
- `unsigned GetActiveIndex() const` -- returns `nActive` (runner, simulation)
- `unsigned GetControllerCount() const` -- returns `vCnt.size()` (runner)
- `void SetActiveIndex(unsigned i)` -- sets `nActive = i` (menu resume)
- `void GoToPreLastScreen()` -- sets `nActive = vCnt.size() - 2` (level game-over)

### Level pointers (`vLevelPointers`)

- `int GetLevelPointer(int index) const` -- returns `vLevelPointers.at(index)` (menu "Level 1/2/3" -> Restart)

### Drawing resources (`pGraph`, `pDr`, `pNum`, `pBigNum`, `pFancyNum`)

- `GraphicalInterface<Index>* GetGraph()` -- used for RefreshAll, DrawRectangle, DrawCursor
- `const smart_pointer<ScalingDrawer>& GetDr()` -- Draw() takes `smart_pointer<ScalingDrawer>`; also `->nFactor` reads
- `const smart_pointer<NumberDrawer>& GetNum()` -- DrawWord, DrawNumber, passed to TutorialTextEntity ctor
- `const smart_pointer<NumberDrawer>& GetBigNum()` -- DrawWord, DrawNumber for big text
- `FontWriter* GetFancyNum()` -- GetSize, DrawWord for intro text

### Sound (`pSnd`)

- `void PlaySound(const std::string& key)` -- combines `pSnd->PlaySound(GetSnd(key))`; covers ~30 call sites
- `void ToggleSound()` -- `pSnd->Toggle(); sbSoundOn.Set(pSnd->Get());` (menu)
- `bool IsSoundOn() const` -- `sbSoundOn.Get()` (menu label)

### Score (`nScore`, `nHighScore`)

- `int GetScore() const`
- `void AddScore(int delta)` -- `nScore += delta` (entities awarding points)
- `void ResetScore()` -- `nScore = 0` (used in StartUp; also exposed for completeness)
- `int GetHighScore() const`
- `void SetHighScore(int hs)` -- score screen writes when record beaten

### Angry flag (`bAngry`)

- `bool IsAngry() const`
- `void MakeAngry()` -- only ever set to true; narrower than a generic setter

### Savable settings

- `int GetProgress() const` -- `snProgress.Get()` (menu)
- `bool IsMusicOn() const` -- `sbMusicOn.Get()` (menu label, level)
- `void ToggleMusic()` -- `plr.ToggleOff(); sbMusicOn.Set(!plr.bOff);` (menu)
- `bool IsTutorialOn() const` -- `sbTutorialOn.Get()` (menu label)
- `void ToggleTutorial()` -- `BoolToggle(sbTutorialOn)` (menu)
- `const bool* GetTutorialOnPointer() const` -- `sbTutorialOn.GetConstPointer()` (passed to TutorialTextEntity for real-time check)
- `bool IsFullScreen() const` -- (menu label)
- `void ToggleFullScreen()` -- `BoolToggle(sbFullScreen)` (menu)
- `bool AreCheatsOn() const` -- (menu label, level, basic controllers)
- `void ToggleCheats()` -- `BoolToggle(sbCheatsOn)` (menu)

### Bounds (`rBound`)

- `const Rectangle& GetBound() const` -- level UI placement; note GameController also has its own `rBound` copy, callers could use that instead where possible

### Bonuses (`lsBonusesToCarryOver`)

- `void ClearBonuses()` -- level start, dragon reset
- `void AddBonusToCarryOver(smart_pointer<TimedFireballBonus> b)` -- dragon end-of-level
- `const std::list<smart_pointer<TimedFireballBonus>>& GetBonuses() const` -- dragon iterates on new level

### Wrapper (`pWrp`)

- `void ExitProgram()` -- `Trigger(pWrp->pExitProgram)` (menu)
- `Size GetActualResolution() const` -- `pWrp->szActualRez` (level scaling)
- `FilePath* GetSaveFilePath() const` -- `pWrp->GetFilePath()` (score screen saves high score)

### No new API needed (only internal access)

These just move to `private:` with no external accessor:

- `pMenu`, `pSndRaw`, `vLvl`, `sbCheatsUnlocked`, `pSelf`, `plr` (music player -- exposed via `ToggleMusic`), `sbSoundOn`, `sbMusicOn`, `sbTutorialOn`, `sbFullScreen`, `sbCheatsOn`, `snProgress`

## Files to update (call sites)

Each file replaces direct member access with the new methods:


| File                                    | Members accessed                                                                                                                                               | Pattern change                                                                                                                                                       |
| --------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `game/dragon_game_runner.cc`            | `vCnt`, `nActive`                                                                                                                                              | `pCnt->vCnt[pCnt->nActive]->X()` -> `pCnt->GetActiveController()->X()`; `twr->nActive` -> `twr->GetActiveIndex()`; `twr->vCnt.size()` -> `twr->GetControllerCount()` |
| `simulation/simulation.cc`              | `nScore`, `nHighScore`                                                                                                                                         | `twr->nScore` -> `twr->GetScore()`; `twr->nHighScore` -> `twr->GetHighScore()`                                                                                       |
| `game/controller/level_controller.cc`   | `pNum`, `pBigNum`, `pGraph`, `pSnd`+`GetSnd`, `nScore`, `rBound`, `pWrp`, `vCnt`+`nActive`, `sbMusicOn`, `sbCheatsOn`, `sbTutorialOn`, `lsBonusesToCarryOver`  | Getters and forwarding methods                                                                                                                                       |
| `game/controller/menu_controller.cc`    | `pDr`, `pGraph`, `pSnd`+`GetSnd`, `nActive`, `vLevelPointers`, `plr`, `sbMusicOn`/`sbSoundOn`/`sbTutorialOn`/`sbFullScreen`/`sbCheatsOn`, `snProgress`, `pWrp` | Toggle/Is methods, `PlaySound()`, `SetActiveIndex()`, `GetLevelPointer()`, `ExitProgram()`                                                                           |
| `game/controller/basic_controllers.cc`  | `pDr`, `pGraph`, `pSnd`+`GetSnd`, `nScore`, `nHighScore`, `sbCheatsOn`, `pWrp`                                                                                 | Getters and forwarding methods                                                                                                                                       |
| `game/controller/buy_now_controller.cc` | `pDr`, `pGraph`                                                                                                                                                | `GetDr()`, `GetGraph()`                                                                                                                                              |
| `game/critters.cc`                      | `pSnd`+`GetSnd`, `pNum`, `bAngry`                                                                                                                              | `PlaySound()`, `GetNum()`, `MakeAngry()`                                                                                                                             |
| `game/critter_generators.cc`            | `pSnd`+`GetSnd`, `nHighScore`, `bAngry`                                                                                                                        | `PlaySound()`, `GetHighScore()`, `IsAngry()`                                                                                                                         |
| `game/dragon.cc`                        | `pSnd`+`GetSnd`, `lsBonusesToCarryOver`                                                                                                                        | `PlaySound()`, `AddBonusToCarryOver()`, `ClearBonuses()`, `GetBonuses()`                                                                                             |
| `game/fireball.cc`                      | `pSnd`+`GetSnd`, `pNum`                                                                                                                                        | `PlaySound()`, `GetNum()`                                                                                                                                            |
| `game/entities.cc`                      | `pBigNum`, `pFancyNum`, `pNum`, `nScore`, `nHighScore`                                                                                                         | Getters, `AddScore()`                                                                                                                                                |
| `game/level.cc`                         | (none -- uses `GetImg()` which is already a method)                                                                                                            | No change                                                                                                                                                            |


## Build and test

After all changes: build with cmake/mingw32-make, then run `simulation_test.exe`.