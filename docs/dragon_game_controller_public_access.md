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
| DragonGameRunner | `game/dragon_game_runner.cc` | **Intent:** The runner must drive the game loop without knowing which screen is active (menu, level, cutscene, etc.). It needs a single place to ask “who handles this frame and this input?” **Why:** By reading `.size()` and `[nActive]` and calling `->Update()`, `->OnKey()`, `->OnMouse()`, etc. on that controller, the runner delegates every tick and every user action to the correct screen. It also reads the active controller’s name so the simulation or host can report which screen is running. **Mechanics:** Read `.size()` and `[nActive]`; call `->Update()`, `->OnKey()`, `->OnMouse()`, `->OnMouseDown()`, `->OnMouseUp()`, `->DoubleClick()`, `->Fire()`, `->GetControllerName()`. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** When the level ends (e.g. game over or cheat “back”), the game must leave the level screen and show the main menu. **Why:** The menu is not at a fixed index; it is the second-to-last controller in `vCnt`. Reading `.size()` is needed to compute that index so `nActive` can be set correctly and the user sees the menu instead of a stale or wrong screen. **Mechanics:** Read `.size()` when computing the menu index. |

### `nActive` (unsigned)

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | **Intent:** The runner must always talk to “the current screen” and never to the wrong one. **Why:** Reading `nActive` (and using it to index `vCnt`) is the only way to know which controller owns the current frame and input. Without it, the runner could not delegate updates and input to the right place. **Mechanics:** Read for active index and indexing into `vCnt` (controller count
comes from `vCnt.size()`). |
| MenuController | `game/controller/menu_controller.cc` | **Intent:** When the user leaves the menu (e.g. “Resume” or “Level 2”), the game must return to the screen they came from or chose, not a random one. **Why:** The menu stores the desired “resume” index; writing it into `nActive` is how the global game state is told which screen is now active, so the next frame runs the correct controller. **Mechanics:** Write `pGl->nActive = pMenuController->nResumePosition` when exiting the menu. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** When the player loses or otherwise leaves the level, they must see the main menu, not the level screen or a blank state. **Why:** Writing `nActive` to the menu’s index switches the global “current screen” to the menu. The runner will then send updates and input to the menu controller. **Mechanics:** Write `nActive` to the menu index (e.g. `vCnt.size() - 2`) when leaving the level. |

### `vLevelPointers` (vector of int)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The menu offers “Level 1”, “Level 2”, “Level 3” (or similar); choosing one must start the game at that level. **Why:** Each slot in the menu corresponds to an index in `vLevelPointers`; that index is the correct argument to `Restart()`. Reading `.at(0)`, `.at(1)`, `.at(2)` and passing to `Restart()` is how the menu tells the game “start at this level” rather than at a default. **Mechanics:** Read `.at(0)`, `.at(1)`, `.at(2)` and pass to `Restart()`. |

### `pMenu` (`smart_pointer` to MenuController)

Only accessed by DragonGameController (omitted).

### `pGraph` (`smart_pointer` to GraphicalInterface of Index)

| Component | File | Access |
|-----------|------|--------|
| EntityListController | `game/controller/basic_controllers.cc` | **Intent:** When the user changes the level list (e.g. level select), the screen must show the new state. **Why:** The controller does not own the display; the global graphics interface does. Calling `->RefreshAll()` is how it asks for a full redraw so the user sees the updated list. **Mechanics:** Call `->RefreshAll()`. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** During the level, the game must dim the screen when needed, show the cursor, and present the latest frame to the player. **Why:** Overlays (e.g. dimming rectangles), cursor drawing, and final present are all done through the same global graphics backend. The level needs that handle to keep the display correct and responsive. **Mechanics:** Call `->DrawRectangle()`, `->RefreshAll()`; pass to
`MouseCursor::DrawCursor()`. |
| MenuController | `game/controller/menu_controller.cc` | **Intent:** When the menu is shown or an option changes, the player must see the updated menu. **Why:** The menu does not own the window; it needs the global graphics interface to trigger a full refresh so the latest menu frame is on screen. **Mechanics:** Call `->RefreshAll()`. |
| BuyNowController | `game/controller/buy_now_controller.cc` | **Intent:** The buy-now / trial screen must show its current content (e.g. slimes, text) to the user. **Why:** Refreshing is done through the single global graphics handle so the trial screen’s updates are actually visible. **Mechanics:** Call `->RefreshAll()`. |

