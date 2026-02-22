# Phase 3 Plan 03: Game entities and animations — Summary

**One-liner:** Leaf entities and controllers migrated to unique_ptr/raw where done;
HitDetection and SimpleSoundEntity raw; SP_Info trimmed; CleanUp and DOC-01 done.

---

## Frontmatter

- **Phase:** 3
- **Plan:** 03
- **Subsystem:** game entities, controllers
- **Tags:** unique_ptr, smart_pointer migration, SP_Info, CleanUp
- **requires:** Phase 2 (wrappers/platform)
- **provides:** Single-owner leaf entities in basic/menu/level controllers; raw
  HitDetection and SimpleSoundEntity; SP_Info only where refcounted; DOC-01 list
- **affects:** Phase 4 (remaining smart_pointer types)

---

## Completed tasks

| Task | Description | Commit |
|------|-------------|--------|
| 1.1 | basic_controllers: AddBackground, DragonScoreController (make_unique + AddOwned*) | a7aa004 |
| 1.3 | MenuController: pMenuCaret unique_ptr, MenuDisplay AddOwnedBoth, GetNonOwned* | 8f28f0b |
| 1.4 | LevelController: pTutorialText unique_ptr, GetNonOwned* return tutorial | 86cc813 |
| 1.5 | tutorial.h/cc: pTexter as TutorialTextEntity* | 86cc813 |
| 1.6 | entities: HitDetection(PhysicalEntity*) only; SimpleSoundEntity pSnd raw; DGC pSnd unique_ptr | 0af1567 |
| 2.1 | CLEAN-01: SP_Info removed from Entity, MenuEntry; added to EventEntity, ScreenEntity, FireballBonus | c918dc5 |
| 2.2 | CleanUp sync: comment in basic_controllers (raw lists before owned_entities) | 17ef855 |
| 2.3 | DOC-01: .planning/smart_pointer_unmigrated.md | 17ef855 |
| 3.1 | ImageSequence: remove SP_Info and smart_pointer include | 03491e2 |

---

## Remaining work (not done this run)

- **Task 1.2 — DGC StartUp:** SoundControls as unique_ptr; logo/burn by value
  (Option C) to pCnt1, pMenu, pBuy; StaticImage pTrial/pBuyNow, Animation
  (pWin, burn copies, pGolem, pSkeleton1–3, pMage, pGhost, pWhiteKnight),
  TextDrawEntity, AnimationOnce (pO, pPlu, pGen), SimpleSoundEntity (already
  passing pSnd.get()) — all via make_unique/AddOwned* and raw in lists. Left
  for a follow-up execution wave.

---

## Why Task 1.2 was deferred: problems and blockers

This section records the technical and practical reasons the executor did not
complete Task 1.2 (full DragonGameController::StartUp migration) in this run.

### 1. AddV / AddE interface and list types

- **Current state:** `EntityListController` (in `basic_controllers.h`) declares
  `AddV(smart_pointer<VisualEntity>)` and `AddE(smart_pointer<EventEntity>)`.
  The draw and update lists are `std::list<smart_pointer<VisualEntity>> lsDraw`
  and `std::list<smart_pointer<EventEntity>> lsUpdate`.
- **What Task 1.2 needs:** The plan says “pass raw to AddE(...)” and “store raw
  in lists”. So entities would be owned elsewhere (e.g. DGC or a controller via
  unique_ptr / AddOwned*) and only raw pointers would be passed into the
  controller and stored in lsDraw / lsUpdate.
- **Blocker:** There are no overloads `AddV(VisualEntity*)` or
  `AddE(EventEntity*)`, and the list types are `list<smart_pointer<...>>`, so
  raw pointers cannot be pushed as-is. Completing 1.2 would require either:
  - Changing lsDraw / lsUpdate to `std::list<VisualEntity*>` and
    `std::list<EventEntity*>` and adding AddV(VisualEntity*) / AddE(EventEntity*)
    overloads, or
  - Introducing a separate “non-owned” list and draw/update logic that iterates
    both owned and non-owned.
