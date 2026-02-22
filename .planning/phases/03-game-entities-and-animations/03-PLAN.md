# Phase 3: Game entities and animations — Plan

---
wave: 1
depends_on: ["02-wrappers-and-platform"]
files_modified:
  - code/game/entities.h
  - code/game/entities.cc
  - code/game/controller/basic_controllers.h
  - code/game/controller/basic_controllers.cc
  - code/game/controller/dragon_game_controller.h
  - code/game/controller/dragon_game_controller.cc
  - code/game/controller/menu_controller.h
  - code/game/controller/menu_controller.cc
  - code/game/controller/level_controller.h
  - code/game/controller/level_controller.cc
  - code/game/tutorial.h
  - code/game/tutorial.cc
  - code/game_utils/image_sequence.h
autonomous: true
---

**Phase:** 3 — Game entities and animations  
**Goal (ROADMAP):** Migrate leaf game types: Entity hierarchy, Drawer/
NumberDrawer and concrete drawers, AnimationOnce, StaticImage, Animation,
simple entities. Ownership lives in level, controllers, or critters.

**Status:** Phase 3 is **largely complete**. Tasks 1.1, 1.3–1.6 and Waves 2–3
are done (see 03-03-SUMMARY.md). **Remaining work is Task 1.2 only** (DGC
StartUp: SoundControls and logo/burn by value; remaining entities to
make_unique + AddOwned*). Execute **03-01.2-PLAN.md** for that.

## must_haves (goal-backward verification)

- [ ] Leaf entities in basic_controllers, DragonGameController, MenuController,
  LevelController use single owner (unique_ptr/AddOwned*) and raw pointers
  in lists; logo/burn passed by value (Option C); MenuController owns
  pMenuCaret and overrides GetNonOwnedDrawEntities/GetNonOwnedUpdateEntities;
  LevelController owns TutorialTextEntity and overrides GetNonOwnedDrawEntities.
- [ ] PhysicalEntity::HitDetection(smart_pointer<PhysicalEntity>) removed;
  HitDetection(PhysicalEntity*) only. SimpleSoundEntity holds
  SoundInterfaceProxy* where owner passes raw.
- [ ] SP_Info removed from Entity and migrated subclasses where no
  smart_pointer use remains (CLEAN-01); Phase 4 types not touched.
- [ ] CleanUp keeps owned_* and lsDraw/lsUpdate in sync when removing
  !bExist entities; strategy documented if needed.
- [ ] Build succeeds; ctest and simulation_test pass (VER-01, VER-02).
- [ ] Phase 4 / skipped types recorded for DOC-01.

---

## Wave 1: Remaining leaf entities

Execute in order. Run build and ctest; run simulation_test after each task
(or after a small group). VER-01, VER-02.

### Task 1.1 — basic_controllers: AddBackground and score showers

- In `game/controller/basic_controllers.cc`: AddBackground: create
  StaticRectangle with std::make_unique<StaticRectangle>(...); use
  AddOwnedVisualEntity(...) and store raw pointer in list. Remove
  make_smart and AddV(pBkg).
- In same file: DragonScoreController ctor — HighScoreShower and
  IntroTextShower: create with std::make_unique; use AddOwnedVisualEntity
  (or AddOwnedBoth); store raw in list. Remove make_smart.
- Build and run ctest; run simulation_test.

### Task 1.2 — DragonGameController::StartUp (remaining)

**Execute 03-01.2-PLAN.md** (not this section). The detailed plan there
reflects CONTEXT and 03-01.2-RESOLUTION-OPTIONS: SoundControls and
logo/burn **by value** (copies; each controller owns); no list redesign;
copyability for StaticImage, Animation, SoundControls. Tasks: copyability,
SoundControls by value, logo/burn by value, remaining StartUp entities.

### Task 1.3 — MenuController: pMenuCaret, pMenuDisplay, pHintText, pOptionText

- In `menu_controller.h`: Change pMenuCaret to std::unique_ptr<Animation>;
  pMenuDisplay to std::unique_ptr<MenuDisplay> or raw if ownership stays
  in DGC; pHintText, pOptionText to TextDrawEntity* (non-owning) or
  unique_ptr if MenuController owns. MenuDisplay ctor takes Animation*
  pMenuCaret_ (non-owning).
- In `menu_controller.cc` and `dragon_game_controller.cc`: Create
  pMenuCaret in MenuController with make_unique; pass pMenuCaret.get()
  to MenuDisplay ctor. MenuController overrides GetNonOwnedDrawEntities
  and GetNonOwnedUpdateEntities to return the caret so the base draw/
  update loop includes it. Do not add caret via AddBoth (raw pointer).
- Ensure MenuDisplay and TextDrawEntity ownership is clear (either
  MenuController or DGC owns; the other holds raw).
- Build and run ctest; run simulation_test.

