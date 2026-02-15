# Tower defense finalize move — implementation summary

## Step 1: Move declarations to core.h and level.h

**What was done:**
- **core.h:** Added TowerDataWrap fwd, MenuController fwd, BackgroundMusicPlayer + BG_* enums, TwrGlobalController fwd, GameController fwd, GetProgramInfo(). Did not add level.h or screen_controllers.h (to avoid cycles). Did not add fireball.h (needs Entity from entities.h).
- **level.h:** Added struct AdvancedController; and void ReadLevels(std::string sFile, Rectangle rBound, LevelStorage &vLvl) after the LevelStorage typedef.
- **tower_defense.h:** Now holds fireball.h include, full GameController, TwrGlobalController, TowerDataWrap, TowerGameGlobalController, FancyRoad (and critters/screen_controllers/critter_generators/dragon/gameplay includes). Removed duplicate declarations that moved to core.h/level.h.
- **core.cpp:** Added #include "tower_defense.h" so it sees full types when we move implementations in step 2.

**Issues:**
- Circular dependency: core.h -> level.h -> entities.h -> core.h prevented adding level.h to core.h.
- FancyRoad in level.h with member SSP<AdvancedController> required AdvancedController to be complete (SSP destructor/copy); level.h cannot include gameplay.h when included from core.h.
- ReadLevels(..., LevelStorage &vLvl) was declared before LevelStorage typedef in level.h.
- Including fireball.h from core.h broke entities.cpp: fireball.h needs Entity, AnimationOnce, Critter (from entities) which are not yet visible when core.h is included from entities.h.
- GameController in core.h with member SSP<TwrGlobalController> pGl forced instantiation of SSP<TwrGlobalController> in every TU that included core.h (e.g. entities.cpp, level.cpp), so TwrGlobalController had to be complete there.

**How you solved them:**
- Did not add level.h to core.h; kept TwrGlobalController, TowerDataWrap, TowerGameGlobalController in tower_defense.h.
- Removed FancyRoad from level.h; kept FancyRoad in tower_defense.h (after gameplay.h).
- Moved ReadLevels declaration to after the LevelStorage typedef in level.h.
- Removed fireball.h from core.h; added #include "fireball.h" in tower_defense.h before TwrGlobalController.
- Moved full struct GameController to tower_defense.h; left only "struct GameController;" in core.h so no TU needs full TwrGlobalController just from including core.h.

**What you learned:**
- Include DAG (core -> level -> entities -> core) blocks moving LevelStorage-dependent types into core.h without a new header.
- SSP<T> and similar smart pointers require T to be a complete type when the containing struct is destroyed or copied, so forward-decl-only is not enough for member types.

**Future avoidance:**
- Before moving a type into a header, grep for SSP</ASSP members and ensure their template args are complete in that header or included after.
- Check which headers include the target header; if a cycle appears (A includes B includes A), either break the cycle or keep the type in a header that is included after the cycle is resolved.

---

## Step 2: Move core implementations to core.cpp

**What was done:**
- Appended to core.cpp: pWr, nSlimeMax; DrawStuff; sFullScreenPath; TwrGlobalController ctor, StartUp, Next, Restart, Menu; AreWeFullScreen, GetProgramInfo, GetGlobalController; TowerGameGlobalController ctor, dtor; TowerDataWrap ctor; TowerGameGlobalController Update, KeyDown, KeyUp, MouseMove, MouseDown, MouseUp, DoubleClick, Fire.
- Removed those blocks from tower_defense.cpp. Left in tower_defense.cpp: ReadLevels (definition; moves in step 3); all gameplay implementations (BackgroundMusicPlayer methods, TrackballTracker, BonusScore, SoundControls, PositionTracker, AdvancedController, AdNumberDrawer, BonusDrawer, HighScoreShower::Draw, IntroTextShower::Draw).

**Issues:**
- First removal pass accidentally removed gameplay implementations (TrackballTracker, BonusScore, SoundControls, PositionTracker, BackgroundMusicPlayer::ToggleOff/SwitchTheme/StopMusic) from tower_defense.cpp; link errors for undefined references.
- PowerShell line-range deletion removed too much (included TwrGlobalController block correctly but an earlier search_replace had already removed the gameplay block).

**How you solved them:**
- Re-added to tower_defense.cpp: BackgroundMusicPlayer::ToggleOff, SwitchTheme, StopMusic; TrackballTracker (all methods); BonusScore ctor; SoundControls::Update; PositionTracker (On, Off, Update, UpdateMouse, UpdateLastDownPosition, GetFlightDirection). Step 2 only moves *core* impl; gameplay moves in step 4.

**What you learned:**
- When removing by line range, double-check that the range matches the plan (core vs gameplay). Grep for symbol names to confirm what lives where before bulk delete.

**Future avoidance:**
- When moving "core" vs "gameplay" blocks, list exact symbols for each step and remove only those; avoid one big range that spans both.

---

## Step 3: Move FancyRoad and ReadLevels to level

**What was done:**
- level.cpp: Added #include "tower_defense.h" (so FancyRoad and AdvancedController are visible). Appended FancyRoad::Draw and ReadLevels implementations at end of file.
- tower_defense.cpp: Removed FancyRoad::Draw and ReadLevels (both blocks).

**Issues:**
- Initially added #include "gameplay.h" before tower_defense.h in level.cpp; build failed because gameplay.h expects Castle, Dragon, BasicController, KnightGenerator, etc., which are not yet in scope when level.h is included first (level.h → entities.h → core.h; gameplay.h does not get the full game include order).
- FancyRoad::Draw uses pAd->pGl->pr["road"]; with only a forward decl of AdvancedController the compiler reported "struct AdvancedController has no member named 'pGl'".

