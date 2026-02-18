# DragonGameController Public Members – Access Catalog

This document lists where each public data member and public method of
`DragonGameController` is accessed from **other classes** (DragonGameController
itself is omitted). For each access, the table describes what the access is
used for.

**Source of truth:** `code/game/controller/dragon_game_controller.h`

---

## Public data members

### `vCnt` (vector of GameController)

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | Read `.size()` and `[nActive]`; call `->Update()`, `->OnKey()`, `->OnMouse()`, etc. on the active controller. Used to forward frame updates and input to the currently active screen (menu, level, cutscene, etc.) and to report the active controller name. |
| LevelController | `game/controller/level_controller.cc` | Read `.size()` when computing which index to switch to. Used when leaving the level (e.g. game over) to set the active controller back to the menu (second-to-last in the vector). |

### `nActive` (unsigned)

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | Read to know how many controllers exist and which one is active, and to index into `vCnt` for updates and input. Used so the runner can delegate to the correct controller. |
| MenuController | `game/controller/menu_controller.cc` | Write: set to the menu’s stored resume position when returning from the menu. Ensures the game returns to the correct screen (e.g. level or start) after the user picks an option. |
| LevelController | `game/controller/level_controller.cc` | Write: set to the menu index when leaving the level. Ensures the menu is shown (e.g. after game over) instead of staying on the level. |

### `vLevelPointers` (vector of int)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Read `.at(0)`, `.at(1)`, `.at(2)` and pass each to `Restart()`. Used when the user selects a level from the menu so the game restarts at the chosen level index. |

### `pMenu` (`smart_pointer` to MenuController)

Only accessed by DragonGameController (omitted).

### `pGraph` (`smart_pointer` to GraphicalInterface of Index)

| Component | File | Access |
|-----------|------|--------|
| EntityListController | `game/controller/basic_controllers.cc` | Call `->RefreshAll()`. Used to redraw the screen when the entity list (e.g. level select) is updated. |
| LevelController | `game/controller/level_controller.cc` | Call `->DrawRectangle()`, `->RefreshAll()`, and `->DrawCursor()` (via `.get()`). Used to draw overlays (e.g. dimming), refresh the display, and show the cursor during the level. |
| MenuController | `game/controller/menu_controller.cc` | Call `->RefreshAll()`. Used to refresh the display when the menu is shown or updated. |
| BuyNowController | `game/controller/buy_now_controller.cc` | Call `->RefreshAll()`. Used to refresh the buy-now / trial screen. |

### `pDr` (`smart_pointer` to ScalingDrawer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Pass to `Draw()` for option and hint text. Used so menu text is drawn with the correct scaling. |
| EntityListController | `game/controller/basic_controllers.cc` | Read `->nFactor` for layout; call `->Draw()` for list entries. Used to scale and draw the level-select (or similar) list. |
| BuyNowController | `game/controller/buy_now_controller.cc` | Call `->Draw()` for slime entities. Used to draw the animated slimes on the buy-now screen. |

### `pNum` (`smart_pointer` to NumberDrawer)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Call `->DrawNumber()`, `->DrawWord()`, `->DrawColorNumber()`, `->DrawColorWord()`; also pass to the tutorial entity. Used to draw the in-level HUD (score, level, time, labels) and to let the tutorial draw its text. |
| Critters | `game/critters.cc` | Call `->DrawWord()` (via `pAc->pGl->pNum`). Used to draw text under critters (e.g. damage or labels). |
| Fireball | `game/fireball.cc` | Call `->DrawWord()`. Used to draw text under fireballs (e.g. labels). |
| Entities | `game/entities.cc` | Call `->DrawWord()`, `->DrawColorWord()`. Used to draw text on generic visual entities. |

### `pBigNum` (`smart_pointer` to NumberDrawer)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Call `->DrawWord()`. Used to draw large text on the level HUD when needed. |
| HighScoreShower / IntroTextShower (entities) | `game/entities.cc` | Call `->DrawWord()`, `->DrawNumber()` for score and high score. Used to show the big score and high score on the intro/high-score screens. |

