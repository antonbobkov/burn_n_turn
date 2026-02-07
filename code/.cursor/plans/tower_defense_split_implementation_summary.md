# Tower Defense File Split — Implementation Summary

This document summarizes how the plan in `tower_defense_file_split_de854034.plan.md` (Grouping 2: split by type) was implemented: what was done, what issues came up, and how they were solved. It also notes types that could not be moved or required special handling.

**Scope and conversation context:** This summary is based on the **exported conversation history** from the refactoring session (e.g. `cursor_tower_defense_file_refactoring.md`). Steps **0–11** were implemented in that single session; the notes below use the actual issues and fixes from that transcript. Where the export was abbreviated, details are retained from earlier passes over the codebase.

---

## At a glance: issues, fixes, and plan deviations

**Issues and how they were solved**

| Issue | Step | Fix |
|-------|------|-----|
| Missing constant (fSlimeSpeed) | 0 | Add the constant to common.h and rebuild. |
| Circular / heavy dependency (core.cpp needing full TwrGlobalController) | 1 | Revert core.cpp including tower_defense.h; keep GameController in tower_defense.h. |
| Type in level.h requires full AdvancedController when compiling level.cpp | 3 | Move FancyRoad out of level.h back into tower_defense.h (after AdvancedController). |
| ODR: header-defined helpers duplicated in multiple TUs | 5 | Make SoundString, MusicString, TutorialString, FullTextString, GetTimeUntillSpell **inline** in their headers. |
| GetProgramInfo used by screen_controllers but defined elsewhere | 5 | Declare GetProgramInfo() in tower_defense.h; define in one .cpp. |
| KnightGenerator / MageGenerator undefined when parsing AdvancedController | 8 | Include critter_generators.h **before** the AdvancedController definition in tower_defense.h. |
| Multiple definition (critter code still in tower_defense.cpp) | 7 | Remove duplicate Slime, MegaSlime, Knight, Ghostiness implementations from tower_defense.cpp. |
| Single large search/replace failed (whitespace, comments, or wrong boundary) | 7, 8, 10 | Remove in smaller chunks; verify “next function” or line boundaries; handle code that appears in two places (e.g. SkellyGenerator::Update). |

**Plan deviations**

- **Core:** GameController, TwrGlobalController, TowerDataWrap, TowerGameGlobalController stayed in tower_defense.h (plan had them in core). Reason: avoid circular or heavy deps; they act as the hub.
- **Level:** FancyRoad stayed in tower_defense.h (plan had it in level). Reason: FancyRoad uses `SSP<AdvancedController>`; level would need the full controller when compiling level.cpp.
- **Gameplay:** BackgroundMusicPlayer stayed in tower_defense.h; gameplay.cpp was not added (implementations remain in tower_defense.cpp). Reason: TwrGlobalController needs BackgroundMusicPlayer before it’s defined, while AdvancedController (in gameplay) needs TwrGlobalController; moving implementations would have been a large, low-value change.
- **Critters:** SkellyGenerator was moved in Step 8 with critter_generators, not in Step 7 with critters.

**Common themes**

1. **Include order and type visibility** — New headers often had to be included only after certain types (or forward decls) were in place. Putting an include in the “wrong” place led to “does not name a type” or incomplete-type errors.
2. **Cross-module dependencies** — Types that hold pointers/references to “later” types (e.g. `SSP<AdvancedController>`) forced either keeping the type in the main header or pulling in the full definition and risking circular includes.
3. **Duplicate definitions** — Forgetting to delete moved code from tower_defense.cpp caused link errors; header-only helpers included in multiple TUs caused ODR violations until made inline.
4. **Mechanical replace failures** — Large blocks were hard to delete in one go (whitespace, comments, or wrong end-of-block). Implementation of one class sometimes appeared in two places in the file.

**If doing this task again: what would help**

