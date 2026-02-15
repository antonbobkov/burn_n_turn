---
name: Tower defense file split
overview: "Split [game/tower_defense.h](game/tower_defense.h) and [game/tower_defense.cpp](game/tower_defense.cpp) into smaller files using a type/hierarchy grouping: core, entities, screen_controllers, level, fireball, dragon, critters, critter_generators, gameplay, tutorial."
todos: []
isProject: false
---

# Refactor tower_defense into smaller files (Grouping 2)

The codebase is currently two large files: [game/tower_defense.h](game/tower_defense.h) (~~2113 lines) and [game/tower_defense.cpp](game/tower_defense.cpp) (~~4877 lines). The class/function inventory is in [game/tower_defense_tables.md](game/tower_defense_tables.md). This plan uses **Grouping 2: by type (hierarchy + related types)** — base types first, then one file per major game object family.

**Shared pieces:**

- **Common header** (e.g. `common.h`): Preprocessor defines (`FULL_VERSION`, `PC_VERSION`, etc.), constants (`nFramesInSecond`, `fDragonSpeed`, etc.), `extern` declarations (`nSlimeMax`, `pWr`), `typedef`/`using` (e.g. `Graphic`, `Soundic`), and **templates**: `SavableVariable<T>`, `CleanUp`, `CopyASSP`, `CopyArrayASSP`, `Union`, `Out`, `PushBackASSP`, `BoolToggle` (template use).
- **Forward declarations**: Each new header will need forward decls or includes for types it references.

Implementation: each module has a `.h` and `.cpp` (e.g. `drawing.cpp` for `drawing.h`). File columns below list only the **module name** (no `.h`/`.cpp`).

---

## Chosen grouping: by type (hierarchy + related types)


| File                   | Classes / structs                                                                                                                                                                                                                                      | Global functions                                                                                                                                                |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **core**               | Polar, Drawer, ScalingDrawer, NumberDrawer, GameController, SavableVariable (decl only; template in common), SoundInterfaceProxy, TwrGlobalController, TowerDataWrap, TowerGameGlobalController                                                        | ComposeDirection, GetWedgeAngle, RandomAngle, BreakUpString, BoolToggle, Reset, DiscreetAngle, Center, GetRandTimeFromRate, GetRandNum, GetRandFromDistribution |
| **entities**           | Entity, EventEntity, ScreenEntity, VisualEntity, PhysicalEntity, SimpleVisualEntity, SimpleSoundEntity, TextDrawEntity, StaticImage, StaticRectangle, Animation, AnimationOnce, ScreenPos, ConsumableEntity, Critter, FancyCritter                     | —                                                                                                                                                               |
| **screen_controllers** | SimpleController, FlashingController, BasicController, AlmostBasicController, StartScreenController, BuyNowController, SlimeUpdater, Cutscene, MenuController, DragonScoreController, MenuEntry, MenuEntryManager, MenuDisplay, Countdown, MouseCursor | OnOffString, SoundString, MusicString, TutorialString, FullTextString                                                                                           |
| **level**              | SegmentSimpleException, BrokenLine, LevelLayout, Road, FancyRoad                                                                                                                                                                                       | operator<<(BrokenLine), operator>>(BrokenLine), operator<<(LevelLayout), operator>>(LevelLayout), operator<<(Road), operator>>(Road)                            |
| **fireball**           | FireballBonus, Chain, ChainExplosion, KnightOnFire, TimedFireballBonus, Fireball, CircularFireball, FireballBonusAnimation                                                                                                                             | operator<<(FireballBonus), GetFireballRaduis, GetSizeSuffix, GetExplosionInitialRaduis, GetExplosionExpansionRate, GetFireballChainNum                          |
| **dragon**             | DragonLeash, ButtonSet, Dragon                                                                                                                                                                                                                         | —                                                                                                                                                               |
| **critters**           | Castle, Princess, Knight, Mage, Trader, Slime, MegaSlime, Ghostiness, Sliminess, MegaSliminess, FloatingSlime                                                                                                                                          | SummonSkeletons, GetTimeUntillSpell, RandomBonus, GetBonusImage                                                                                                 |
| **critter_generators** | KnightGenerator, PrincessGenerator, MageGenerator, TraderGenerator, SkellyGenerator                                                                                                                                                                    | —                                                                                                                                                               |
| **gameplay**           | TrackballTracker, PositionTracker, AdvancedController, HighScoreShower, IntroTextShower, BonusScore, BackgroundMusicPlayer, SoundControls                                                                                                              | —                                                                                                                                                               |
| **tutorial**           | TutorialTextEntity, TutorialLevelOne, TutorialLevelTwo                                                                                                                                                                                                 | —                                                                                                                                                               |