### `pFancyNum` (`smart_pointer` to FontWriter)

| Component | File | Access |
|-----------|------|--------|
| IntroTextShower | `game/entities.cc` | Call `->GetSize()` and `->DrawWord()`. Used to measure and draw the fancy intro text. |

### `pSndRaw` (`smart_pointer` to SoundInterface of Index)

Only accessed by DragonGameController (omitted).

### `pSnd` (`smart_pointer` to SoundInterfaceProxy)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Call `->PlaySound()`, `->Toggle()`, `->Get()`; sync `sbSoundOn` with `pSnd->Get()`. Used to play menu sounds (e.g. beeps), toggle sound on/off, and keep the saved sound setting in sync with the proxy. |
| LevelController | `game/controller/level_controller.cc` | Call `->PlaySound()`. Used to play in-level sounds (e.g. timer beeps). |
| Basic controllers | `game/controller/basic_controllers.cc` | Call `->PlaySound()`. Used to play sounds on start screen, cutscenes, and score screens (e.g. start game, beep/boop). |
| Critters | `game/critters.cc` | Call `->PlaySound()` (via `pAc->pGl`, `pAdv->pGl`). Used to play critter sounds (death, steps, hits, spawn, etc.). |
| CritterGenerators | `game/critter_generators.cc` | Call `->PlaySound()`. Used to play sounds when critters are spawned (e.g. princess arrive). |
| Dragon | `game/dragon.cc` | Call `->PlaySound()`. Used to play dragon sounds (pickup, shoot, capture, win, etc.). |
| Fireball | `game/fireball.cc` | Call `->PlaySound()`. Used to play fireball sounds (death, explosion). |

### `vLvl` (vector of LevelLayout)

Only accessed by DragonGameController (omitted).

### `nScore` (int)

| Component | File | Access |
|-----------|------|--------|
| Simulation | `simulation/simulation.cc` | Read for logging. Used to print the current score in test output. |
| LevelController | `game/controller/level_controller.cc` | Read for HUD drawing. Used to show the current score on the level screen. |
| Entities | `game/entities.cc` | Read in HighScoreShower; write `pGl->nScore +=` when awarding points. Used to display score and to add points when the player earns them (e.g. killing critters). |
| DragonScoreController (basic_controllers) | `game/controller/basic_controllers.cc` | Read to compare with high score and to write to the output stream; write `pGl->nHighScore = pGl->nScore` when the score exceeds high score. Used to update and persist the high score and to show score at the end of a run. |

### `nHighScore` (int)

| Component | File | Access |
|-----------|------|--------|
| Simulation | `simulation/simulation.cc` | Read for logging. Used to print the high score in test output. |
| CritterGenerators | `game/critter_generators.cc` | Read `pBc->pGl->nHighScore == 0`. Used to decide first-time behaviour (e.g. different spawns when the player has never scored). |
| Entities | `game/entities.cc` | Read for drawing in HighScoreShower. Used to show the high score on the intro/high-score screen. |
| DragonScoreController (basic_controllers) | `game/controller/basic_controllers.cc` | Read and write when comparing with `nScore`. Used to update the saved high score when the current run beats it. |

### `bAngry` (bool)

| Component | File | Access |
|-----------|------|--------|
| Critters | `game/critters.cc` | Write `pAc->pGl->bAngry = true`. Used to mark the game as “angry” when the player triggers certain events (e.g. attacking a princess). |
| CritterGenerators | `game/critter_generators.cc` | Read `pBc->pGl->bAngry` and pass into constructors. Used to spawn or configure critters differently when the game is in angry mode. |

### `snProgress` (SavableVariable of int)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Read `.Get()` to decide menu state. Used to enable/disable menu options (e.g. which levels are unlocked) based on how far the player has progressed. |

### `sbSoundOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Write `.Set(...)` when the user toggles sound; read `.Get()` for menu label. Used to persist and display the “sound on/off” setting. |

### `sbMusicOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Write `.Set(...)` when the user toggles music; read `.Get()` for menu label. Used to persist and display the “music on/off” setting. |
| LevelController | `game/controller/level_controller.cc` | Read `.Get()` in gameplay logic. Used to decide whether to play or allow music during the level (e.g. ghost-time or blink logic). |