### `pDr` (`smart_pointer` to ScalingDrawer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** Menu option and hint text must appear at the right size on different resolutions. **Why:** The scaling drawer knows the current scale factor and how to draw in screen space; passing `pDr` to `Draw()` ensures menu text uses the same scaling as the rest of the game so it stays readable and aligned. **Mechanics:** Pass `pGl->pDr` to `Draw()` for option and hint text. |
| EntityListController | `game/controller/basic_controllers.cc` | **Intent:** The level-select (or similar) list must be laid out and drawn at the correct scale. **Why:** Layout depends on `nFactor` (e.g. rectangle sizes), and drawing must go through the same drawer so list entries match the game’s coordinate system. **Mechanics:** Read `->nFactor` for layout; call `->Draw()` for list entries. |
| BuyNowController | `game/controller/buy_now_controller.cc` | **Intent:** The buy-now screen must show animated slimes at the right scale. **Why:** The global scaling drawer is the single place that knows how to draw scaled content; the controller needs it so the slimes look correct on the trial screen. **Mechanics:** Call `->Draw()` for slime entities. |

### `pNum` (`smart_pointer` to NumberDrawer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** Menu entries and option labels must be drawn in the game’s standard font and scale. **Why:** MenuDisplay is given `pNum` at creation and uses it in `Draw()` so menu text matches the rest of the game. **Mechanics:** Call `->DrawWord()`, `->DrawColorWord()` for entries and labels. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** The level HUD must show score, level number, time, and labels in the game’s standard number/text style; the tutorial hint must use the same style. **Why:** There is one shared number drawer for consistent typography and scaling; the level and tutorial both need it so the HUD and hints look correct and readable. **Mechanics:** Call `->DrawNumber()`, `->DrawWord()`, `->DrawColorNumber()`, `->DrawColorWord()`; pass to tutorial entity. |
| Critters | `game/critters.cc` | **Intent:** Critters must show short text (e.g. damage numbers or labels) above or near themselves. **Why:** The global number drawer provides the single way to draw that text in the game’s font and scale; critters need it so feedback is visible and consistent. **Mechanics:** Call `->DrawWord()` (via `pAc->pGl->pNum`). |
| Fireball | `game/fireball.cc` | **Intent:** Fireballs may need labels or text near them (e.g. for debugging or feedback). **Why:** Access to the shared number drawer ensures that text matches the rest of the game’s drawing. **Mechanics:** Call `->DrawWord()`. |
| Entities | `game/entities.cc` | **Intent:** Generic visual entities must display text (and sometimes coloured text) as part of their presentation. **Why:** Using the global number drawer keeps text style and scaling consistent across all entities. **Mechanics:** Call `->DrawWord()`, `->DrawColorWord()`. |

### `pBigNum` (`smart_pointer` to NumberDrawer)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | **Intent:** Some level HUD elements need larger, more prominent text (e.g. messages or emphasis). **Why:** The “big number” drawer is the shared way to draw that; the level uses it so those elements stand out without duplicating font logic. **Mechanics:** Call `->DrawWord()`. |
| HighScoreShower / IntroTextShower (entities) | `game/entities.cc` | **Intent:** The intro and high-score screens must show score and high score in large, readable numbers so the player sees their progress at a glance. **Why:** The big-number drawer is dedicated to that role; these entities need it so the most important numbers on those screens are drawn in the right size and style. **Mechanics:** Call `->DrawWord()`, `->DrawNumber()` for score and high score. |

### `pFancyNum` (`smart_pointer` to FontWriter)

| Component | File | Access |
|-----------|------|--------|
| IntroTextShower | `game/entities.cc` | **Intent:** The intro screen must show stylised text (e.g. title or story) that looks different from the standard HUD. **Why:** The fancy font writer is the single place for that style; the intro needs it to measure and draw that text so it fits and looks correct. **Mechanics:** Call `->GetSize()`, `->DrawWord()`. |

### `pSndRaw` (`smart_pointer` to SoundInterface of Index)

Only accessed by DragonGameController (omitted).