**Placement notes (review pass):**

- **Menu string helpers** (OnOffString, SoundString, MusicString, TutorialString, FullTextString) moved from core to **screen_controllers** — they are only used for menu option labels.
- **BreakUpString** remains in **core** — used by TextDrawEntity (entities) and tutorial; general string util.
- **Core** keeps drawing types, base GameController, global state (TwrGlobalController, TowerDataWrap, TowerGameGlobalController), and shared math/random helpers.
- **Entities** holds the full entity hierarchy up to and including Critter/FancyCritter/ConsumableEntity; **critters** holds only concrete unit types (Castle, Princess, Knight, etc.) and unit-related helpers (SummonSkeletons, GetTimeUntillSpell, RandomBonus, GetBonusImage).
- **Level** holds level geometry and layout (BrokenLine, LevelLayout, Road, FancyRoad); **fireball** holds all fireball/bonus/explosion types and their accessors.
- **Dragon** holds only dragon steering and player (DragonLeash, ButtonSet, Dragon); Fireball/CircularFireball stay in **fireball** as projectile logic.

**Dependencies:** `core` is the root. `entities` depends on core. `screen_controllers` and `level` depend on core (and optionally entities). `fireball` and `dragon` depend on entities and level. `critters` and `critter_generators` depend on entities and level. `gameplay` depends on dragon, critters, level, screen_controllers. `tutorial` depends on core and entities.

---

## Dependency order (least to most dependent)

Groups ordered by how many other groups they depend on, then by topological order so each group is built only after its dependencies:


| Order | Group                  | Depends on                                            | Notes                                                                 |
| ----- | ---------------------- | ----------------------------------------------------- | --------------------------------------------------------------------- |
| 1     | **core**               | (none)                                                | Root: drawing, GameController, global state, shared utils             |
| 2     | **entities**           | core                                                  | Entity hierarchy through Critter, FancyCritter, ConsumableEntity      |
| 3     | **level**              | core, entities                                        | Road/FancyRoad extend VisualEntity; BrokenLine, LevelLayout           |
| 4     | **tutorial**           | core, entities                                        | TutorialTextEntity, TutorialLevelOne/Two                              |
| 5     | **screen_controllers** | core, entities                                        | All screen controllers + menu (MenuDisplay, etc.)                     |
| 6     | **fireball**           | core, entities                                        | FireballBonus, Chain, Fireball, ChainExplosion, etc.                  |
| 7     | **critters**           | entities                                              | Castle, Princess, Knight, Mage, Trader, Slime, etc.                   |
| 8     | **critter_generators** | entities, level                                       | KnightGenerator, PrincessGenerator, etc.; use BrokenLine, LevelLayout |
| 9     | **dragon**             | entities, critters                                    | Dragon holds SSPCastle; DragonLeash, ButtonSet                        |
| 10    | **gameplay**           | entities, level, screen_controllers, critters, dragon | AdvancedController, input, HUD, music                                 |


---

## Implementation plan (incremental)

Move **one group at a time** in the dependency order above. After each move, **build and verify** before starting the next group. Keep `tower_defense.h` / `tower_defense.cpp` as the shrinking “rest” until the last step; optionally keep a single `tower_defense.h` that includes all new headers for existing callers.

**Step 0 — Prepare**

- Create `common.h` (and optionally `common.cpp` if any non-inline code): defines, constants, externs, typedefs, and templates (SavableVariable, CleanUp, CopyASSP, etc.). Leave everything else in `tower_defense.h` / `tower_defense.cpp`.
- Update `tower_defense.h` to include `common.h` and remove the moved content. Update `tower_defense.cpp` to remove the moved implementations.
- **Build and verify.**

**Step 1 — core**