**How you solved them:**
- Removed #include "gameplay.h" from level.cpp. Kept only #include "tower_defense.h" so level.cpp sees FancyRoad and the full AdvancedController (and all other types) via the same include order as before. Plan’s “level.cpp includes gameplay.h” is satisfied indirectly through tower_defense.h until step 5 when game.h replaces tower_defense.h.

**What you learned:**
- Including a “middle” header (gameplay.h) before the “hub” (tower_defense.h) in a TU that previously only included the hub breaks the expected include order and causes missing-type errors.
- For step 3 we only move implementations; FancyRoad struct stays in tower_defense.h until step 5.

**Future avoidance:**
- When a .cpp needs a type that lives in a hub header, include the hub there rather than a subset of its dependencies to preserve the working include order until the hub is removed.

---

## Step 4: Create gameplay.cpp and move gameplay implementations

**What was done:**
- Added gameplay.cpp to game target in CMakeLists.txt. Created gameplay.cpp with #include "tower_defense.h" and moved from tower_defense.cpp: BackgroundMusicPlayer::ToggleOff, SwitchTheme, StopMusic; TrackballTracker (ctor, Update, IsTrigger, GetAvMovement, GetDerivative); BonusScore ctor, Update, Draw; SoundControls::Update; PositionTracker (On, Off, Update, UpdateMouse, UpdateLastDownPosition, GetFlightDirection); AdvancedController copy ctor and main ctor; AdNumberDrawer and BonusDrawer (structs + impl); AdvancedController OnKey, OnMouse, OnMouseDown, OnMouseUp, Fire, GetCompletionRate, MegaGeneration (x2), Update; HighScoreShower::Draw; IntroTextShower::Draw.
- Replaced tower_defense.cpp content with only #include "tower_defense.h".

**Issues:**
- None. Single copy of each symbol; build passed.

**How you solved them:**
- N/A.

**What you learned:**
- Moving an entire TU’s implementation into a new .cpp in one go (and emptying the old one) avoids duplicate-definition mistakes from partial moves.

**Future avoidance:**
- For “create new .cpp and move all X implementations,” write the new file with the full block first, then replace the old file’s content with the minimal include so both files compile and only the new file defines symbols.

---

## Step 5: Add game.h, switch includes, delete tower_defense files

**What was done:**
- Created game.h with: includes (common, core, entities, level, tutorial, fireball), then GameController and TwrGlobalController (full), TowerDataWrap fwd, then screen_controllers, critters, critter_generators, dragon, gameplay, then FancyRoad, TowerDataWrap (full), TowerGameGlobalController. So game.h holds the former tower_defense.h content and the single “include the whole game” list.
- Replaced #include "tower_defense.h" with #include "game.h" in core.cpp, gameplay.cpp, level.cpp, dragon.cpp, critters.cpp, critter_generators.cpp, fireball.cpp, screen_controllers.cpp.
- Removed tower_defense.cpp and tower_defense.h from the build; deleted both files.

**Issues:**
- Include order: screen_controllers.h inherits from GameController and uses pGl->Next() in inline OnMouseDown. When game.h was included from core.cpp (which includes core.h first), only the forward decl of TwrGlobalController from core.h was visible where screen_controllers.h was parsed, so “invalid use of incomplete type” for TwrGlobalController.
- Link: undefined reference to BasicController::OnMouseDown after moving the inline to the .cpp (first add of the def was missed).

**How you solved them:**
- Defined GameController and TwrGlobalController in game.h before #include "screen_controllers.h", and used a forward decl of TowerDataWrap before TwrGlobalController. Kept critters/critter_generators/dragon/gameplay includes after TwrGlobalController so FancyRoad and the rest stay in the right order.
- Moved the two inline bodies (BasicController::OnMouseDown and AlmostBasicController::OnMouseDown that only call pGl->Next()) from screen_controllers.h to declarations in the header and definitions in screen_controllers.cpp, so the header no longer needs the complete TwrGlobalController type.
- Added the missing BasicController::OnMouseDown(Point pPos) definition in screen_controllers.cpp.

**What you learned:**
- A hub header (game.h) that (1) includes a set of module headers and (2) defines types that depend on those headers must order “type definitions that other headers need” before the includes that use them (e.g. GameController/TwrGlobalController before screen_controllers.h). Inline methods in those included headers that use the hub types require the full type; moving such methods to the .cpp avoids the need for the full type in the header.
- When adding a new out-of-line definition, confirm it is present in the file and that the project was rebuilt so the linker sees it.

**Future avoidance:**
- Before deleting the old hub header, replicate its include order and type order in the new hub (game.h). If an included header has inline methods that use a type only forward-declared in an earlier header, move those methods to the .cpp or ensure the full type is visible before that header is included.

---

## Step 6: Build and fix

**What was done:**
- Ran full build: `cd c:\coding\dragongame_git\build; mingw32-make`. Build succeeded with no duplicate symbols, missing includes, or link errors.

**Issues:**
- None. All fixes were applied in Step 5 (game.h order, OnMouseDown moved to screen_controllers.cpp, BasicController::OnMouseDown definition added).

**How you solved them:**
- N/A.

**What you learned:**
- Final “build and fix” step can be a no-op if the previous step’s fixes are complete.

**Future avoidance:**
- Run the full build after each step so Step 6 is only a final confirmation.