### `pSnd` (`smart_pointer` to SoundInterfaceProxy)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The menu must play feedback sounds (e.g. beeps on selection), let the user turn sound on/off, and show the current sound state in the menu. **Why:** All sound goes through the global proxy (which respects mute); the menu needs it to play its sounds and to read/write the effective sound state so the “sound on/off” option and label stay correct. **Mechanics:** Call `->PlaySound()`, `->Toggle()`, `->Get()`; sync `sbSoundOn` with `pSnd->Get()`. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** The level must play audio cues (e.g. timer warnings, level events) so the player gets feedback during play. **Why:** The level does not own the audio device; it needs the global sound proxy so those cues are heard and respect the user’s sound setting. **Mechanics:** Call `->PlaySound()`. |
| Basic controllers | `game/controller/basic_controllers.cc` | **Intent:** Start screen, cutscenes, and score screens must play sounds (e.g. “start game”, beep/boop on keypress) so the experience feels responsive. **Why:** Those screens are not audio owners; they need the shared proxy to trigger the right sounds at the right time. **Mechanics:** Call `->PlaySound()`. |
| Critters | `game/critters.cc` | **Intent:** Critters must play sounds for actions (death, steps, hits, spawn, etc.) so the player gets audio feedback for what is happening. **Why:** Critters are game entities, not sound owners; they need the global proxy so their sounds play through the same pipeline and respect mute. **Mechanics:** Call `->PlaySound()` (via `pAc->pGl`, `pAdv->pGl`). |
| CritterGenerators | `game/critter_generators.cc` | **Intent:** When a critter is spawned (e.g. princess arriving), a sound should play to signal the event. **Why:** Generators do not own audio; they need the global proxy so spawn sounds are heard and consistent. **Mechanics:** Call `->PlaySound()`. |
| Dragon | `game/dragon.cc` | **Intent:** The dragon must play sounds for pickups, shooting, capturing princesses, winning, etc., so the player has clear audio feedback. **Why:** The dragon is a game entity; it needs the global sound proxy so its sounds play and respect the user’s sound setting. **Mechanics:** Call `->PlaySound()`. |
| Fireball | `game/fireball.cc` | **Intent:** Fireballs must play sounds for death, explosion, etc., so impacts and hits are audible. **Why:** Fireballs need the shared sound proxy so those effects are played through the same system as the rest of the game. **Mechanics:** Call `->PlaySound()`. |

### `vLvl` (vector of LevelLayout)

Only accessed by DragonGameController (omitted).

### `nScore` (int)

| Component | File | Access |
|-----------|------|--------|
| Simulation | `simulation/simulation.cc` | **Intent:** Tests need to observe the current score to verify game behaviour (e.g. score increases after kills). **Why:** Reading `nScore` is the only way for the simulation to assert on score without depending on internal controller details. **Mechanics:** Read for logging. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** The level HUD must show the player’s current score so they see progress during play. **Why:** The score is stored globally; the level needs to read it each frame to draw the correct number. **Mechanics:** Read for HUD drawing. |
| Entities | `game/entities.cc` | **Intent:** When the player earns points (e.g. killing critters), the global score must increase; the high-score screen must show the current score. **Why:** Score is a single global value; entities that award points must write to it, and the high-score display must read it so the player sees accurate feedback and results. **Mechanics:** Read in HighScoreShower; write `pGl->nScore +=` when awarding points. |
| DragonScoreController (basic_controllers) | `game/controller/basic_controllers.cc` | **Intent:** At the end of a run, the game must show the score and update the saved high score if this run beat it. **Why:** The score screen does not own score or high score; it must read both to display and compare, and write `nHighScore` when the run is a new record so progress is persisted. **Mechanics:** Read to compare and to write to output stream; write `pGl->nHighScore = pGl->nScore` when score exceeds high score. |

### `nHighScore` (int)

