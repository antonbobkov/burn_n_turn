---
name: Tower defense finalize move
overview: Move all content out of tower_defense.h and tower_defense.cpp into existing modules (core, level, gameplay). Add one include-only header (game.h). Then delete tower_defense.h and tower_defense.cpp.
todos: []
isProject: false
---

# Finalize tower_defense split — delete both files

**Goal:** Nothing remains in tower_defense.h or tower_defense.cpp. At the end **both files are deleted**. One new file only: **game.h** (include-only) so the five .cpp callers do not each need 10+ includes.

---

## Where everything goes (nothing stays in tower_defense)


| Content                                                                                                                                                                                              | Destination                                                                                                                             |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------- |
| BackgroundMusicPlayer + BG_* enums                                                                                                                                                                   | **core.h** (before TwrGlobalController)                                                                                                 |
| GameController, TwrGlobalController, TowerDataWrap, TowerGameGlobalController, GetProgramInfo() decl                                                                                                 | **core.h** (core.h adds #include level.h, screen_controllers.h, fireball.h; does not include gameplay.h)                                |
| FancyRoad (struct)                                                                                                                                                                                   | **level.h** (forward decl AdvancedController)                                                                                           |
| FancyRoad::Draw                                                                                                                                                                                      | **level.cpp** (#include "gameplay.h")                                                                                                   |
| ReadLevels                                                                                                                                                                                           | **level.h** decl + **level.cpp** defn                                                                                                   |
| pWr, nSlimeMax, DrawStuff, sFullScreenPath, TwrGlobalController impl, TowerDataWrap ctor, TowerGameGlobalController impl, ReadLevels call site, AreWeFullScreen, GetProgramInfo, GetGlobalController | **core.cpp** (#include "game.h" or core.h + deps)                                                                                       |
| BackgroundMusicPlayer methods, TrackballTracker, BonusScore, SoundControls, PositionTracker, AdvancedController, AdNumberDrawer, BonusDrawer, HighScoreShower::Draw, IntroTextShower::Draw           | **gameplay.cpp** (#include "game.h")                                                                                                    |
| Single include for whole game                                                                                                                                                                        | **game.h** (new): common, core, entities, level, tutorial, fireball, critters, screen_controllers, critter_generators, dragon, gameplay |


---

## Implementation summary doc (while implementing)

While executing the plan, maintain a summary document (e.g. `.cursor/plans/tower_defense_finalize_move_summary.md`). **After completing each step**, add a section for that step with:

- **What was done** — Brief list of edits (files changed, symbols moved).
- **Issues** — Any build failures, missing symbols, ODR errors, include order problems, or other surprises.
- **How you solved them** — Describe fix (e.g. added include, made function inline, reordered declarations).
- **What you learned** — One or two takeaways.
- **(MOST IMPORTANT) Future avoidance** — What to do next time to avoid this (e.g. grep for all definitions of a symbol before removing from the old file).

This gives a repeatable record for similar refactors.

---

## Lessons from the original split

The [implementation summary](.cursor/plans/tower_defense_split_implementation_summary.md) Apply when moving so nothing stays in tower_defense:

- **Include order** — BackgroundMusicPlayer then TwrGlobalController in core.h; core.h does not include gameplay.h. FancyRoad in level.h with forward decl AdvancedController; level.cpp includes gameplay.h for FancyRoad::Draw. In game.h, critter_generators.h before gameplay.h.
- **ODR** — Header-defined helpers used in multiple TUs must be **inline**.
- **Duplicate definitions** — Before removing from tower_defense.cpp, **grep for all definitions** of that symbol (e.g. SkellyGenerator::Update).
- **Build** — PowerShell: `;` not `&&`. Build **once at the end of each step**.

---

**Include order:** BackgroundMusicPlayer then TwrGlobalController in core.h; core.h does not include gameplay.h. FancyRoad in level.h with forward decl AdvancedController; level.cpp includes gameplay.h for FancyRoad::Draw.

---

## Current state vs plan

The [original plan](.cursor/plans/tower_defense_file_split_de854034.plan.md) chose **Grouping 2** (by type). New modules exist (common, core, entities, level, tutorial, screen_controllers, fireball, critters, critter_generators, dragon, gameplay.h), but:

- **core**: GameController, TwrGlobalController, TowerDataWrap, TowerGameGlobalController are still in [tower_defense.h](game/tower_defense.h); implementation summary says they were left there as the hub. **Implementations** (TwrGlobalController ctor/StartUp/Next/Restart/Menu, TowerDataWrap ctor, TowerGameGlobalController, DrawStuff, ReadLevels, GetProgramInfo, GetGlobalController, AreWeFullScreen, pWr, nSlimeMax) are still in tower_defense.cpp.
- **level**: FancyRoad (and FancyRoad::Draw) are in tower_defense.h/cpp; they move to level.h/level.cpp with level.cpp including gameplay.h.
- **gameplay**: BackgroundMusicPlayer is in tower_defense.h (must be before TwrGlobalController). **gameplay.cpp** was not added; all gameplay **implementations** (TrackballTracker, PositionTracker, BonusScore, SoundControls, AdvancedController, HighScoreShower::Draw, IntroTextShower::Draw, AdNumberDrawer, BonusDrawer) are in tower_defense.cpp.

Additional symbols in tower_defense.cpp not in the original plan tables:

- `pWr`, `nSlimeMax` (declared in common.h) → define in **core.cpp**.
- `DrawStuff`, `sFullScreenPath`, `AreWeFullScreen`, `ReadLevels`, `GetProgramInfo`, `GetGlobalController` → **core.cpp** (or ReadLevels in level.cpp; declare in level.h).
- `AdNumberDrawer`, `BonusDrawer` → move to **gameplay.cpp** as file-local structs with AdvancedController.

---

## Implementation summary

**Goal:** Move all declarations to core.h/level.h and all implementations to core.cpp/level.cpp/gameplay.cpp. Add game.h. Then delete tower_defense.h and tower_defense.cpp. Nothing stays in tower_defense.


| Target module    | In tower_defense.cpp (implementations)                                                                                                                                                                                                                                                                                        | Action                                                                                                                                                                             |
| ---------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **core.cpp**     | pWr, nSlimeMax; DrawStuff, sFullScreenPath; TwrGlobalController ctor, StartUp, Next, Restart, Menu; TowerDataWrap ctor; TowerGameGlobalController (all methods); ReadLevels, AreWeFullScreen, GetProgramInfo, GetGlobalController                                                                                             | Move to core.cpp. core.cpp includes game.h (or core.h + deps). Declarations in core.h.                                                                                             |
| **level.cpp**    | FancyRoad::Draw and ReadLevels                                                                                                                                                                                                                                                                                                | Move FancyRoad::Draw and ReadLevels to level.cpp; level.cpp includes gameplay.h. ReadLevels declared in level.h. FancyRoad struct in level.h with forward decl AdvancedController. |
| **gameplay.cpp** | BackgroundMusicPlayer methods; TrackballTracker; BonusScore (ctor, Update, Draw); SoundControls::Update; PositionTracker; AdvancedController (both ctors, OnKey, Update, OnMouse, OnMouseDown, OnMouseUp, Fire, GetCompletionRate, MegaGeneration); AdNumberDrawer, BonusDrawer; HighScoreShower::Draw; IntroTextShower::Draw | Create gameplay.cpp; include game.h; move all listed implementations. BackgroundMusicPlayer struct moves to core.h (before TwrGlobalController).                                   |


**Declarations:** All move to core.h or level.h (see "Where everything goes" table). Nothing stays in tower_defense.

**After moves:**

- **game.h** (new): Include-only — common.h, core.h, entities.h, level.h, tutorial.h, fireball.h, critters.h, screen_controllers.h, critter_generators.h, dragon.h, gameplay.h. All .cpp files that currently include tower_defense.h switch to `#include "game.h"`.
- **tower_defense.h** and **tower_defense.cpp**: **Deleted.** Removed from CMakeLists.txt.

---

## Step-by-step plan

**Principle:** Move **declarations** from tower_defense.h to core.h / level.h first (in the order that avoids circular includes: BackgroundMusicPlayer then TwrGlobalController in core.h; FancyRoad in level.h with forward decl). Then move **implementations** from tower_defense.cpp in small chunks; Grep for all definitions of a symbol before removing (e.g. SkellyGenerator::Update). **Build once at the end of each step** (do not build multiple times within a step). After each step, update the implementation summary doc (see "Implementation summary doc" section above).

### 1. Move declarations to core.h and level.h

- **core.h:** Append `#include "level.h"`, `#include "screen_controllers.h"`, `#include "fireball.h"`. Then add in order: BackgroundMusicPlayer + BG_* enums, TwrGlobalController, GameController, TowerDataWrap, TowerGameGlobalController, `ProgramInfo GetProgramInfo();`. Do not include gameplay.h from core.h.
- **level.h:** Add `struct AdvancedController;` and struct FancyRoad (after Road). Add `void ReadLevels(std::string sFile, Rectangle rBound, LevelStorage &vLvl);`.
- Remove these declarations from tower_defense.h.
- At the end of this step, build once. Update the implementation summary doc.

### 2. Move core implementations to core.cpp

- [core.cpp](game/core.cpp): Include tower_defense.h. Move from tower_defense.cpp: pWr, nSlimeMax; DrawStuff, sFullScreenPath; TwrGlobalController ctor, StartUp, Next, Restart, Menu; TowerDataWrap ctor; TowerGameGlobalController (all methods); call site for ReadLevels (ReadLevels itself moves to level.cpp in step 3); AreWeFullScreen, GetProgramInfo, GetGlobalController. Remove each moved block from tower_defense.cpp. Grep for duplicate definitions before deleting.
- At the end of this step, build once. Update the implementation summary doc.

### 3. Move FancyRoad and ReadLevels to level

- [level.cpp](game/level.cpp): Add `#include "gameplay.h"`. Move FancyRoad::Draw and ReadLevels implementation from tower_defense.cpp. Remove from tower_defense.cpp. (Declarations and FancyRoad struct are already in level.h from step 1.)
- At the end of this step, build once. Update the implementation summary doc.

### 4. Create gameplay.cpp and move gameplay implementations

- Add [gameplay.cpp](game/gameplay.cpp) to the game target in [CMakeLists.txt](game/CMakeLists.txt). gameplay.cpp includes tower_defense.h (until step 5 when game.h exists).
- Move from tower_defense.cpp: (1) BackgroundMusicPlayer::ToggleOff, SwitchTheme, StopMusic; (2) TrackballTracker ctor, Update, IsTrigger, GetAvMovement, GetDerivative; (3) BonusScore ctor, Update, Draw; (4) SoundControls::Update; (5) PositionTracker On, Off, Update, UpdateMouse, UpdateLastDownPosition, GetFlightDirection; (6) AdvancedController copy ctor, then main ctor, then OnKey, Update, OnMouse, OnMouseDown, OnMouseUp, Fire, GetCompletionRate, MegaGeneration (x2); (7) AdNumberDrawer and BonusDrawer (struct definitions + implementations); (8) HighScoreShower::Draw, IntroTextShower::Draw. Remove each moved block from tower_defense.cpp. (BackgroundMusicPlayer struct is already in core.h from step 1.) Grep for duplicate definitions.
- At the end of this step, build once. Update the implementation summary doc.

### 5. Add game.h, switch includes, delete tower_defense files

- Create **game.h** with only: #include common.h, core.h, entities.h, level.h, tutorial.h, fireball.h, critters.h, screen_controllers.h, critter_generators.h, dragon.h, gameplay.h.
- In every .cpp that has `#include "tower_defense.h"`, replace with `#include "game.h"`.
- Delete tower_defense.h and tower_defense.cpp. Remove tower_defense.cpp from [CMakeLists.txt](game/CMakeLists.txt).
- At the end of this step, build once. Update the implementation summary doc.

### 6. Build and fix

- Build: `cd c:\coding\dragongame_git\build; mingw32-make` (PowerShell: use `;` not `&&`). Fix duplicate symbols (only one definition of pWr, nSlimeMax, each method), include order, and missing includes. Use forward declarations in headers and full includes in .cpp where needed.
- Update the implementation summary doc.

---

## Dependency and circular reference notes

- **core.cpp** will need the full definitions of TwrGlobalController, TowerDataWrap, TowerGameGlobalController, LevelStorage, MenuController, etc., to compile their methods. core.cpp includes game.h (or core.h + level.h + screen_controllers.h + fireball.h + gameplay.h). core.h does not include gameplay.h so gameplay.h can include core.h.
- **level**'s FancyRoad::Draw is in level.cpp; level.cpp must include a header that defines AdvancedController (e.g. gameplay.h). level.h can forward-declare AdvancedController for FancyRoad's member type.
- **gameplay.cpp** — Will need game.h (or core.h + entities, level, critters, fireball, screen_controllers, tutorial, dragon). BackgroundMusicPlayer is in core.h (before TwrGlobalController). **Include order:** In game.h (and previously tower_defense.h), critter_generators.h must come **before** gameplay.h so KnightGenerator and MageGenerator are complete when AdvancedController is parsed.

---

## Checklist (what was missing or deferred from original plan)

- **gameplay.cpp** was never created; all gameplay implementations stayed in tower_defense.cpp. Finalize step: add gameplay.cpp and move those implementations.
- **Core** type declarations move to core.h; implementations to core.cpp. Nothing stays in tower_defense.
- **FancyRoad** moves to level.h (struct + forward decl) and level.cpp (FancyRoad::Draw); level.cpp includes gameplay.h.
- **BackgroundMusicPlayer** moves to core.h (before TwrGlobalController); method implementations to gameplay.cpp.
- **Helper/entry symbols** and **AdNumberDrawer/BonusDrawer** go to core.cpp, level.cpp, gameplay.cpp. Add **game.h** (include-only). Delete tower_defense.h and tower_defense.cpp.