### `sbTutorialOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Call `BoolToggle(pGl->sbTutorialOn)`; read `.Get()` for menu label. Used to let the user turn the tutorial on/off and to show the current state in the menu. |
| LevelController | `game/controller/level_controller.cc` | Read `.GetConstPointer()` and pass to an entity. Used so the tutorial entity can show or hide hints based on the saved tutorial setting. |

### `sbFullScreen` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Call `BoolToggle()`; read `.Get()`. Used to toggle fullscreen from the menu and to show the current fullscreen state (e.g. for deciding whether to show “exit” option). |

### `sbCheatsOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Call `BoolToggle()`; read `.Get()` for menu. Used to let the user enable/disable cheats and to show whether cheats are on. |
| LevelController | `game/controller/level_controller.cc` | Read `.Get()`. Used to enable cheat behaviour during the level (e.g. skip to next level). |
| Basic controllers | `game/controller/basic_controllers.cc` | Read `.Get()` when handling key input. Used to react to the cheat key (e.g. backslash) only when cheats are enabled. |

### `sbCheatsUnlocked` (SavableVariable of bool)

Only accessed by DragonGameController (omitted).

### `rBound` (Rectangle)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Read `pGl->rBound.sz.x` and `pGl->rBound.sz.y`. Used to position UI (e.g. tutorial hint) relative to the game’s screen size. |

### `plr` (BackgroundMusicPlayer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Call `->ToggleOff()`; read `.bOff`. Used to mute/unmute background music when the user toggles music in the menu, and to keep the menu label in sync with the actual mute state. |

### `lsBonusesToCarryOver` (list of `smart_pointer` to TimedFireballBonus)

| Component | File | Access |
|-----------|------|--------|
| Dragon | `game/dragon.cc` | Call `.push_back()`, `.clear()`; read `.begin()`, `.end()` to iterate. Used to carry timed fireball bonuses from one level to the next (dragon adds bonuses when finishing a level and consumes/clears them when starting the next). |
| LevelController | `game/controller/level_controller.cc` | Call `.clear()`. Used to clear carried-over bonuses when starting or resetting a level so the list does not persist incorrectly. |

### `pWrp` (TowerDataWrap pointer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Read `->pExitProgram` in `Exit()`. Used to trigger program exit when the user chooses “exit” from the menu. |
| LevelController | `game/controller/level_controller.cc` | Read `->szActualRez`. Used to get the actual resolution for layout or rendering decisions during the level. |
| DragonScoreController (basic_controllers) | `game/controller/basic_controllers.cc` | Call `->GetFilePath()`. Used to get the file path for saving high score (or similar) to disk. |

### `pSelf` (DragonGameController pointer)

Only accessed by DragonGameController (omitted).

---

## Public methods

### `StartUp(DragonGameController *pSelf)`

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | Call `pCnt->StartUp(pCnt.get())` after creating the controller. Used to initialise the controller and build the initial screen stack (menu, start screen, etc.). |

### `Next()`

| Component | File | Access |
|-----------|------|--------|
| Dragon | `game/dragon.cc` | Call `pAd->pGl->Next()`. Used when the dragon wins the level (e.g. all princesses captured) to advance to the next screen. |
| Basic controllers | `game/controller/basic_controllers.cc` | Call `pGl->Next()` from various controllers (start screen, cutscene, score screen, entity list, etc.). Used when the user completes a screen or clicks to advance (e.g. “next level”, “continue”). |
| LevelController | `game/controller/level_controller.cc` | Call `pGl->Next()` when a cheat is used. Used to skip to the next level from in-game. |

### `Restart(int nActive_)`

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | Call `pGl->Restart()` or `Restart(vLevelPointers.at(0/1/2))`. Used when the user picks “resume” or a specific level from the menu so the game restarts from the chosen point. |

### `Menu()`