- **Include-order notes in the plan** — For each new header, document “include after X” (and any forward decls that must appear first) so placement is right the first time.
- **Per-step removal checklist** — For each move, list the exact function/member names (and file locations) to remove from tower_defense.cpp so nothing is left behind.
- **Dependency list for “hub” types** — Before moving, list which types hold `SSP<T>` or references to types defined later; decide up front whether they stay in tower_defense.h or which module is allowed to include which.
- **Search before delete** — Grep for all definitions of a symbol (e.g. `Slime::`, `Ghostiness::`) to catch duplicates and second occurrences (e.g. SkellyGenerator::Update far from other generators).
- **Small, bounded edits** — Prefer deleting one function or one small block at a time and re-running the build, rather than one big replace.
- **Shell/build note** — On PowerShell, use `;` not `&&` between commands (e.g. for cmake and make).

---

## Step 0 — common.h (Prepare)

**Goal:** Create `common.h` with defines, constants, externs, typedefs, and templates; no `common.cpp`.

**What was done:** `common.h` was created with preprocessor defines (`FULL_VERSION`, `PC_VERSION`, etc.), constants (`nFramesInSecond`, `fDragonSpeed`, speed constants, etc.), `extern` declarations (`nSlimeMax`, `pWr`), and templates (`SavableVariable`, `CleanUp`, `CopyASSP`, `PushBackASSP`, etc.). **BoolToggle** was made inline in `common.h` (one-line body). `tower_defense.h` was updated to include `common.h` and remove the moved content. No `common.cpp` was added.

**Issues:** (1) **Build failed after first pass** — The build was run to verify Step 0; it failed because **fSlimeSpeed** was missing from `common.h`. The constant was added and the build then succeeded.

---

## Step 1 — core

**Goal:** Add `core.h` / `core.cpp` with Polar, Drawer, ScalingDrawer, NumberDrawer, GameController, SoundInterfaceProxy, TwrGlobalController, and shared helpers (ComposeDirection, RandomAngle, etc.).

**What was done:** **TwrGlobalController**, **TowerDataWrap**, and **TowerGameGlobalController** were left in `tower_defense.h` from the start (they use `LevelStorage`, which lives with the level module). Moved into core: Polar, Drawer, ScalingDrawer, NumberDrawer, GameController, SoundInterfaceProxy, and the listed global functions. Then `core.cpp` was added and the build run.

**Issues:** (1) **Circular dependency** — To let `SSP` be instantiated for TwrGlobalController, the implementation tried including the full `TwrGlobalController` and added `#include "tower_defense.h"` to `core.cpp`. That created a circular or undesirable dependency. (2) **Fix:** The `core.cpp` include of `tower_defense.h` was reverted and **GameController** was moved back to `tower_defense.h` so core does not require the full TwrGlobalController. GameController was re-added to `tower_defense.h` after the core include. Core therefore contains Polar, Drawer, ScalingDrawer, NumberDrawer, SoundInterfaceProxy, and the global functions; GameController remains in `tower_defense.h` (it depends on TwrGlobalController).

**Classes not moved:** GameController, TwrGlobalController, TowerDataWrap, TowerGameGlobalController remain in `tower_defense.h`.

---

## Step 2 — entities

**Goal:** Add `entities.h` / `entities.cpp` with the entity hierarchy through Critter, FancyCritter, ConsumableEntity, ScreenPos.

**What was done:** Entity struct boundaries and implementations were located; `entities.h` and `entities.cpp` were created. `tower_defense.h` was updated to add `#include "entities.h"` and remove the entity definitions (keeping TutorialTextEntity and tutorial types for Step 4). Moved implementations were removed from `tower_defense.cpp`.

**Issues:** None documented in the transcript. Build and continuation to Step 3 followed.

---

## Step 3 — level

**Goal:** Add `level.h` / `level.cpp` with SegmentSimpleException, BrokenLine, LevelLayout, Road, **FancyRoad**, and stream operators.

**What was done:** `level.h` and `level.cpp` were created. In `tower_defense.h`: added `#include "level.h"`, removed LevelLayout/LevelStorage forward decls, and removed the definitions of Road, FancyRoad, SegmentSimpleException, BrokenLine, LevelLayout, and their stream operators. In `tower_defense.cpp`: removed implementations of Road::Draw, Road::RoadMap, BrokenLine, LevelLayout, and stream operators; **kept FancyRoad::Draw**. `level.cpp` was added to the build.

