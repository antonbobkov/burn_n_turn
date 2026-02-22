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
