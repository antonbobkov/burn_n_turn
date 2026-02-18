# DragonGameController Public Members – Access Catalog

This document lists where each public data member and public method of
`DragonGameController` is accessed from, and whether each access is a read,
write, or a call to a member function on that member.

**Source of truth:** `code/game/controller/dragon_game_controller.h`

---

## Public data members

### `vCnt` (vector of GameController)

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | Read: `.size()`, `[nActive]`; member calls: `->Update()`, `->OnKey()`, `->OnMouse()`, `->OnMouseDown()`, `->OnMouseUp()`, `->DoubleClick()`, `->Fire()`, `->GetControllerName()` |
| LevelController | `game/controller/level_controller.cc` | Read: `.size()` (in expression for `nActive`) |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Read: `.size()` in `Next()`, `Restart()`; write: `.clear()` in `Restart()`; used when building controllers in `StartUp()` |

### `nActive` (unsigned)

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | Read: for controller count, current name, and indexing into `vCnt` |
| MenuController | `game/controller/menu_controller.cc` | Write: `pGl->nActive = pMenuController->nResumePosition` |
| LevelController | `game/controller/level_controller.cc` | Write: `pGl->nActive = pGl->vCnt.size() - 2` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Read/write: in `Next()`, `Restart()`, `Menu()` |

### `vLevelPointers` (vector<int>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Read: `.at(0)`, `.at(1)`, `.at(2)` passed to `Restart()` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Read: `.at(i)` in `Next()`; write: initialised in ctor (size 3) |

### `pMenu` (smart_pointer<MenuController>)

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | Member calls: `->AddBoth()`, `->AddV()`, `->AddE()`, `->pMenuDisplay =`, `->pMenuDisplay->UpdateMenuEntries()`, `->nResumePosition =`; also `->pHintText =`, `->pOptionText =` (PC_VERSION) |

### `pGraph` (smart_pointer<GraphicalInterface<Index>>)

| Component | File | Access |
|-----------|------|--------|
| SimpleController / FlashingController | `game/controller/basic_controllers.cc` | Member calls: `->LoadImage()`, `->DeleteImage()`, `->DrawImage()`, `->GetImage()`, `->RefreshAll()` |
| EntityListController | `game/controller/basic_controllers.cc` | Member call: `->RefreshAll()` |
| LevelController | `game/controller/level_controller.cc` | Member calls: `->DrawRectangle()`, `->RefreshAll()`, `->DrawCursor()` (via `.get()`) |
| MenuController | `game/controller/menu_controller.cc` | Member call: `->RefreshAll()` |
| BuyNowController | `game/controller/buy_now_controller.cc` | Member call: `->RefreshAll()` |

### `pDr` (smart_pointer<ScalingDrawer>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Member call: passed to `Draw()` (option/hint text) |
| EntityListController | `game/controller/basic_controllers.cc` | Read: `->nFactor`; member call: `->Draw()` for entries |
| BuyNowController | `game/controller/buy_now_controller.cc` | Member call: `->Draw()` for slimes |
| LevelController | `game/controller/level_controller.cc` | (LevelController uses its own drawer; no direct `pGl->pDr` in the grep – `pNum` used for HUD) |

### `pNum` (smart_pointer<NumberDrawer>)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Member calls: `->DrawNumber()`, `->DrawWord()`, `->DrawColorNumber()`, `->DrawColorWord()`; also passed to tutorial entity |
| Critters | `game/critters.cc` | Member call: `->DrawWord()` (via `pAc->pGl->pNum`) |
| Fireball | `game/fireball.cc` | Member call: `->DrawWord()` |
| Entities | `game/entities.cc` | Member calls: `->DrawWord()`, `->DrawColorWord()` |

### `pBigNum` (smart_pointer<NumberDrawer>)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Member call: `->DrawWord()` |
| HighScoreShower / IntroTextShower (entities) | `game/entities.cc` | Member calls: `->DrawWord()`, `->DrawNumber()` (score, high score) |

### `pFancyNum` (smart_pointer<FontWriter>)

| Component | File | Access |
|-----------|------|--------|
| IntroTextShower | `game/entities.cc` | Member calls: `->GetSize()`, `->DrawWord()` |

### `pSndRaw` (smart_pointer<SoundInterface<Index>>)

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | Used in ctor (e.g. `plr.pSnd = pSndRaw_`), and in `DrawStuff()` during loading |

