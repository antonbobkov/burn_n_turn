# Phase 3: Game entities and animations — Plan

---
wave: 1
depends_on: ["02-wrappers-and-platform"]
files_modified:
  - code/game_utils/draw_utils.h
  - code/game_utils/draw_utils.cc
  - code/game_utils/image_sequence.h
  - code/game/entities.h
  - code/game/entities.cc
  - code/game/level.h
  - code/game/level.cc
  - code/game/fireball.h
  - code/game/fireball.cc
  - code/game/critters.h
  - code/game/critters.cc
  - code/game/dragon.h
  - code/game/dragon.cc
  - code/game/dragon_game_runner.h
  - code/game/dragon_game_runner.cc
  - code/game/controller/basic_controllers.h
  - code/game/controller/basic_controllers.cc
  - code/game/controller/menu_controller.h
  - code/game/controller/menu_controller.cc
  - code/game/controller/dragon_game_controller.h
  - code/game/controller/dragon_game_controller.cc
  - code/game/controller/level_controller.h
  - code/game/controller/level_controller.cc
  - code/game/controller/buy_now_controller.h
  - code/game/controller/buy_now_controller.cc
  - code/game/tutorial.h
  - code/game/tutorial.cc
  - code/simulation/simulation.cc
  - code/simulation/simulation_test.cc
autonomous: false
---

**Phase:** 3 — Game entities and animations  
**Goal (ROADMAP):** Migrate leaf game types: Entity hierarchy, Drawer/
NumberDrawer and concrete drawers, AnimationOnce, StaticImage, Animation,
simple entities. Ownership lives in level, controllers, or critters.

## must_haves (goal-backward verification)

- [ ] ScalingDrawer: single owner in TowerDataWrap (unique_ptr); DragonGameController
  and all call sites use ScalingDrawer*; Draw(ScalingDrawer*) everywhere.
- [ ] NumberDrawer: single owner in TowerDataWrap (unique_ptr); DragonGameController,
  TextDrawEntity, TutorialTextEntity, MenuDisplay use NumberDrawer*.
- [ ] Leaf entities (Animation, AnimationOnce, StaticImage, StaticRectangle,
  TextDrawEntity, SimpleSoundEntity, HighScoreShower, IntroTextShower,
  BonusScore, AdNumberDrawer, BonusDrawer, TutorialTextEntity, MenuDisplay,
  SoundControls): single owner at creating controller/level; raw pointers in
  lists and in entities that reference them.
- [ ] SP_Info removed from Drawer, ScalingDrawer, NumberDrawer, Entity and
  migrated subclasses where no smart_pointer use remains (CLEAN-01).
- [ ] Build succeeds; ctest and simulation_test pass (VER-01, VER-02).
- [ ] Any type skipped (lifetime unclear) recorded for DOC-01.

---

## Wave 1: ScalingDrawer and Draw(ScalingDrawer*)

Execute in order. Run build and ctest; run simulation_test after each task
(or after a small group). VER-01, VER-02.

### Task 1.1 — TowerDataWrap: own two ScalingDrawers with unique_ptr; NumberDrawer takes ScalingDrawer*

- In `dragon_game_runner.h`: Change TowerDataWrap to hold
  `std::unique_ptr<ScalingDrawer> pDr` and
  `std::unique_ptr<ScalingDrawer> pBigDr`. Remove smart_pointer<ScalingDrawer>.
- In `dragon_game_runner.cc`: TowerDataWrap ctor creates both with
  std::make_unique<ScalingDrawer>(pGr, nScale) and
  std::make_unique<ScalingDrawer>(pGr, nScale * 2). Pass pDr.get() and
  pBigDr.get() into NumberDrawer ctors (NumberDrawer must accept raw ptr).
- In `game_utils/draw_utils.h` and `draw_utils.cc`: Change NumberDrawer
  ctor and member `pDr` to take/store `ScalingDrawer*` (non-owning) so
  TowerDataWrap can pass pDr.get() and pBigDr.get(). Do not yet change
  TowerDataWrap to unique_ptr<NumberDrawer> (Wave 2).
- Build and run ctest; run simulation_test.

### Task 1.2 — DragonGameController: store ScalingDrawer* and return from GetDrawer()

- In `dragon_game_controller.h`: Change `pDr` from smart_pointer<ScalingDrawer>
  to `ScalingDrawer*`. Declare GetDrawer() to return ScalingDrawer*.
- In `dragon_game_controller.cc`: Ctor takes ScalingDrawer* (e.g. from
  TowerDataWrap); assign and store raw pointer; GetDrawer() returns pDr.