| Component | File | Access |
|-----------|------|--------|
| EntityListController | `game/controller/basic_controllers.cc` | Call `pGl->Menu()`. Used when the user chooses “back to menu” from the level-select (or similar) screen. |
| LevelController | `game/controller/level_controller.cc` | Call `pGl->Menu()`. Used when the user pauses or quits the level to return to the main menu. |

### `GetImg(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| Level | `game/level.cc` | Call `pAd->pGl->GetImg("road")` and use the returned reference. Used to get the road tile image for drawing the level’s road. |
| Fireball | `game/fireball.cc` | Call `pAd->pGl->GetImg("empty")`. Used to get the empty/placeholder image when needed for drawing. |

### `GetImgSeq(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | Call `GetImgSeq(...)` for bonus icons, claw cursor, dragon, timer, etc. Used to get image sequences for the level HUD and entities. |
| MenuController | `game/controller/menu_controller.cc` | Call `pGl_->GetImgSeq("claw")` in ctor. Used to get the claw cursor sequence for the menu. |
| Basic controllers | `game/controller/basic_controllers.cc` | Call `pGl_->GetImgSeq(sRun)`, `GetImgSeq(sChase)` for cutscenes. Used to get the run and chase animation sequences for cutscene playback. |
| BuyNowController | `game/controller/buy_now_controller.cc` | Call `pGl->GetImgSeq("slime")`. Used to get the slime animation for the buy-now screen. |
| Critters | `game/critters.cc` | Call `pAc->pGl->GetImgSeq(...)`, `pAdv->pGl->GetImgSeq(...)` for many sequences (death, walk, burn, spawn, etc.). Used to get the correct sprite sequences for each critter type and state. |
| CritterGenerators | `game/critter_generators.cc` | Call `pBc->pGl->GetImgSeq(...)`, `pAdv->pGl->GetImgSeq(...)`. Used to get image sequences when constructing critters (golem, princess, mage, trader, skeleton, etc.). |
| Fireball | `game/fireball.cc` | Call `pBc->pGl->GetImgSeq(...)`, `pAd->pGl->GetImgSeq(...)` for fireball, laser, explosion, bonus. Used to get the correct graphics for fireballs and effects. |

### `GetSnd(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| MenuController, LevelController, Basic controllers, Critters, CritterGenerators, Dragon, Fireball | Various | Call `pGl->GetSnd("...")` and pass the result to `pGl->pSnd->PlaySound()`. Used to resolve a sound name to an index and then play that sound (menu beeps, level sounds, critter/dragon/fireball effects, etc.). |

### `GetSndSeq(std::string key)`

Only used internally by DragonGameController (e.g. for menu/game-over sound entities via the preloader). No other classes call it (omitted).

---

## Summary by component

- **DragonGameRunner** – vCnt, nActive (read / delegate to active controller); StartUp (call to initialise).
- **Simulation** – nScore, nHighScore (read for logging).
- **LevelController** – vCnt, nActive (read/write for screen switching); pGraph, pNum, pBigNum, pSnd, rBound, sbMusicOn, sbTutorialOn, sbCheatsOn, lsBonusesToCarryOver, pWrp; Next, Menu, GetImgSeq, GetSnd (calls).
- **MenuController** – nActive, vLevelPointers, pGraph, pDr, pSnd, snProgress, sbSoundOn, sbMusicOn, sbTutorialOn, sbFullScreen, sbCheatsOn, plr, pWrp; Restart, GetImgSeq, GetSnd (calls).
- **Basic controllers** – pGraph, pDr, pNum, pSnd, nScore, nHighScore, sbCheatsOn, pWrp; Next, Menu, GetImgSeq, GetSnd (calls).
- **BuyNowController** – pGraph, pDr, GetImgSeq (calls).
- **Dragon** – pSnd, lsBonusesToCarryOver; Next, GetImgSeq, GetSnd (calls).
- **Critters / CritterGenerators** – pNum, pSnd, nHighScore, bAngry; GetImgSeq, GetSnd (calls).
- **Fireball** – pNum, pSnd; GetImg, GetImgSeq, GetSnd (calls).
- **Entities** – pBigNum, pFancyNum, pNum, nScore, nHighScore (draw and update score).
- **Level** – GetImg (call for road image).
