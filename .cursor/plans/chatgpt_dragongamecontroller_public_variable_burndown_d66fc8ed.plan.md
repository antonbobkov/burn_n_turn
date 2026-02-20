---
name: ChatGPT DragonGameController public variable burndown
overview: Make all `DragonGameController` data members private and replace all external direct member access with a narrow function-based API, per `docs/public_variable_burndown.md`, updating all call sites and verifying the build + `simulation_test`.
todos:
  - id: design-api
    content: Design and add the minimal function-based public API on DragonGameController; move all data members to private; add missing standard includes in the header.
    status: pending
  - id: update-callers
    content: Update all external call sites to use the new DragonGameController APIs (runner, controllers, entities, simulation, critters, dragon, etc.).
    status: pending
  - id: verify-build-tests
    content: Build with mingw32-make, run ctest, and run simulation_test; fix any regressions.
    status: pending
isProject: false
---

# ChatGPT: DragonGameController public variable burndown

## Goal

- Remove **all public data members** from `DragonGameController` and ensure every external access goes through functions, following `[docs/public_variable_burndown.md](docs/public_variable_burndown.md)`.
- Update all call sites identified in `[docs/dragon_game_controller_public_access.md](docs/dragon_game_controller_public_access.md)` so the project still builds and `simulation_test` passes.

## Current problem (what we’re changing)

`DragonGameController` currently exposes many public fields (controllers, graphics, sound, score, options, etc.), e.g.:

```31:90:e:\code\dragongame\code\game\controller\dragon_game_controller.h
struct DragonGameController {
  std::vector<smart_pointer<GameController>> vCnt;
  unsigned nActive;

  std::vector<int> vLevelPointers;

  smart_pointer<MenuController> pMenu;

  GraphicalInterface<Index> *pGraph;
  smart_pointer<ScalingDrawer> pDr;
  smart_pointer<NumberDrawer> pNum;
  smart_pointer<NumberDrawer> pBigNum;
  FontWriter *pFancyNum;

  SoundInterface<Index> *pSndRaw;
  smart_pointer<SoundInterfaceProxy> pSnd;

  std::vector<LevelLayout> vLvl;

  int nScore;
  int nHighScore;

  bool bAngry;

  SavableVariable<int> snProgress;
  SavableVariable<bool> sbSoundOn;
  SavableVariable<bool> sbMusicOn;
  SavableVariable<bool> sbTutorialOn;
  SavableVariable<bool> sbFullScreen;
  SavableVariable<bool> sbCheatsOn;
  SavableVariable<bool> sbCheatsUnlocked;

  Rectangle rBound;

  BackgroundMusicPlayer plr;

  std::list<smart_pointer<TimedFireballBonus>> lsBonusesToCarryOver;
  // ... plus pWrp, pSelf ...
```

External code directly indexes controllers and calls through pointers, for example in the runner:

```75:139:e:\code\dragongame\code\game\dragon_game_runner.cc
unsigned DragonGameRunner::GetActiveControllerIndex() const {
  DragonGameController *twr = GetTowerController();
  return twr ? twr->nActive : 0;
}

void DragonGameRunner::Update() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->Update();
}
```

## Target public API shape (narrow, function-based)

Implement a **minimal set of accessors/forwarders** to cover all documented external uses, preferring narrow operations (e.g. `PlaySound("key")`) over broad pointer exposure, but using getters where that’s the practical shared-resource pattern (drawers/fonts).

Planned `DragonGameController` additions (in `[code/game/controller/dragon_game_controller.h](code/game/controller/dragon_game_controller.h)` / `.cc`):

- **Controller stack (no index visibility outside class)**
  - **Chosen approach**: treat the controller stack as a private state machine.
  External code never reads/writes the active index and never indexes `vCnt`.
  - **Runner dispatch**:
    - `GameController *GetActiveController() const`
    - Runner calls `GetActiveController()->Update()` and forwards input to that
    controller (no `nActive`, no `vCnt` access).
  - **Screen flow transitions (replaces `nActive` writes and `vCnt.size()` math)**:
    - `void Next()` (already exists)
    - `void Restart(int start_screen = -1)` (already exists)
    - `void EnterMenu()` (stores resume screen internally and activates menu)
    - `void ExitMenuResume()` (restores internally saved resume screen)
    - `void RestartFromChapter(int chapter)` (menu “chapter 1/2/3”)
    - `void ShowGameOverScreen()` (for level/game-over jump)
  - **Observability without index**:
    - `std::string GetActiveControllerName() const`
    - `bool IsOnGameOverScreen() const` (used by `simulation_test`)