- In `dragon_game_runner.cc`: When constructing DragonGameController, pass
  tower_data.pDr.get() (or equivalent) so controller receives raw pointer.
- Build and run ctest; run simulation_test.

### Task 1.3 — Draw(smart_pointer<ScalingDrawer>) → Draw(ScalingDrawer*) in headers

- In `game/entities.h`: Change VisualEntity::Draw and all overrides from
  `void Draw(smart_pointer<ScalingDrawer> pDr)` to `void Draw(ScalingDrawer*
  pDr)`.
- In `game/level.h`: Change Road::Draw and FancyRoad::Draw to take
  ScalingDrawer*.
- In `game/fireball.h`: Change FireballBonusAnimation::Draw and any other
  Draw taking ScalingDrawer to ScalingDrawer*.
- In `game/critters.h`: Change all Draw(ScalingDrawer*) overrides.
- In `game/dragon.h`: Change Dragon::Draw to take ScalingDrawer*.
- In `game/controller/menu_controller.h`: Any Draw(ScalingDrawer*) signature.
- In `game/controller/buy_now_controller.h`: Draw(ScalingDrawer*).
- Build (may fail until .cc updated).

### Task 1.4 — Draw(ScalingDrawer*) implementations and call sites

- In `game/entities.cc`: Implement all Draw(ScalingDrawer* pDr) (remove
  .get() or smart_pointer usage in body).
- In `game/level.cc`: Road::Draw, FancyRoad::Draw(ScalingDrawer*).
- In `game/fireball.cc`: Draw(ScalingDrawer*) overrides.
- In `game/critters.cc`: Draw(ScalingDrawer*) overrides.
- In `game/dragon.cc`: Dragon::Draw(ScalingDrawer*).
- In `game/controller/basic_controllers.cc`: entry.second->Draw(pGl->GetDrawer())
  (call site; GetDrawer() now returns raw ptr — no change to call).
- In `game/controller/menu_controller.cc`: Draw(ScalingDrawer*) and call sites.
- In `game/controller/level_controller.cc`: AdNumberDrawer, BonusDrawer
  Draw(ScalingDrawer*).
- In `game/controller/buy_now_controller.cc`: Draw(ScalingDrawer*).
- In `game/tutorial.cc`: TutorialTextEntity::Draw(ScalingDrawer*).
- Build and run ctest; run simulation_test.

### Task 1.5 — Remove SP_Info from Drawer and ScalingDrawer (CLEAN-01)

- In `game_utils/draw_utils.h` and `draw_utils.cc`: After no
  smart_pointer<ScalingDrawer> remains, remove SP_Info base from Drawer
  and ScalingDrawer. Remove #include "utils/smart_pointer.h" from
  draw_utils.h if no longer needed.
- Build and run ctest; run simulation_test.

---

## Wave 2: NumberDrawer

### Task 2.1 — TowerDataWrap: unique_ptr for NumberDrawer

- NumberDrawer already takes ScalingDrawer* (Task 1.1). In `dragon_game_runner.h`:
  Change TowerDataWrap to hold `std::unique_ptr<NumberDrawer> pNum` and
  `std::unique_ptr<NumberDrawer> pBigNum`.
- In `dragon_game_runner.cc`: Create NumberDrawer with
  std::make_unique<NumberDrawer>(pDr.get(), pFancyNum_.get(), ...) and
  std::make_unique<NumberDrawer>(pBigDr.get(), ...). Pass pNum.get() and
  pBigNum.get() to DragonGameController.
- Build and run ctest; run simulation_test.

### Task 2.2 — DragonGameController: store NumberDrawer* and return from getters

- In `dragon_game_controller.h`: Change pNum, pBigNum to NumberDrawer*.
  GetNumberDrawer() and GetBigNumberDrawer() return NumberDrawer*.
- In `dragon_game_controller.cc`: Ctor takes NumberDrawer* for pNum and
  pBigNum; store raw pointers; getters return them.
- Build and run ctest; run simulation_test.

### Task 2.3 — TextDrawEntity, TutorialTextEntity, MenuDisplay: NumberDrawer*

- In `game/entities.h`: TextDrawEntity: change smart_pointer<NumberDrawer> pNum
  to NumberDrawer*.
- In `game/tutorial.h`: TutorialTextEntity: pNum → NumberDrawer*.
- In `game/controller/menu_controller.h`: MenuDisplay: pNum → NumberDrawer*.
- In corresponding .cc files and call sites: pass GetNumberDrawer() or raw
  pointer from controller; no .get() on NumberDrawer.
- Build and run ctest; run simulation_test.

### Task 2.4 — Remove SP_Info from NumberDrawer (CLEAN-01)