### Task 1.4 — LevelController: TutorialTextEntity ownership and draw

- In `level_controller.h`: Change pTutorialText from
  smart_pointer<TutorialTextEntity> to std::unique_ptr<TutorialTextEntity>.
- In `level_controller.cc` Init: Create TutorialTextEntity with
  std::make_unique<TutorialTextEntity>(...); assign to pTutorialText.
  Assign pTutorialText.get() to tutOne->pTexter and tutTwo->pTexter (raw).
- Override GetNonOwnedDrawEntities in LevelController to return the
  tutorial text entity so the base draw loop draws it.
- Build and run ctest; run simulation_test.

### Task 1.5 — tutorial.h: pTexter as non-owning pointer

- In `tutorial.h`: In TutorialLevelOne and TutorialLevelTwo, change
  smart_pointer<TutorialTextEntity> pTexter to TutorialTextEntity*
  pTexter (non-owning).
- In `tutorial.cc`: Replace pTexter.is_null() checks with (pTexter !=
  nullptr); call pTexter->SetText(...) unchanged.
- Build and run ctest; run simulation_test.

### Task 1.6 — entities: HitDetection(PhysicalEntity*) and SimpleSoundEntity pSnd

- In `entities.h`: Remove HitDetection(smart_pointer<PhysicalEntity> pPh);
  keep only HitDetection(PhysicalEntity* pPh). SimpleSoundEntity: change
  smart_pointer<SoundInterfaceProxy> pSnd to SoundInterfaceProxy* pSnd
  (non-owning). Ctor and call sites: owner (DragonGameController) passes
  raw pointer.
- In `entities.cc`: HitDetection implementation and all call sites use
  PhysicalEntity* only. SimpleSoundEntity ctor takes SoundInterfaceProxy*.
- In `dragon_game_controller.cc`: Where SimpleSoundEntity is constructed,
  pass raw SoundInterfaceProxy* (e.g. from a unique_ptr or owned proxy
  held by DGC).
- Build and run ctest; run simulation_test.

---

## Wave 2: Entity base cleanup, CleanUp sync, DOC-01

### Task 2.1 — Remove SP_Info from Entity and migrated subclasses (CLEAN-01)

- In `game/entities.h` (and subclass headers as needed): Remove SP_Info
  base from Entity and from subclasses that no longer have any
  smart_pointer<T> pointing to them. Do not remove from types deferred to
  Phase 4 (Critter, Dragon, Castle, Road, generators, etc.). Remove
  get_class_name overrides only if they only served SP_Info. Remove
  #include "utils/smart_pointer.h" where no longer needed.
- Build and run ctest; run simulation_test.

### Task 2.2 — CleanUp and owned list sync

- In controllers that own entities (basic_controllers, dragon_game_controller,
  menu_controller, level_controller): Ensure CleanUp removes entities with
  !bExist from owned_* and from lsDraw/lsUpdate in one pass so no raw
  pointer in a list outlives its unique_ptr. Document the sync strategy
  (e.g. same pass, or clear raw list when owned is cleared) in code or
  in this phase folder if non-obvious.
- Build and run ctest; run simulation_test.

### Task 2.3 — Record skipped types (DOC-01)

- In this phase folder or in `.planning/smart_pointer_unmigrated.md`:
  list types intentionally not migrated in Phase 3 (e.g. Critter, Dragon,
  Castle, Road, generators, TimedFireballBonus, ConsumableEntity,
  FancyCritter) with a one-line rationale (e.g. "Phase 4" or "lifetime
  owned elsewhere"). Ensures success criterion (3) is verifiable.
- No build change; doc-only.

---

## Wave 3 (optional): ImageSequence

### Task 3.1 — Remove SP_Info from ImageSequence (CLEAN-01)

- In `game_utils/image_sequence.h`: If no smart_pointer<ImageSequence>
  remains in the codebase, remove SP_Info base from ImageSequence and
  remove #include "utils/smart_pointer.h" if no longer needed.
- Build and run ctest; run simulation_test.

---

## Verification (every wave / task)

- From project root (code): sibling `build` dir. Run: `cd build; cmake
  ../code -G "MinGW Makefiles"; mingw32-make`. Build must succeed.
- Run: `ctest --output-on-failure` from build. All tests pass (VER-01).
- Run: `cd bin; .\simulation_test.exe` (or equivalent). simulation_test
  passes (VER-02).

---

## PLANNING COMPLETE

Phase 3: Tasks 1.1, 1.3–1.6 and Waves 2–3 are **done** (03-03-SUMMARY.md).
**Remaining:** Task 1.2 only — see **03-01.2-PLAN.md** for DGC StartUp
(SoundControls and logo/burn by value; copyability; remaining entities
to make_unique + AddOwned*). Execute that plan to finish Phase 3.