| Component | File | Access |
|-----------|------|--------|
| Simulation | `simulation/simulation.cc` | **Intent:** Tests may need to check that the high score is updated (e.g. after a run that beats the previous record). **Why:** Reading `nHighScore` lets the simulation assert on persistence without touching file or controller internals. **Mechanics:** Read for logging. |
| CritterGenerators | `game/critter_generators.cc` | **Intent:** The game may treat “first time ever” (player has never had a score) differently—e.g. different spawns or tutorials. **Why:** Comparing `nHighScore == 0` is the way to detect that; generators need it so they can adapt behaviour for new players. **Mechanics:** Read `pBc->pGl->nHighScore == 0`. |
| Entities | `game/entities.cc` | **Intent:** The intro/high-score screen must show the player’s best score so they see their record. **Why:** High score is stored globally; HighScoreShower reads it so the display is accurate. **Mechanics:** Read for drawing in HighScoreShower. |
| DragonScoreController (basic_controllers) | `game/controller/basic_controllers.cc` | **Intent:** When a run ends, if the score beats the previous high score, that record must be updated and persisted. **Why:** The score screen is the place that knows “run just ended”; it must read both scores to compare and write `nHighScore` when appropriate so the player’s best is saved. **Mechanics:** Read and write when comparing with `nScore`. |

### `bAngry` (bool)

| Component | File | Access |
|-----------|------|--------|
| Critters | `game/critters.cc` | **Intent:** When the player does something that “angers” the game (e.g. attacking a princess), the whole game should enter “angry mode” so other systems can react (different spawns, tone, etc.). **Why:** That state must be global and visible to generators and other logic; critters set it when they detect the triggering action. **Mechanics:** Write `pAc->pGl->bAngry = true`. |
| CritterGenerators | `game/critter_generators.cc` | **Intent:** In angry mode, spawns or critter behaviour may change (e.g. different types or aggression). **Why:** Generators need to read the current angry flag so they create the right critters for the current game state. **Mechanics:** Read `pBc->pGl->bAngry` and pass into constructors. |

### `snProgress` (SavableVariable of int)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The menu must only offer options the player has unlocked (e.g. Level 2 only after completing Level 1). **Why:** Progress is stored in `snProgress` and persisted; the menu reads it so it can enable/disable or show/hide entries and avoid letting the player jump to levels they have not reached. **Mechanics:** Read `.Get()` to decide menu state. |

### `sbSoundOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The user must be able to turn sound on/off from the menu, and that choice must persist and be shown correctly next time. **Why:** The menu is the place that handles the toggle; it must write the new value so it is saved, and read it to show the current state (e.g. “Sound: On”) so the label is accurate. **Mechanics:** Write `.Set(...)` when the user toggles; read `.Get()` for menu label. |

### `sbMusicOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The user must be able to turn background music on/off from the menu, and that choice must persist and be reflected in the menu label. **Why:** The menu handles the toggle and needs to write the value for persistence and read it so the “Music: On/Off” label is correct. **Mechanics:** Write `.Set(...)` when the user toggles; read `.Get()` for menu label. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** During the level, music (or certain music-related behaviour) should only play or apply when the user has music enabled. **Why:** The level does not own the setting; it reads `sbMusicOn` so it can respect the user’s preference (e.g. in ghost-time or blink logic). **Mechanics:** Read `.Get()` in gameplay logic. |

### `sbTutorialOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The user must be able to enable/disable the in-level tutorial from the menu, and the menu must show the current state. **Why:** The menu handles the toggle and needs to read the value so the label (e.g. “Tutorial: On”) is correct and the choice is persisted. **Mechanics:** Call `BoolToggle(pGl->sbTutorialOn)`; read `.Get()` for menu label. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** The tutorial hint entity must show or hide based on the user’s saved preference. **Why:** The level creates the tutorial entity and must give it access to the current setting (e.g. via pointer) so it can decide in real time whether to display hints without the level having to poll. **Mechanics:** Read `.GetConstPointer()` and pass to entity. |

### `sbFullScreen` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The user must be able to switch between fullscreen and windowed from the menu, and the menu must reflect the current mode (e.g. show “Exit” only when fullscreen). **Why:** The menu owns the fullscreen toggle; it must write the new value so it persists and read it so the menu options and labels match the actual state. **Mechanics:** Call `BoolToggle()`; read `.Get()`. |

### `sbCheatsOn` (SavableVariable of bool)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The user must be able to turn cheats on/off from the menu (e.g. for testing or fun), and the menu must show the current state. **Why:** The menu handles the toggle and needs to read the value so the “Cheats: On/Off” label is correct and the choice is persisted. **Mechanics:** Call `BoolToggle()`; read `.Get()` for menu. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** Cheat actions in the level (e.g. “skip to next level”) should only work when cheats are enabled. **Why:** The level is where those actions are triggered; it must read the global flag so it only performs cheat behaviour when the user has turned cheats on. **Mechanics:** Read `.Get()`. |
| Basic controllers | `game/controller/basic_controllers.cc` | **Intent:** The cheat key (e.g. backslash) should only do something when cheats are enabled, so normal play is not accidentally triggered. **Why:** Key handling is in the controllers; they must read the flag so the cheat key is ignored when cheats are off. **Mechanics:** Read `.Get()` when handling key input. |