**Issues:** **FancyRoad in level.h caused a dependency problem** — FancyRoad contains `SSP<AdvancedController>`, so defining it in `level.h` forced `AdvancedController` to be complete when compiling `level.cpp`. **Fix:** FancyRoad was moved out of `level.h` and back into `tower_defense.h` (after `AdvancedController` is defined). So FancyRoad was initially placed in the level module, then moved back when the build/compile revealed the dependency.

**Classes not moved:** FancyRoad (and FancyRoad::Draw) remain in `tower_defense.h` / `tower_defense.cpp`.

---

## Step 4 — tutorial

**Goal:** Add `tutorial.h` / `tutorial.cpp` with TutorialTextEntity, TutorialLevelOne, TutorialLevelTwo.

**What was done:** `tutorial.h` and `tutorial.cpp` were created. The steer/shooting/takeoff message string constants were moved into `tutorial.cpp` (anonymous namespace). In `tower_defense.h`: added `#include "tutorial.h"` and removed the three tutorial structs and the `#ifdef` string constants. In `tower_defense.cpp`: removed all tutorial method implementations. `tutorial.cpp` was added to the build.

**Issues:** None documented. Build succeeded.

---

## Step 5 — screen_controllers

**Goal:** Add `screen_controllers.h` / `screen_controllers.cpp` with SimpleController through AlmostBasicController and menu string helpers.

**What was done:** Screen-controller types and implementations were located. `screen_controllers.h` was created and the include added to `tower_defense.h`. A **forward declaration for MenuController** was added before MenuDisplay (MenuDisplay references it). The screen_controllers block in `tower_defense.h` was replaced with `#include "screen_controllers.h"`; remaining types (BasicController through AlmostBasicController) were removed. `screen_controllers.cpp` was written with the moved implementations and added to the build. Implementations were removed from `tower_defense.cpp` in chunks.

**Issues:** (1) **GetProgramInfo** — A `GetProgramInfo()` declaration was added to `tower_defense.h` so `screen_controllers.cpp` could call it (definition lives in one .cpp). (2) **ODR violations** — After the move, duplicate definitions were checked; the header-defined string helpers (and GetTimeUntillSpell) were causing multiple-definition link errors. **Fix:** Those functions were made **inline** in their headers to fix the ODR violations.

---

## Step 6 — fireball

**Goal:** Add `fireball.h` / `fireball.cpp` with all fireball-related types and GetFireball* / operator<<(FireballBonus).

**What was done:** Fireball module was created with the header and **include order** updated in `tower_defense.h`. `fireball.h` and `fireball.cpp` were created (with `operator<<` and `GetFireballChainNum` checked in the .cpp). `tower_defense.h` was updated to add the fireball include and remove the fireball block. `fireball.cpp` was created with the moved implementations; fireball implementations were removed from `tower_defense.cpp` and `fireball.cpp` was added to the build.

**Issues:** None explicitly called out in the transcript. The plan’s “updating the include order” indicates the fireball include was placed after the necessary forward declarations (TwrGlobalController, BasicController, AdvancedController) so that fireball types that reference controllers compile correctly.

---

## Step 7 — critters

**Goal:** Add `critters.h` / `critters.cpp` with Castle, Princess, Knight, Mage, Trader, Slime, MegaSlime, Ghostiness, Sliminess, MegaSliminess, FloatingSlime, and SummonSkeletons, GetTimeUntillSpell, RandomBonus, GetBonusImage.

**What was done:** Critter types and helpers moved. **SkellyGenerator** stayed in `tower_defense.h` until Step 8 (it was moved with critter_generators).

**Issues:**