### `pSnd` (smart_pointer<SoundInterfaceProxy>)

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | Member calls: `->SetVolume()`, and via `plr`; also `PlaySound(GetSnd(...))` during loading |
| MenuController | `game/controller/menu_controller.cc` | Member calls: `->PlaySound()`, `->Toggle()`, `->Get()`; `sbSoundOn.Set(pSnd->Get())` |
| LevelController | `game/controller/level_controller.cc` | Member call: `->PlaySound()` |
| Basic controllers | `game/controller/basic_controllers.cc` | Member call: `->PlaySound()` |
| Critters | `game/critters.cc` | Member call: `->PlaySound()` (via `pAc->pGl`, `pAdv->pGl`) |
| CritterGenerators | `game/critter_generators.cc` | Member call: `->PlaySound()` |
| Dragon | `game/dragon.cc` | Member call: `->PlaySound()` |
| Fireball | `game/fireball.cc` | Member call: `->PlaySound()` |

### `vLvl` (vector<LevelLayout>)

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | Read: in ctor (copy), in `StartUp()` loop (`.size()`, `vLvl[i]`) when creating LevelControllers |

### `nScore` (int)

| Component | File | Access |
|-----------|------|--------|
| Simulation | `simulation/simulation.cc` | Read: for logging |
| DragonGameRunner | (only via GetTowerController(); score read in simulation) | — |
| LevelController | `game/controller/level_controller.cc` | Read: for HUD drawing |
| Entities | `game/entities.cc` | Read: HighScoreShower; write: `pGl->nScore +=` (score updates) |
| DragonScoreController (basic_controllers) | `game/controller/basic_controllers.cc` | Read: for high-score check and output; write: `pGl->nHighScore = pGl->nScore` when updating high score |

### `nHighScore` (int)

| Component | File | Access |
|-----------|------|--------|
| Simulation | `simulation/simulation.cc` | Read: for logging |
| CritterGenerators | `game/critter_generators.cc` | Read: `pBc->pGl->nHighScore == 0` (first-time logic) |
| Entities | `game/entities.cc` | Read: for drawing in HighScoreShower |
| DragonScoreController | `game/controller/basic_controllers.cc` | Read/write: compare and set from `nScore` |

### `bAngry` (bool)

| Component | File | Access |
|-----------|------|--------|
| Critters | `game/critters.cc` | Write: `pAc->pGl->bAngry = true` |
| CritterGenerators | `game/critter_generators.cc` | Read: `pBc->pGl->bAngry`; passed into constructor |

### `snProgress` (SavableVariable<int>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Read: `.Get()` for menu state |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Read: `.Get()` in `Next()`; write: `.Set(i)` in `Next()` |

### `sbSoundOn` (SavableVariable<bool>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Write: `.Set(...)`; read: `.Get()` for menu display |

### `sbMusicOn` (SavableVariable<bool>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Write: `.Set(...)`; read: `.Get()` for menu display |
| LevelController | `game/controller/level_controller.cc` | Read: `.Get()` for gameplay logic |

### `sbTutorialOn` (SavableVariable<bool>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Member call: `BoolToggle(pGl->sbTutorialOn)`; read: `.Get()` |
| LevelController | `game/controller/level_controller.cc` | Read: `.GetConstPointer()` passed to entity |

### `sbFullScreen` (SavableVariable<bool>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Member call: `BoolToggle()`; read: `.Get()` |

### `sbCheatsOn` (SavableVariable<bool>)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Member call: `BoolToggle()`; read: `.Get()` |
| LevelController | `game/controller/level_controller.cc` | Read: `.Get()` |
| Basic controllers | `game/controller/basic_controllers.cc` | Read: `.Get()` (cheat key) |

### `sbCheatsUnlocked` (SavableVariable<bool>)

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | Read: `.Get()` passed to MenuDisplay ctor |

### `rBound` (Rectangle)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Read: `pGl->rBound.sz.x`, `pGl->rBound.sz.y` for positioning |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Read/write: used in ctor and StartUp (e.g. menu position); passed to controllers. Not accessed from other files as `pGl->rBound` except LevelController above. |

### `plr` (BackgroundMusicPlayer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Member call: `->ToggleOff()`; read: `.bOff` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Write: `plr.pSnd =`, `plr.vThemes[...] =`; member call: `plr.pSnd->SetVolume()` |

### `lsBonusesToCarryOver` (list of TimedFireballBonus)

| Component | File | Access |
|-----------|------|--------|
| Dragon | `game/dragon.cc` | Member calls: `.push_back()`, `.clear()`; read: `.begin()`, `.end()` for iteration |
| LevelController | `game/controller/level_controller.cc` | Member call: `.clear()` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | (Indirect: level setup; list is cleared by LevelController/Dragon.) |

### `pWrp` (TowerDataWrap*)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Member access: `->pExitProgram` (in `Exit()`) |
| LevelController | `game/controller/level_controller.cc` | Read: `->szActualRez` |
| DragonScoreController | `game/controller/basic_controllers.cc` | Member call: `->GetFilePath()` |