### `sbCheatsUnlocked` (SavableVariable of bool)

Only accessed by DragonGameController (omitted).

### `rBound` (Rectangle)

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | **Intent:** UI elements (e.g. tutorial hint) must be placed relative to the current game window size so they appear in the right place on any resolution. **Why:** The global bounds rectangle holds the game’s logical screen size; the level reads width and height so it can position overlays (e.g. centre bottom) without hardcoding coordinates. **Mechanics:** Read `pGl->rBound.sz.x`, `pGl->rBound.sz.y`. |

### `plr` (BackgroundMusicPlayer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** When the user toggles “music on/off” in the menu, the background music must actually mute or unmute, and the menu label must reflect the real state (in case it differs from the saved setting). **Why:** The music player is global; the menu needs to call `ToggleOff()` to change playback and read `.bOff` so it can sync the displayed “Music: On/Off” with what the user hears. **Mechanics:** Call `.ToggleOff()`; read `.bOff`. |
| SoundControls (entities) | `game/entities.cc` | **Intent:** Start and score screens must start or stop the correct background music so the mood matches the screen. **Why:** SoundControls entities are given a reference to the global music player at creation; in `Update()` they call `SwitchTheme()` or `StopMusic()` so music plays or stops when that screen is active. **Mechanics:** Call `plr.SwitchTheme()`, `plr.StopMusic()`. |

### `lsBonusesToCarryOver` (list of `smart_pointer` to TimedFireballBonus)

| Component | File | Access |
|-----------|------|--------|
| Dragon | `game/dragon.cc` | **Intent:** Timed fireball bonuses earned at the end of a level should carry over to the next level so the player keeps their reward. **Why:** The dragon is the entity that knows when a level is completed and what bonuses were earned; it must push those onto the global list so the next level can consume them, and clear or iterate as needed when applying or starting the next level. **Mechanics:** Call `.push_back()`, `.clear()`; read `.begin()`, `.end()` to iterate. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** When a level starts or is reset, any stale “carried over” bonuses from a previous run should be cleared so the level starts in a clean state. **Why:** The level is the place that owns “level start”; it must clear the global list so old bonuses do not leak into the new level or cause double application. **Mechanics:** Call `.clear()`. |

### `pWrp` (TowerDataWrap pointer)

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** When the user chooses “Exit” from the menu, the game must shut down cleanly (e.g. trigger the platform’s exit path). **Why:** The menu does not own the application lifecycle; it needs the wrapper’s exit hook (e.g. `pExitProgram`) so the user’s choice actually quits the process. **Mechanics:** Read `->pExitProgram` in `Exit()`. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** Level layout or rendering may depend on the actual window/resolution (e.g. scaling, letterboxing). **Why:** The wrapper holds the real resolution; the level reads it so it can adapt drawing or layout to the current display. **Mechanics:** Read `->szActualRez`. |
| DragonScoreController (basic_controllers) | `game/controller/basic_controllers.cc` | **Intent:** The high score (or similar) must be saved to a file so it persists across runs. **Why:** The score screen does not know where the game’s data directory or file root is; it needs the wrapper’s `GetFilePath()` so it can open the correct file for writing. **Mechanics:** Call `->GetFilePath()`. |

### `pSelf` (DragonGameController pointer)

Only accessed by DragonGameController (omitted).

---

## Public methods

### `StartUp(DragonGameController *pSelf)`

| Component | File | Access |
|-----------|------|--------|
| DragonGameRunner | `game/dragon_game_runner.cc` | **Intent:** After creating the controller, the game must be in a runnable state with the first screen (e.g. menu or start screen) ready. **Why:** The controller does not fully initialise in its constructor; `StartUp` builds the screen stack and wiring. The runner is responsible for bootstrapping, so it must call `StartUp` once the controller exists so the first frame has a valid active screen. **Mechanics:** Call `pCnt->StartUp(pCnt.get())` after creating the controller. |