- **Impact:** Changing the list types and AddV/AddE affects the base controller
  and every place that adds to or iterates these lists (including Phase 4 code
  that still uses smart_pointer). The executor avoided this larger refactor in
  the same run.

### 2. Option C (logo and burn “by value”) — shared visuals, no shared ownership

- **Current state:** One `StaticImage` (logo, pL) and two `Animation`s
  (pBurnL, pBurnR) are created with make_smart and then passed to three
  controllers: StartScreenController (pCnt1), MenuController (pMenu), and
  BuyNowController (pBuy) via AddV / AddBoth. The same smart_pointer is
  shared, so refcounting gives shared ownership.
- **Plan (Option C):** “Create once (by value or local unique_ptr then copy);
  pass **copies** to pCnt1, pMenu, pBuy. No shared ownership.”
- **Blocker:** Implementing Option C means either:
  - **Copy semantics:** StaticImage and Animation must be copyable, and we
    create one object and pass copies to each controller. Copy constructors
    may not exist or may be non-trivial (e.g. image sequence references,
    internal state).
  - **Multiple instances:** Create three separate logos and six separate burn
    animations (three left, three right), and give each controller its own
    instance via AddOwned* (each controller owns one copy). That is a clear
    design but multiplies creation code and requires careful wiring (positions,
    sequence state) and more branches (e.g. TRIAL_VERSION).
- **Risk:** Without confirming copyability or choosing the “multiple instances”
  approach and implementing it consistently, Option C can introduce subtle
  lifetime or double-free bugs. The executor left this for a focused
  follow-up.

### 3. SoundControls: one owner, many consumers

- **Current state:** Two `SoundControls` (pBckgMusic, pNoMusic) are created with
  make_smart in StartUp and passed to many controllers via AddE: pCnt1, pMenu,
  pCut1–pCut3, pCnt2, pCnt3, and every LevelController (plus pAd->pSc =
  pBckgMusic.get()).
- **Plan:** DragonGameController owns both (e.g. two unique_ptrs); all others
  receive raw pointers (e.g. AddE(pBckgMusic.get()) and pSc = pBckgMusic.get()).
- **Blocker:** AddE currently takes `smart_pointer<EventEntity>`. To pass raw
  pointers, we need either AddE(EventEntity*) or another way to register
  non-owned entities (see §1). So the SoundControls part of 1.2 is blocked by
  the same AddE/list design as above. In addition, DGC would need two new
  members (e.g. std::unique_ptr<SoundControls> pBckgMusic_, pNoMusic_) and
  correct lifetime (e.g. created in StartUp, live for the whole game). That
  part is straightforward once the AddE/raw-list design is in place.

### 4. Scope and interdependencies

- **Number of entities:** StartUp creates and wires many leaf entities: pStr,
  pBckgMusic, pNoMusic, pWin, pL, pBurnL, pBurnR, pHintText, pOptionText, pO,
  pPlu, pGen, pOver, pPluSnd, pClkSnd, pTrial, pGolem, pSkeleton1–3, pMage,
  pGhost, pWhiteKnight, pBuyNow, pSlimeUpd, plus MenuDisplay and caret (done in
  1.3). Each has a designated owner and one or more consumers.
- **Number of controllers:** pCnt0_1, pCnt0_2, pCnt1, pCnt2, pCnt3, pMenu,
  pCut1–pCut3, pScore, pIntro, every LevelController, and (in TRIAL_VERSION)
  pBuy. One mistake in “who owns” or “who gets raw” can cause use-after-free or
  double delete.
- **Conditional compilation:** TRIAL_VERSION and FULL_VERSION change which
  controllers and entities are created (e.g. pTrial, pBuy, pBuyNow, pGolem,
  etc.). Any migration must be correct on both paths and keep build and
  simulation_test green.
- **Executor choice:** Doing 1.2 fully in one go would require solving §1 and §2
  and then touching a large block of StartUp. The executor deferred 1.2 to
  keep the run manageable and to avoid breaking the build or tests.