- In `game_utils/draw_utils.h` and `draw_utils.cc`: Remove SP_Info base
  from NumberDrawer when no smart_pointer<NumberDrawer> remains. Remove
  smart_pointer include if no longer needed.
- Build and run ctest; run simulation_test.

---

## Wave 3: Leaf entities

Execute sub-waves 3a–3d in order. Run build and ctest; run simulation_test
after each task or small group.

### Wave 3a — Entities with no dependency on other smart_pointer entity types

### Task 3.1 — StaticRectangle and AddBackground (basic_controllers)

- In `game/controller/basic_controllers.cc`: AddBackground: create
  StaticRectangle with std::make_unique; use AddOwnedVisualEntity (or
  AddOwnedBoth) and store raw pointer in owned_visual_entities. Remove
  make_smart.
- Build and run ctest; run simulation_test.

### Task 3.2 — StaticImage (dragon_game_controller: pTrial, pBuyNow)

- In `game/controller/dragon_game_controller.cc`: Create pTrial, pBuyNow
  with std::make_unique<StaticImage>(...); add via AddOwnedVisualEntity or
  AddOwnedBoth; pass raw pointers where needed (e.g. pCnt1, pBuy).
- Build and run ctest; run simulation_test.

### Task 3.3 — Animation (dragon_game_controller, buy_now: mSlimes)

- In `game/controller/dragon_game_controller.cc`: Create pMenuCaret, pWin,
  pBurnL, pBurnR, pGolem, pSkeleton1–3, pMage, pGhost, pWhiteKnight with
  make_unique; add via AddOwnedVisualEntity/AddOwnedBoth; store raw in
  lists. Leave pMenuCaret shared with MenuDisplay for 3c.
- In `game/controller/buy_now_controller.cc`: mSlimes — change to
  vector<unique_ptr<Animation>> or store owned and raw in list; creation
  with make_unique, AddOwned* pattern.
- Build and run ctest; run simulation_test.

### Task 3.4 — AnimationOnce (dragon_game_controller, level_controller, critters, fireball, generators)

- Migration sites: dragon_game_controller.cc, level_controller.cc,
  critters.cc, fireball.cc, critter_generators.cc. At each creation site,
  use make_unique<AnimationOnce>(...); add via AddOwnedVisualEntity/
  AddOwnedEventEntity/AddOwnedBoth; store raw in lsDraw/lsUpdate. Ensure
  single owner (controller or level); pass raw to AddBoth/AddV.
- Build and run ctest; run simulation_test.

### Task 3.5 — HighScoreShower, IntroTextShower (basic_controllers)

- In `game/controller/basic_controllers.cc`: DragonScoreController ctor:
  create with make_unique; AddOwnedVisualEntity (or AddOwnedBoth); store
  raw in owned_visual_entities.
- Build and run ctest; run simulation_test.

### Task 3.6 — BonusScore (critters, level_controller)

- In `game/critters.cc`: Princess, Trader, Knight, Slime, etc.: create
  BonusScore with make_unique; pass to pAc->AddV(pB) or AddBoth — ensure
  ownership transferred to LevelController (AddOwned* and raw in list).
- In `game/controller/level_controller.cc`: If any BonusScore created here,
  same pattern. Ensure CleanUp for owned lists removes dead entities and
  keeps raw lists in sync.
- Build and run ctest; run simulation_test.

### Task 3.7 — AdNumberDrawer, BonusDrawer (level_controller Init)

- In `game/controller/level_controller.cc`: Init: create AdNumberDrawer and
  BonusDrawer with make_unique; add via AddOwnedVisualEntity; store raw in
  owned_visual_entities.
- Build and run ctest; run simulation_test.

### Task 3.8 — SimpleSoundEntity (dragon_game_controller)

- In `game/controller/dragon_game_controller.cc`: pOver, pPluSnd, pClkSnd —
  create with make_unique; add via AddE to controllers; store raw in
  lsUpdate. If SoundInterfaceProxy is still smart_pointer, SimpleSoundEntity
  can hold SoundInterfaceProxy* once DragonGameController passes raw (or
  defer until SoundInterfaceProxy migrated).
- Build and run ctest; run simulation_test.

### Task 3.9 — SoundControls (level_controller)

- In `game/controller/level_controller.h`: Change pSc from
  smart_pointer<SoundControls> to unique_ptr or owned in list; raw in
  lsUpdate.
- In `game/controller/level_controller.cc`: Create with make_unique; store
  as single owner; pass raw where needed.
- Build and run ctest; run simulation_test.

### Wave 3b — Entities that depend on NumberDrawer* (already raw after Wave 2)