- Add `core.h` and `core.cpp`. Move Polar, Drawer, ScalingDrawer, NumberDrawer, GameController, SoundInterfaceProxy, TwrGlobalController, TowerDataWrap, TowerGameGlobalController; SavableVariable declaration (template body stays in common); and global functions ComposeDirection, GetWedgeAngle, RandomAngle, BreakUpString, BoolToggle, Reset, DiscreetAngle, Center, GetRandTimeFromRate, GetRandNum, GetRandFromDistribution.
- Remove that content from `tower_defense.h` / `tower_defense.cpp`. Have `tower_defense.h` include `core.h`.
- **Build and verify.**

**Step 2 — entities**

- Add `entities.h` and `entities.cpp`. Move Entity, EventEntity, ScreenEntity, VisualEntity, PhysicalEntity, SimpleVisualEntity, SimpleSoundEntity, TextDrawEntity, StaticImage, StaticRectangle, Animation, AnimationOnce, ScreenPos, ConsumableEntity, Critter, FancyCritter.
- Remove from `tower_defense.*`; `tower_defense.h` includes `entities.h` (and entities includes core or common as needed).
- **Build and verify.**

**Step 3 — level**

- Add `level.h` and `level.cpp`. Move SegmentSimpleException, BrokenLine, LevelLayout, Road, FancyRoad and their stream operators.
- Remove from `tower_defense.*`; add `#include "level.h"` where needed (e.g. in `tower_defense.h` or only in files that use level types).
- **Build and verify.**

**Step 4 — tutorial**

- Add `tutorial.h` and `tutorial.cpp`. Move TutorialTextEntity, TutorialLevelOne, TutorialLevelTwo.
- Remove from `tower_defense.*`; include `tutorial.h` where needed.
- **Build and verify.**

**Step 5 — screen_controllers**

- Add `screen_controllers.h` and `screen_controllers.cpp`. Move SimpleController, FlashingController, BasicController, AlmostBasicController, StartScreenController, BuyNowController, SlimeUpdater, Cutscene, MenuController, DragonScoreController, MenuEntry, MenuEntryManager, MenuDisplay, Countdown, MouseCursor and OnOffString, SoundString, MusicString, TutorialString, FullTextString.
- Remove from `tower_defense.*`; include `screen_controllers.h` where needed.
- **Build and verify.**

**Step 6 — fireball**

- Add `fireball.h` and `fireball.cpp`. Move FireballBonus, Chain, ChainExplosion, KnightOnFire, TimedFireballBonus, Fireball, CircularFireball, FireballBonusAnimation and their global functions (operator<<(FireballBonus), GetFireballRaduis, GetSizeSuffix, GetExplosionInitialRaduis, GetExplosionExpansionRate, GetFireballChainNum).
- Remove from `tower_defense.*`; include `fireball.h` where needed.
- **Build and verify.**

**Step 7 — critters**

- Add `critters.h` and `critters.cpp`. Move Castle, Princess, Knight, Mage, Trader, Slime, MegaSlime, Ghostiness, Sliminess, MegaSliminess, FloatingSlime and SummonSkeletons, GetTimeUntillSpell, RandomBonus, GetBonusImage.
- Remove from `tower_defense.*`; include `critters.h` where needed.
- **Build and verify.**

**Step 8 — critter_generators**

- Add `critter_generators.h` and `critter_generators.cpp`. Move KnightGenerator, PrincessGenerator, MageGenerator, TraderGenerator, SkellyGenerator.
- Remove from `tower_defense.*`; include `critter_generators.h` where needed.
- **Build and verify.**

**Step 9 — dragon**

- Add `dragon.h` and `dragon.cpp`. Move DragonLeash, ButtonSet, Dragon.
- Remove from `tower_defense.*`; include `dragon.h` where needed.
- **Build and verify.**

**Step 10 — gameplay**

- Add `gameplay.h` and `gameplay.cpp`. Move TrackballTracker, PositionTracker, AdvancedController, HighScoreShower, IntroTextShower, BonusScore, BackgroundMusicPlayer, SoundControls.
- Remove from `tower_defense.*`; include `gameplay.h` where needed.
- **Build and verify.**

**Step 11 — Finalize**