1. **Duplicate implementations in `tower_defense.cpp`**  
   After moving code to `critters.cpp`, the same member functions were still defined in `tower_defense.cpp`, causing **multiple definition** link errors. The following blocks had to be removed from `tower_defense.cpp`:
   - **Slime::Update** and **Slime::OnHit** — One search/replace failed because the next function in the file was **BonusScore::Draw**, not Knight::OnHit as assumed. The correct “next function” was used in a second replace to delete only the Slime block.
   - **MegaSlime::OnHit** — Removed in a separate replace.
   - **Knight::OnHit** — Removed (definition already in `critters.cpp`).
   - **Ghostiness::Ghostiness** constructor — Linker reported the constructor defined in both `tower_defense.cpp` and `critters.cpp`; the definition in `tower_defense.cpp` was removed.

2. **Search/replace failures**  
   Some replaces failed with “exact match not found” due to whitespace or comment differences (e.g. “// golems” on its own line). Fixes: use the exact text from the file, or remove in smaller chunks.

**Classes not moved:** SkellyGenerator stayed in `tower_defense.h` until Step 8 (moved with critter_generators).

---

## Step 8 — critter_generators

**Goal:** Add `critter_generators.h` / `critter_generators.cpp` with KnightGenerator, PrincessGenerator, MageGenerator, TraderGenerator, SkellyGenerator.

**What was done:** All five generator structs and their implementations moved. `tower_defense.h` includes `critter_generators.h`; generator implementations removed from `tower_defense.cpp`.

**Issues:**

1. **Include order and undefined types**  
   Initially `#include "critter_generators.h"` was placed **after** the definition of AdvancedController. When `screen_controllers.cpp` (which includes `tower_defense.h`) was compiled, **KnightGenerator** and **MageGenerator** were not yet defined at the point where AdvancedController (with members `KnightGenerator *pGr` and `MageGenerator *pMgGen`) was parsed. **Fix:** Include `critter_generators.h` **before** the definition of AdvancedController (after the Slime/Sliminess forward decls), so the generator types are complete when AdvancedController is defined.

2. **Removing a large block from `tower_defense.cpp`**  
   Removing all generator code in one replace failed (comment/whitespace differences; e.g. “// golems” on its own line in KnightGenerator::Generate). The block was removed in smaller steps: first SkellyGenerator constructor only, then the rest from KnightGenerator::GetRate through TraderGenerator::Update up to (but not including) DragonLeash::ModifyTilt. **SkellyGenerator::Update** (defined later in the file, after TowerGameGlobalController::Fire) was removed in a separate replace.

---

## Step 9 — dragon

**Goal:** Add `dragon.h` / `dragon.cpp` with DragonLeash, ButtonSet, Dragon.

**What was done:** All three types and their implementations moved. `dragon.h` includes `critters.h` and `fireball.h` and forward-declares AdvancedController. `dragon.cpp` includes `tower_defense.h` for full context. `tower_defense.h` includes `dragon.h` after `critter_generators.h`; the former `struct Dragon` forward decl was removed.

**Issues:** None. Build succeeded after adding `dragon.cpp` to CMake and removing the duplicate definitions from `tower_defense.h` and `tower_defense.cpp`.

---

## Step 10 — gameplay

**Goal:** Add `gameplay.h` / `gameplay.cpp` with TrackballTracker, PositionTracker, AdvancedController, HighScoreShower, IntroTextShower, BonusScore, BackgroundMusicPlayer, SoundControls.

**What was done:**

- **gameplay.h** was added with: TrackballTracker, BonusScore, SoundControls, PositionTracker, AdvancedController, HighScoreShower, IntroTextShower. A **forward declaration** `struct AdvancedController;` was added at the top so BonusScore (which has `SSP<AdvancedController> pAc`) compiles before the full AdvancedController definition.
- **BackgroundMusicPlayer** (and the BG_MUSIC_* enums) were **not** moved to `gameplay.h`. TwrGlobalController has a member `BackgroundMusicPlayer plr` and must be defined after BackgroundMusicPlayer. AdvancedController and the rest of gameplay need TwrGlobalController (e.g. constructor). So “music” types had to stay before TwrGlobalController and “gameplay” types after it. Keeping BackgroundMusicPlayer in `tower_defense.h` avoided a two-phase or circular include for gameplay.
- **gameplay.cpp was not added.** All implementations for TrackballTracker, BonusScore, SoundControls, PositionTracker, AdvancedController, HighScoreShower, and IntroTextShower remain in `tower_defense.cpp` to avoid moving a large amount of code and to keep the refactor low-risk. So gameplay is “declarations in gameplay.h, implementations in tower_defense.cpp.”

**Issues:**

1. **Include order and duplicates**  
   After adding `#include "gameplay.h"` and removing the old AdvancedController/FancyRoad/HighScoreShower/IntroTextShower block, the **first** replace also removed the block that contained TrackballTracker through PositionTracker. A **second** replace then tried to remove that same block again and failed (it was already gone). The remaining duplicate block (TrackballTracker through SoundControls and “struct Slime; struct Sliminess”) was removed in a separate replace so that only the intended includes (critter_generators, dragon, gameplay) and FancyRoad remain before TowerDataWrap.

**Classes not moved (or only partially):**

- **BackgroundMusicPlayer** — Stays in `tower_defense.h` so it is defined before TwrGlobalController.
- **SoundControls** — Declared in `gameplay.h` but depends on BackgroundMusicPlayer (reference member); BackgroundMusicPlayer remains in `tower_defense.h`.
- **Gameplay implementations** — Not moved to `gameplay.cpp`; they remain in `tower_defense.cpp`.

---

## Step 11 — Finalize

**Goal:** Ensure `tower_defense.h` includes common and all new module headers, and all new .cpp files are in the build. Optional: reduce `tower_defense.cpp` to a stub.

**What was done:**

- Confirmed **tower_defense.h** pulls in: `core.h`, `entities.h`, `level.h`, `tutorial.h`, `fireball.h`, `critters.h`, `screen_controllers.h`, `critter_generators.h`, `dragon.h`, `gameplay.h`. So a single `#include "tower_defense.h"` still gets all modules.
- Confirmed **game/CMakeLists.txt** lists all new .cpp files: core, entities, level, tutorial, screen_controllers, fireball, critters, critter_generators, dragon, tower_defense. **gameplay.cpp** was not added because gameplay implementations were left in `tower_defense.cpp`.
- Full build from the build directory succeeded.

**Issues:** None.

**Optional not done:** `tower_defense.cpp` was not reduced to a stub; it still holds gameplay implementations and the remaining tower_defense-specific code (e.g. FancyRoad::Draw, BackgroundMusicPlayer, TowerDataWrap, TowerGameGlobalController, etc.).

---

## Summary: Types That Weren’t Moved or Had Special Handling

| Type | Location | Reason |
|------|----------|--------|
| GameController, TwrGlobalController, TowerDataWrap, TowerGameGlobalController | tower_defense.h | Central hub; screen controllers and game flow depend on them. |
| FancyRoad (and FancyRoad::Draw) | tower_defense.h / tower_defense.cpp | Uses `SSP<AdvancedController>`; kept with main game as in plan. |
| BackgroundMusicPlayer | tower_defense.h | Must be defined before TwrGlobalController (member `plr`). |
| SoundControls | gameplay.h (declaration only) | Depends on BackgroundMusicPlayer; impl still in tower_defense.cpp. |
| All gameplay types (TrackballTracker, BonusScore, PositionTracker, AdvancedController, HighScoreShower, IntroTextShower) | gameplay.h (declarations) | Implementations intentionally left in tower_defense.cpp; no gameplay.cpp. |

---

## Build and Verification

- **Build command used:** `cd c:\coding\dragongame_git\build; cmake ../code -G "MinGW Makefiles"; mingw32-make` (PowerShell: use `;` not `&&`; the first attempt with `&&` can fail with “The token '&&' is not a valid statement separator”).
- In the refactoring session, the build was run after each step; missing symbols (e.g. fSlimeSpeed), duplicate definitions, and include-order issues were fixed until the build succeeded.
- Final state: all 11 steps completed, full build succeeds, and `#include "tower_defense.h"` still provides access to all split modules.