- **Graphics & drawing**
  - `GraphicalInterface<Index> *GetGraphics() const` (needed for `MouseCursor::DrawCursor`)
  - `void RefreshAll()`
  - `smart_pointer<ScalingDrawer> GetDrawer() const`
  - `unsigned GetDrawScaleFactor() const` (replaces `pDr->nFactor` reads)
- **Text drawers / fonts**
  - `smart_pointer<NumberDrawer> GetNumberDrawer() const`
  - `smart_pointer<NumberDrawer> GetBigNumberDrawer() const`
  - `FontWriter *GetFancyFont() const`
- **Sound**
  - `void PlaySound(std::string key)` (replaces almost all `pSnd->PlaySound(GetSnd("..."))`)
  - `void ToggleSoundOutput()`
  - `bool IsSoundOutputOn() const`
  - Menu persists the setting via its injected `DragonGameSettings *`.
- **Music playback (non-savable side effects)**
  - `void ToggleMusicPlayback()`
  - `bool IsMusicPlaybackOff() const`
  - Menu persists the setting via its injected `DragonGameSettings *`.
- **Score / high score**
  - `int GetScore() const`, `int GetHighScore() const`
  - `void AddScore(int delta)` (replaces `nScore += ...`)
  - `void UpdateHighScoreIfNeeded()` (also persists to `high.txt` via `FilePath`)
- **Game state flags**
  - `bool IsAngry() const`, `void SetAngry()`
- **Savable options (new struct)**
  - Add `struct DragonGameSettings` that owns all `SavableVariable<>` members:
    - `snProgress`, `sbSoundOn`, `sbMusicOn`, `sbTutorialOn`, `sbFullScreen`,
    `sbCheatsOn`, `sbCheatsUnlocked`
  - Define this struct in the same file as `DragonGameController` (in
  `dragon_game_controller.h`), not in a separate header.
  - `DragonGameController` keeps this struct private and exposes **getters only**
  (read-only access) for non-menu code:
    - `int GetProgress() const`
    - `bool IsSoundOnSetting() const`
    - `bool IsMusicOnSetting() const`
    - `bool IsTutorialOnSetting() const`
    - `bool IsFullScreenSetting() const`
    - `bool AreCheatsOnSetting() const`
    - `bool CheatsUnlocked() const`
  - `TutorialTextEntity` receives `DragonGameController *` and calls
  `IsTutorialOnSetting()` when it needs the tutorial-on state (no
  `GetTutorialOnConstPointer()`).
  - `MenuController` receives a `DragonGameSettings *` pointer (injected at
  construction) and may mutate/persist settings directly.
  Non-setting side effects remain behind `DragonGameController` APIs
  (e.g. toggling sound proxy or music playback).
- **Bounds / wrapper**
  - `const Rectangle &GetBounds() const`
  - `Size GetActualResolution() const`
  - `FilePath *GetFilePath() const`
  - `void ExitProgram()` (wraps `Trigger(pWrp->pExitProgram)`)
- **Carry-over bonuses**
  - `void ClearBonusesToCarryOver()`
  - `void AddBonusToCarryOver(smart_pointer<TimedFireballBonus> b)`
  - `const std::list<smart_pointer<TimedFireballBonus>> &GetBonusesToCarryOver() const`

All former public data members become `private:`.

## Call-site updates (files to change)

Update direct field access to the new APIs:

- `[code/game/dragon_game_runner.cc](code/game/dragon_game_runner.cc)`
  - Replace direct `vCnt`/`nActive` reads + dispatch with
  `GetTowerController()->GetActiveController()` and dispatch through the
  returned `GameController *`.