- Ensure `tower_defense.h` includes common + all new module headers so existing `#include "tower_defense.h"` callers still build. Optionally reduce `tower_defense.cpp` to a small stub or remove it if all code is moved. Add all new .cpp files to the game target in CMake (or Makefile).
- **Build and verify.**

After every step, run the full project build from the build directory (e.g. `cmake ../code -G "MinGW Makefiles"; mingw32-make` in `build/`) and fix any errors before proceeding.

---

## Alternatives considered

### Grouping 1: By domain (feature-based)

Organize by game feature (e.g. “menu”, “dragon”, “level”). Good for feature-oriented navigation.


| File                 | Classes / structs                                                                                                                                                                                    | Global functions                                                                                                                       |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| **drawing**          | Polar, Drawer, ScalingDrawer, NumberDrawer                                                                                                                                                           | ComposeDirection, GetWedgeAngle, RandomAngle, Reset(ImageSequence), DiscreetAngle, Center(Size)                                        |
| **audio**            | BackgroundMusicPlayer, SoundInterfaceProxy, SoundControls                                                                                                                                            | —                                                                                                                                      |
| **global**           | TwrGlobalController, TowerDataWrap, TowerGameGlobalController                                                                                                                                        | —                                                                                                                                      |
| **controllers_base** | GameController, SimpleController, FlashingController, BasicController, AlmostBasicController, StartScreenController, BuyNowController, SlimeUpdater, Cutscene, MenuController, DragonScoreController | —                                                                                                                                      |
| **entities_base**    | Entity, EventEntity, ScreenEntity, VisualEntity, PhysicalEntity, SimpleVisualEntity, SimpleSoundEntity, TextDrawEntity, StaticImage, StaticRectangle, Animation, AnimationOnce, ScreenPos            | —                                                                                                                                      |
| **menu**             | MenuEntry, MenuEntryManager, MenuDisplay, Countdown, MouseCursor                                                                                                                                     | OnOffString, SoundString, MusicString, TutorialString, FullTextString                                                                  |
| **level**            | SegmentSimpleException, BrokenLine, LevelLayout, Road, FancyRoad                                                                                                                                     | operator<<(BrokenLine), operator>>(BrokenLine), operator<<(LevelLayout), operator>>(LevelLayout), operator<<(Road), operator>>(Road)   |
| **fireball**         | FireballBonus, Chain, ChainExplosion, KnightOnFire, TimedFireballBonus, FireballBonusAnimation                                                                                                       | operator<<(FireballBonus), GetFireballRaduis, GetSizeSuffix, GetExplosionInitialRaduis, GetExplosionExpansionRate, GetFireballChainNum |
| **dragon**           | DragonLeash, ButtonSet, Dragon, Fireball, CircularFireball                                                                                                                                           | —                                                                                                                                      |
| **units**            | ConsumableEntity, Critter, FancyCritter, Castle, Princess, Knight, Mage, Trader, Slime, MegaSlime, Ghostiness, Sliminess, MegaSliminess, FloatingSlime                                               | SummonSkeletons, GetTimeUntillSpell, RandomBonus, GetBonusImage                                                                        |
| **generators**       | KnightGenerator, PrincessGenerator, MageGenerator, TraderGenerator, SkellyGenerator                                                                                                                  | —                                                                                                                                      |
| **main_controller**  | TrackballTracker, PositionTracker, AdvancedController, HighScoreShower, IntroTextShower, BonusScore                                                                                                  | —                                                                                                                                      |
| **tutorial**         | TutorialTextEntity, TutorialLevelOne, TutorialLevelTwo                                                                                                                                               | BreakUpString                                                                                                                          |
| **utils**            | —                                                                                                                                                                                                    | GetRandTimeFromRate, GetRandNum, GetRandFromDistribution                                                                               |


~14 modules; clear feature boundaries.

### Grouping 3: By balanced size (many smaller modules)

Aim for smaller, more evenly sized files (~200–500 lines).