### `Next()`

| Component | File | Access |
|-----------|------|--------|
| Dragon | `game/dragon.cc` | **Intent:** When the player wins the level (e.g. all princesses captured), the game must leave the level and show the next screen (e.g. score or next level). **Why:** The dragon does not control the screen stack; it must ask the global controller to advance so the runner will switch to the correct next controller on the next frame. **Mechanics:** Call `pAd->pGl->Next()`. |
| Basic controllers | `game/controller/basic_controllers.cc` | **Intent:** When the user finishes a screen (start screen, cutscene, score screen, level list) or clicks “continue” / “next”, the game must show the following screen. **Why:** Each of those controllers only knows “the user is done here”; they need the global `Next()` to advance the active index so the runner picks up the next controller. **Mechanics:** Call `pGl->Next()` from start screen, cutscene, score screen, entity list, etc. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** When the user triggers a cheat (e.g. “skip to next level”), the game must jump to the next level without playing through. **Why:** The level does not manage the controller stack; it calls `Next()` so the global state advances and the runner switches to the next level controller. **Mechanics:** Call `pGl->Next()` when cheat is used. |

### `Restart(int nActive_)`

| Component | File | Access |
|-----------|------|--------|
| MenuController | `game/controller/menu_controller.cc` | **Intent:** When the user picks “Resume” or “Level 1/2/3” from the menu, the game must (re)start from that point—rebuilding the screen stack and jumping to the right level or resume position. **Why:** Only the global controller can clear and rebuild the controller list and set the active index; the menu must call `Restart()` with the right argument (or none for resume) so the next frame runs the correct screen. **Mechanics:** Call `pGl->Restart()` or `Restart(vLevelPointers.at(0/1/2))`. |

### `Menu()`

| Component | File | Access |
|-----------|------|--------|
| EntityListController | `game/controller/basic_controllers.cc` | **Intent:** When the user chooses “back to menu” from the level-select (or similar) list, the game must show the main menu. **Why:** The entity list controller does not own the screen stack; it must call the global `Menu()` so the active index is set to the menu and the runner switches to the menu controller. **Mechanics:** Call `pGl->Menu()`. |
| LevelController | `game/controller/level_controller.cc` | **Intent:** When the user pauses or quits the level, they must return to the main menu instead of staying in the level or seeing a blank state. **Why:** The level does not change the global active index; it calls `Menu()` so the global state switches to the menu and the next frame runs the menu controller. **Mechanics:** Call `pGl->Menu()`. |

### `GetImg(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| Level | `game/level.cc` | **Intent:** The level’s road (and similar tiles) must be drawn with the correct artwork. **Why:** Images are loaded and keyed globally; the level does not own the asset pool. It needs `GetImg("road")` (and use of the returned reference) so it can draw the road tiles without duplicating load logic or asset keys. **Mechanics:** Call `pAd->pGl->GetImg("road")` and use the returned reference. |
| Fireball | `game/fireball.cc` | **Intent:** Some fireball drawing may need a placeholder or “empty” image (e.g. for slots or fallback). **Why:** The fireball code does not own images; it needs the global lookup so it can get the correct placeholder without hardcoding paths or keys. **Mechanics:** Call `pAd->pGl->GetImg("empty")`. |