### Task 3.10 — TextDrawEntity, TutorialTextEntity, MenuDisplay ownership

- TextDrawEntity, TutorialTextEntity, MenuDisplay already hold
  NumberDrawer* after Wave 2. In `dragon_game_controller.cc`,
  `level_controller.cc`, `menu_controller.cc`: create these entities with
  make_unique; add via AddOwnedVisualEntity/AddOwnedEventEntity/AddOwnedBoth;
  store raw in owned_visual_entities/owned_event_entities. Ensure
  LevelController owns TutorialTextEntity; MenuController owns MenuDisplay;
  MenuController or StartScreenController owns TextDrawEntity (pHintText,
  pOptionText).
- Build and run ctest; run simulation_test.

### Wave 3c — Shared reference (pMenuCaret)

### Task 3.11 — MenuDisplay and MenuController: single owner for pMenuCaret

- In `game/controller/menu_controller.h`: MenuDisplay holds Animation*
  pMenuCaret (non-owning). MenuController holds unique_ptr<Animation> for
  caret (or stores in owned list) and passes raw pointer to MenuDisplay
  ctor.
- In `game/controller/menu_controller.cc` and `dragon_game_controller.cc`:
  Create caret Animation with make_unique in MenuController; pass .get() to
  MenuDisplay ctor; add MenuDisplay and caret to lists with raw pointers.
- Build and run ctest; run simulation_test.

### Wave 3d — Entity base and SP_Info removal

### Task 3.12 — PhysicalEntity::HitDetection(PhysicalEntity*)

- In `game/entities.h`: Change HitDetection parameter from
  smart_pointer<PhysicalEntity> to PhysicalEntity*.
- In `game/entities.cc`: HitDetection implementation and call sites use
  raw pointer.
- Build and run ctest; run simulation_test.

### Task 3.13 — Remove SP_Info from Entity and migrated subclasses (CLEAN-01)

- After all leaf entity types are migrated off smart_pointer: In
  `game/entities.h` (and any subclass headers): Remove SP_Info base from
  Entity and from subclasses that no longer have smart_pointer<T> pointing
  to them. Do not remove from types deferred to Phase 4 (Critter, Dragon,
  Castle, Road, etc.). Remove get_class_name overrides only if they only
  served SP_Info. Remove #include "utils/smart_pointer.h" where no longer
  needed.
- Build and run ctest; run simulation_test.

### Task 3.14 — CleanUp and owned list sync (if needed)

- In `game/controller/basic_controllers.cc` (and any controller with
  owned_entities/owned_visual_entities/owned_event_entities): Ensure
  CleanUp for owned entities runs in sync: when removing an entity with
  !bExist, remove it from owned_entities and from the corresponding entry
  in owned_visual_entities and/or owned_event_entities in one pass so that
  no raw pointer in owned_* outlives its unique_ptr. Document the chosen
  sync strategy if needed.
- Build and run ctest; run simulation_test.

### Task 3.15 — Record skipped types for DOC-01 (success criterion 3)

- In this phase folder (or in .planning/smart_pointer_unmigrated.md): list
  types intentionally not migrated in Phase 3 (e.g. Critter, Dragon, Castle,
  Road, generators, TimedFireballBonus, ConsumableEntity, FancyCritter)
  with one-line rationale (e.g. "Phase 4" or "lifetime owned elsewhere").
  Ensures success criterion (3) is verifiable.
- No build change; optional doc-only step.

---

## Wave 4 (optional): ImageSequence

### Task 4.1 — Remove SP_Info from ImageSequence (CLEAN-01)

- In `game_utils/image_sequence.h`: ImageSequence has no smart_pointer<
  ImageSequence> in codebase. Remove SP_Info base from ImageSequence.
  Remove #include "utils/smart_pointer.h" if no longer needed.
- Build and run ctest; run simulation_test.

---

## Verification (every wave / task)

- From project root (code): sibling `build` dir. Run: `cd build; cmake
  ../code -G "MinGW Makefiles"; mingw32-make`. Build must succeed.
- Run: `ctest --output-on-failure` from build. All tests pass (VER-01).
- Run: `cd bin; .\simulation_test.exe` (or equivalent). simulation_test
  passes (VER-02).

## PLANNING COMPLETE

Phase 3 plan: 4 waves. Wave 1 = ScalingDrawer + Draw(ScalingDrawer*); Wave 2
= NumberDrawer; Wave 3 = Leaf entities (3a–3d); Wave 4 optional =
ImageSequence. Execute in order; verify after each task or at wave
boundaries. Do not migrate Phase 4 types (Critter, Dragon, Castle, Road,
generators, etc.); record any skipped types for DOC-01.