### 5. What was done instead (Task 1.6 overlap)

- SimpleSoundEntity and DGC’s sound proxy were migrated: DGC holds
  `std::unique_ptr<SoundInterfaceProxy> pSnd` and passes `pSnd.get()` to
  SimpleSoundEntity constructors. So the “owner holds unique_ptr, entity holds
  raw pointer” pattern is in place for sound. The rest of 1.2 (SoundControls,
  logo/burn, StaticImage, Animation, TextDrawEntity, AnimationOnce, and
  list/AddV/AddE design) was left for a follow-up.

### 6. Summary for a future run

To finish Task 1.2, a future run should:

1. **Decide and implement the list/AddV/AddE design:** Either change
   lsDraw/lsUpdate to raw-pointer lists and add AddV(VisualEntity*) /
   AddE(EventEntity*) (and update all iterators and CleanUp), or introduce
   a documented “non-owned” path that does not rely on smart_pointer in
   those lists.
2. **Implement Option C for logo and burn:** Either add (or use) copy
   constructors for StaticImage/Animation and pass copies, or create
   separate instances per controller and add each via AddOwned*.
3. **Migrate DGC StartUp in order:** Store SoundControls in DGC as
   unique_ptr; create all other leaf entities with make_unique; add via
   AddOwned* where the controller owns, and pass raw pointers for
   non-owned registration; keep conditional compilation (TRIAL/FULL) and
   run build, ctest, and simulation_test after each logical step.

---

## Decisions made

- **MenuController ownership:** MenuController owns pMenuCaret (unique_ptr) and
  MenuDisplay (via AddOwnedBoth); MenuDisplay ctor takes Animation* and
  MenuController*; caret drawn/updated via GetNonOwnedDrawEntities /
  GetNonOwnedUpdateEntities.
- **pHintText / pOptionText:** Kept as smart_pointer in MenuController so
  lifetime is correct (no dangling after StartUp).
- **Entity / SP_Info:** Entity no longer inherits SP_Info; EventEntity and
  ScreenEntity do (for lsDraw/lsUpdate refcounting); Entity has virtual
  get_class_name() for overrides.
- **FireballBonus:** Explicit SP_Info added so smart_pointer<TimedFireballBonus>
  refcounting remains valid.

---

## Deviations from plan

- **Task 1.2 deferred:** Full DragonGameController::StartUp migration (SoundControls,
  logo/burn Option C, StaticImage, Animation, TextDrawEntity, AnimationOnce,
  SimpleSoundEntity ownership) was not executed; only SimpleSoundEntity and DGC
  pSnd (unique_ptr) were done in Task 1.6. Rest is documented as remaining.
  See **Why Task 1.2 was deferred** below for a full list of problems and
  blockers.
- **Rule 2 (minor):** Defensive null check in GetNonOwnedDrawEntities loop
  (basic_controllers) and pMenuDisplay guard in MenuController::Update and
  Next() to avoid crashes during migration.

---

## Key files

- **Created:** .planning/smart_pointer_unmigrated.md
- **Modified:** code/game/entities.h, entities.cc; code/game/controller/
  basic_controllers.cc, dragon_game_controller.h, dragon_game_controller.cc,
  menu_controller.h, menu_controller.cc, level_controller.h, level_controller.cc;
  code/game/tutorial.h, tutorial.cc; code/game/fireball.h; code/game/critters.cc,
  fireball.cc, dragon.cc; code/game_utils/image_sequence.h

---

## Verification

- Build: `cd build; mingw32-make` — success
- Tests: `ctest --output-on-failure` — all pass
- simulation_test: `cd bin; .\simulation_test.exe` — pass

---

## Next phase readiness

- Phase 4 can proceed with Critter, Dragon, Castle, Road, generators,
  TimedFireballBonus, ConsumableEntity, FancyCritter, etc., as listed in
  smart_pointer_unmigrated.md.
- Completing Task 1.2 in a later run will finish DGC StartUp leaf-entity
  migration.