| File                   | Classes / structs                                                                                                                                                         | Global functions                                                                                                                       |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| **drawing**            | Polar, Drawer, ScalingDrawer, NumberDrawer                                                                                                                                | ComposeDirection, GetWedgeAngle, RandomAngle, Reset, DiscreetAngle, Center                                                             |
| **audio**              | BackgroundMusicPlayer, SoundInterfaceProxy, SoundControls                                                                                                                 | —                                                                                                                                      |
| **controller_base**    | GameController, SimpleController, FlashingController, BasicController, AlmostBasicController, StartScreenController                                                       | —                                                                                                                                      |
| **controller_screens** | BuyNowController, SlimeUpdater, Cutscene, MenuController, DragonScoreController                                                                                           | —                                                                                                                                      |
| **entity_visual**      | Entity, EventEntity, ScreenEntity, VisualEntity, SimpleVisualEntity, TextDrawEntity, StaticImage, StaticRectangle, Animation, AnimationOnce, ScreenPos, SimpleSoundEntity | —                                                                                                                                      |
| **entity_physical**    | PhysicalEntity, ConsumableEntity, Critter, FancyCritter                                                                                                                   | —                                                                                                                                      |
| **menu**               | MenuEntry, MenuEntryManager, MenuDisplay, Countdown, MouseCursor                                                                                                          | OnOffString, SoundString, MusicString, TutorialString, FullTextString                                                                  |
| **level**              | SegmentSimpleException, BrokenLine, LevelLayout, Road, FancyRoad                                                                                                          | operator<<(BrokenLine), operator>>(BrokenLine), operator<<(LevelLayout), operator>>(LevelLayout), operator<<(Road), operator>>(Road)   |
| **bonus**              | FireballBonus, Chain, TimedFireballBonus, FireballBonusAnimation                                                                                                          | operator<<(FireballBonus), GetFireballRaduis, GetSizeSuffix, GetExplosionInitialRaduis, GetExplosionExpansionRate, GetFireballChainNum |
| **explosion**          | ChainExplosion, KnightOnFire                                                                                                                                              | —                                                                                                                                      |
| **dragon**             | DragonLeash, ButtonSet, Dragon, Fireball, CircularFireball                                                                                                                | —                                                                                                                                      |
| **castle_road**        | Castle                                                                                                                                                                    | —                                                                                                                                      |
| **units**              | Princess, Knight, Mage, Trader, Slime, MegaSlime, Ghostiness, Sliminess, MegaSliminess, FloatingSlime                                                                     | SummonSkeletons, GetTimeUntillSpell, RandomBonus, GetBonusImage                                                                        |
| **generators**         | KnightGenerator, PrincessGenerator, MageGenerator, TraderGenerator, SkellyGenerator                                                                                       | —                                                                                                                                      |
| **advanced**           | TrackballTracker, PositionTracker, AdvancedController, HighScoreShower, IntroTextShower, BonusScore                                                                       | —                                                                                                                                      |
| **global**             | TwrGlobalController, TowerDataWrap, TowerGameGlobalController                                                                                                             | —                                                                                                                                      |
| **tutorial**           | TutorialTextEntity, TutorialLevelOne, TutorialLevelTwo                                                                                                                    | BreakUpString                                                                                                                          |
| **rand**               | —                                                                                                                                                                         | GetRandTimeFromRate, GetRandNum, GetRandFromDistribution                                                                               |


~18 modules; finer split of controllers and entities.

---

## Implementation notes

1. **Include order**: Each new header should include the common header (e.g. `common.h`) and only the minimal other new headers it needs; avoid a single “include everything” header except as a convenience for external code.
2. **Circular dependencies**: Several types reference each other (e.g. AdvancedController ↔ Dragon, Castle, KnightGenerator). Use forward declarations and move method implementations to .cpp so headers don’t need full definitions of all related types.
3. **Templates**: Keep `SavableVariable<T>`, `CleanUp`, `CopyASSP`, `CopyArrayASSP`, `Union`, `Out`, `PushBackASSP` (and any other templates) in the common header so they’re defined once and visible everywhere.
4. **Build**: Add new .cpp files to the game target in the existing CMake (or Makefile). After refactor, run a full build from the project’s build directory to verify (see [.cursor/rules/Verify-correctness.mdc](.cursor/rules/Verify-correctness.mdc)).
5. **Line length / style**: Keep lines under 80 characters and use simple, non-technical comments per [.cursor/rules/Code-style.mdc](.cursor/rules/Code-style.mdc).

The existing `tower_defense.h` can include all new headers for callers that currently only include `tower_defense.h`. Apply the split incrementally (e.g. one module at a time) and run the build after each step.