### `pSelf` (DragonGameController*)

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | Write: in `StartUp(pSelf_)`; read: passed to controller constructors in `StartUp()` |

---

## Public methods

### `StartUp(DragonGameController *pSelf)`

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | Call: `pCnt->StartUp(pCnt.get())` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Call: `StartUp(pSelf)` from `Restart()` |

### `Next()`

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | (Definition) |
| Dragon | `game/dragon.cc` | Call: `pAd->pGl->Next()` |
| Basic controllers | `game/controller/basic_controllers.cc` | Call: `pGl->Next()` (many controllers) |
| LevelController | `game/controller/level_controller.cc` | Call: `pGl->Next()` (cheat) |

### `Restart(int nActive_)`

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Call: `pGl->Restart()` and `Restart(vLevelPointers.at(0/1/2))` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | Call: `Restart()` from `Next()` (no arg) |

### `Menu()`

| Component | File | Access |
|-----------|------|--------|
| EntityListController | `game/controller/basic_controllers.cc` | Call: `pGl->Menu()` |
| LevelController | `game/controller/level_controller.cc` | Call: `pGl->Menu()` |

### `GetImg(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| Level | `game/level.cc` | Call: `pAd->pGl->GetImg("road")` (and usage of returned reference) |
| Fireball | `game/fireball.cc` | Call: `pAd->pGl->GetImg("empty")` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | (Definition; also used internally via preloader in loading) |

### `GetImgSeq(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Call: `GetImgSeq(...)` for bonus, claw, dragon, etc. |
| MenuController | `game/controller/menu_controller.cc` | Call: `pGl_->GetImgSeq("claw")` in ctor |
| Basic controllers | `game/controller/basic_controllers.cc` | Call: `pGl_->GetImgSeq(sRun)`, `GetImgSeq(sChase)` |
| BuyNowController | `game/controller/buy_now_controller.cc` | Call: `pGl->GetImgSeq("slime")` |
| Critters | `game/critters.cc` | Call: many `pAc->pGl->GetImgSeq(...)`, `pAdv->pGl->GetImgSeq(...)` |
| CritterGenerators | `game/critter_generators.cc` | Call: `pBc->pGl->GetImgSeq(...)`, `pAdv->pGl->GetImgSeq(...)` |
| Dragon | `game/dragon.cc` | (Images/sequences from level/fireball; GetImgSeq used via level/fireball) |
| Fireball | `game/fireball.cc` | Call: `pBc->pGl->GetImgSeq(...)`, `pAd->pGl->GetImgSeq(...)` |

### `GetSnd(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| All components that call `pGl->pSnd->PlaySound(pGl->GetSnd("..."))` | Multiple (critters, dragon, fireball, menu, level, basic_controllers, critter_generators) | Call: used to get sound index for `PlaySound()` |
| DragonGameController | `game/controller/dragon_game_controller.cc` | (Definition; also used via preloader for `plr.vThemes`) |

### `GetSndSeq(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| DragonGameController | `game/controller/dragon_game_controller.cc` | (Definition; used for menu/game-over sound entities via preloader `pr->GetSndSeq(...)`) |

---

## Summary by component

- **DragonGameRunner** – vCnt (read, member calls), nActive (read), StartUp (call).
- **Simulation** – nScore, nHighScore (read, for logging).
- **LevelController** – vCnt (read), nActive (write), pGraph, pNum, pBigNum, pSnd, rBound, sbMusicOn, sbTutorialOn, sbCheatsOn, lsBonusesToCarryOver, pWrp; Next, Menu, GetImgSeq, GetSnd (calls).
- **MenuController** – nActive (write), vLevelPointers (read), pGraph, pDr, pSnd, snProgress, sbSoundOn, sbMusicOn, sbTutorialOn, sbFullScreen, sbCheatsOn, plr, pWrp; Restart, GetImgSeq, GetSnd (calls).
- **Basic controllers** – pGraph, pDr, pNum, pSnd, nScore, nHighScore, sbCheatsOn, pWrp; Next, Menu, GetImgSeq, GetSnd (calls).
- **BuyNowController** – pGraph, pDr, GetImgSeq (calls).
- **Dragon** – pSnd, lsBonusesToCarryOver; Next, GetImgSeq, GetSnd (calls).
- **Critters / CritterGenerators** – pNum, pSnd, nHighScore, bAngry; GetImgSeq, GetSnd (calls).
- **Fireball** – pNum, pSnd; GetImg, GetImgSeq, GetSnd (calls).
- **Entities** – pBigNum, pFancyNum, pNum, nScore, nHighScore; GetImgSeq (via other types) and score write.
- **Level** – GetImg (call).
- **DragonGameController (self)** – All members and methods used or defined in `dragon_game_controller.cc`.