- `[code/game/dragon_game_runner.h](code/game/dragon_game_runner.h)`
  - Remove `GetActiveControllerIndex()` and `GetControllerCount()` so index is
  not visible outside `DragonGameController`.
  - Keep `GetActiveControllerName()` for simulation/inspection.
- `[code/game/controller/level_controller.cc](code/game/controller/level_controller.cc)`
  - Replace `pGl->nScore`, `pGl->pNum`, `pGl->pGraph`, `pGl->rBound`, `pGl->sb*`,
  `pGl->pSnd`, and `pGl->nActive = pGl->vCnt.size()-2`.
  - Replace the game-over jump with `pGl->ShowGameOverScreen()`.
  - Pass `DragonGameController *` (e.g. `pGl`) into `TutorialTextEntity` instead
  of `pGl->sbTutorialOn.GetConstPointer()`; the entity calls
  `IsTutorialOnSetting()` when it needs the tutorial-on state.
- `[code/game/controller/menu_controller.cc](code/game/controller/menu_controller.cc)`
  - Replace direct reads/writes of `pGl->sb*` / `pGl->snProgress` with the
  injected `DragonGameSettings *settings_`.
  - Replace `pGl->plr` access with `DragonGameController` methods (toggle/query
  music playback), and replace `pGl->pSnd` toggle/get with
  `ToggleSoundOutput()` / `IsSoundOutputOn()`.
  - Replace `pGl->nActive` writes with `ExitMenuResume()` (continue) and remove
  any menu-visible index state.
  - Replace chapter restart via `pGl->vLevelPointers` with
  `pGl->RestartFromChapter(chapter)`.
  - Replace exit via `pGl->pWrp->pExitProgram` with `pGl->ExitProgram()`.
- `[code/game/controller/basic_controllers.cc](code/game/controller/basic_controllers.cc)`
  - Replace `pGl->pDr->nFactor`, `pGl->pGraph->RefreshAll()`, `pGl->pSnd->PlaySound(...)`, `pGl->sbCheatsOn.Get()`, `pGl->nScore/nHighScore` writes/reads, and `pGl->pWrp->GetFilePath()`.
- `[code/game/entities.cc](code/game/entities.cc)`
  - Replace `pGl->pBigNum`, `pGl->pFancyNum`, `pGl->nScore/nHighScore`, and score increments.
- `[code/game/dragon.cc](code/game/dragon.cc)`
  - Replace direct access to `lsBonusesToCarryOver` with the carry-over bonus APIs.
- `[code/game/critters.cc](code/game/critters.cc)` and `[code/game/critter_generators.cc](code/game/critter_generators.cc)`
  - Replace `pGl->pSnd->PlaySound(...)`, `pGl->bAngry`, `pGl->nHighScore`, `pGl->pNum`.
- `[code/game/fireball.cc](code/game/fireball.cc)`
  - Replace `pGl->pSnd->PlaySound(...)`.
- `[code/game/controller/buy_now_controller.cc](code/game/controller/buy_now_controller.cc)`
  - Replace `pGl->pDr` and `pGl->pGraph->RefreshAll()`.
- `[code/simulation/simulation.cc](code/simulation/simulation.cc)`
  - Replace `twr->nScore` / `twr->nHighScore` reads with getters.
  - Remove index/count logging; log screen name only.
- `[code/simulation/simulation_test.cc](code/simulation/simulation_test.cc)`
  - Replace `GetActiveControllerIndex() == GetControllerCount() - 2` checks with
  `IsOnGameOverScreen()` and/or controller-name-based assertions.

## Header hygiene (as part of touching the header)

- Add missing standard includes in `dragon_game_controller.h` for types it uses directly (notably `#include <list>` and `#include <vector>`), to avoid relying on transitive includes.
- Keep line width under 80 characters.

## Verification (required)

From the workspace root:

- Build (no CMake changes expected, so build-only):
  - `cd build; mingw32-make`
- Run tests:
  - `cd build; ctest --output-on-failure`
- Run the most important test:
  - `cd bin; .\simulation_test.exe`

If any failures occur (even seemingly unrelated), fix them before finishing.