### `GetImgSeq(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| LevelController | `game/controller/level_controller.cc` | **Intent:** The level needs the right animation sequences for the HUD (bonus icons, claw cursor, dragon, timer) and for entities so the level looks and behaves correctly. **Why:** Sequences are loaded and keyed globally; the level does not own the asset pool. It needs `GetImgSeq` so it can request by name and use the same assets as the rest of the game. **Mechanics:** Call `GetImgSeq(...)` for bonus, claw, dragon, timer, etc. |
| MenuController | `game/controller/menu_controller.cc` | **Intent:** The menu must show a cursor (e.g. claw) that is animated. **Why:** The cursor art is a sequence keyed globally; the menu needs it at construction time so the cursor is ready when the menu is shown. **Mechanics:** Call `pGl_->GetImgSeq("claw")` in ctor. |
| Basic controllers | `game/controller/basic_controllers.cc` | **Intent:** Cutscenes must play the correct run and chase animations for the characters. **Why:** Those sequences are keyed by name in the global preloader; the cutscene controller needs them so it can play the right animations without duplicating asset loading. **Mechanics:** Call `pGl_->GetImgSeq(sRun)`, `GetImgSeq(sChase)`. |
| BuyNowController | `game/controller/buy_now_controller.cc` | **Intent:** The buy-now screen must show the slime animation so the trial screen looks appealing. **Why:** The slime sequence is a global asset; the controller needs `GetImgSeq("slime")` so it can display it without owning the asset pool. **Mechanics:** Call `pGl->GetImgSeq("slime")`. |
| Critters | `game/critters.cc` | **Intent:** Each critter type and state (death, walk, burn, spawn, etc.) must show the correct sprite sequence so the game looks right and feedback is clear. **Why:** All sequences live in the global preloader; critters need to resolve by name so they get the right art for their type and state without each critter loading assets. **Mechanics:** Call `pAc->pGl->GetImgSeq(...)`, `pAdv->pGl->GetImgSeq(...)` for many sequences. |
| CritterGenerators | `game/critter_generators.cc` | **Intent:** When creating critters (golem, princess, mage, trader, skeleton, etc.), the correct image sequence must be assigned so they render properly. **Why:** Generators do not own art; they need the global lookup by name so they can pass the right sequence into critter constructors. **Mechanics:** Call `pBc->pGl->GetImgSeq(...)`, `pAdv->pGl->GetImgSeq(...)`. |
| Fireball | `game/fireball.cc` | **Intent:** Fireballs and their effects (laser, explosion, bonus) must use the correct graphics for size and type. **Why:** Those sequences are keyed globally; the fireball code needs `GetImgSeq` so it can request the right art for the current fireball and effects without duplicating asset logic. **Mechanics:** Call `pBc->pGl->GetImgSeq(...)`, `pAd->pGl->GetImgSeq(...)` for fireball, laser, explosion, bonus. |

### `GetSnd(std::string key)`

| Component | File | Access |
|-----------|------|--------|
| MenuController, LevelController, Basic controllers, Critters, CritterGenerators, Dragon, Fireball | Various | **Intent:** Each of these needs to play a specific sound (menu beeps, level cues, critter/dragon/fireball effects) by a logical name (e.g. `"death"`, `"pickup"`) rather than by raw index. **Why:** Sounds are loaded and keyed globally; none of these components own the sound pool. They need `GetSnd(key)` to resolve the name to an index, then pass that to `pSnd->PlaySound()` so the right sound plays through the shared audio path. **Mechanics:** Call `pGl->GetSnd("...")` and pass result to `pGl->pSnd->PlaySound()`. |

### `GetSndSeq(std::string key)`

Only used internally by DragonGameController (e.g. for menu/game-over sound entities via the preloader). No other classes call it (omitted).

---

## Summary by component

- **DragonGameRunner** – vCnt, nActive (read / delegate to active controller); StartUp (call to initialise).
- **Simulation** – nScore, nHighScore (read for logging).
- **LevelController** – vCnt, nActive (read/write for screen switching); pGraph, pNum, pBigNum, pSnd, rBound, sbMusicOn, sbTutorialOn, sbCheatsOn, lsBonusesToCarryOver, pWrp; Next, Menu, GetImgSeq, GetSnd (calls).
- **MenuController** – nActive, vLevelPointers, pGraph, pDr, pNum, pSnd, snProgress, sbSoundOn, sbMusicOn, sbTutorialOn, sbFullScreen, sbCheatsOn, plr, pWrp; Restart, GetImgSeq, GetSnd (calls).
- **Basic controllers** – pGraph, pDr, pSnd, nScore, nHighScore, sbCheatsOn, pWrp; Next, Menu, GetImgSeq, GetSnd (calls).
- **BuyNowController** – pGraph, pDr, GetImgSeq (calls).
- **Dragon** – pSnd, lsBonusesToCarryOver; Next, GetImgSeq, GetSnd (calls).
- **Critters / CritterGenerators** – pNum, pSnd, nHighScore, bAngry; GetImgSeq, GetSnd (calls).
- **Fireball** – pNum, pSnd; GetImg, GetImgSeq, GetSnd (calls).
- **Entities** – pBigNum, pFancyNum, pNum, nScore, nHighScore (draw and update score); plr (SoundControls: switch/stop music).
- **Level** – GetImg (call for road image).
