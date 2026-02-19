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

- **Controller stack**
  - `GameController *GetActiveController() const`
  - `unsigned GetActiveControllerIndex() const`
  - `unsigned GetControllerCount() const`
  - `std::string GetActiveControllerName() const`
  - `void SetActiveControllerIndex(unsigned idx)` (used by menu “continue”)
  - `void ShowGameOverScreen()` (replaces `nActive = vCnt.size() - 2`)
  - `void RestartChapter(unsigned chapterIndex)` (replaces `vLevelPointers.at(i)` in menu)
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
  - `bool IsSoundOn() const` + `void ToggleSoundSetting()` (updates proxy + persists via savable)
- **Score / high score**
  - `int GetScore() const`, `int GetHighScore() const`
  - `void AddScore(int delta)` (replaces `nScore += ...`)
  - `void UpdateHighScoreIfNeeded()` (also persists to `high.txt` via `FilePath`)
- **Game state flags**
  - `bool IsAngry() const`, `void SetAngry()`
- **Savable options**
  - `int GetProgress() const`
  - `bool IsMusicOn() const` + `void ToggleMusicSetting()` (keeps `plr` + savable in sync)
  - `bool IsTutorialOn() const` + `void ToggleTutorialSetting()` + `const bool *GetTutorialOnConstPointer() const`
  - `bool IsFullScreen() const` + `void ToggleFullScreenSetting()`
  - `bool AreCheatsOn() const` + `void ToggleCheatsSetting()`
  - `bool CheatsUnlocked() const`
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
  - Replace direct `vCnt`/`nActive` reads + dispatch with `GetActiveController*()` methods.
- `[code/game/controller/level_controller.cc](code/game/controller/level_controller.cc)`
  - Replace `pGl->nScore`, `pGl->pNum`, `pGl->pGraph`, `pGl->rBound`, `pGl->sb*`, `pGl->pSnd`, and `pGl->nActive = pGl->vCnt.size()-2`.
- `[code/game/controller/menu_controller.cc](code/game/controller/menu_controller.cc)`
  - Replace `pGl->pDr`, `pGl->pGraph`, `pGl->pSnd`, `pGl->sb*`, `pGl->plr`, `pGl->nActive`, `pGl->vLevelPointers`, `pGl->pWrp->pExitProgram`.
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