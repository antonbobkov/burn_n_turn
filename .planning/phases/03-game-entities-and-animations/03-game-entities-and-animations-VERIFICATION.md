---
phase: 03-game-entities-and-animations
verified: 2025-02-21T00:00:00Z
status: passed
score: 5/5 must-haves verified
---

# Phase 3: Game entities and animations Verification Report

**Phase Goal:** Migrate leaf game types: Entity hierarchy, Drawer/NumberDrawer
and concrete drawers, AnimationOnce, StaticImage, Animation, simple entities.
Ownership lives in level, controllers, or critters.

**Verified:** 2025-02-21
**Status:** passed
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths (must-haves from 03-01.2-PLAN.md)

| # | Must-have | Status | Evidence |
|---|-----------|--------|----------|
| 1 | StaticImage and Animation copyable; logo and burn by value per controller via AddOwned* | ✓ VERIFIED | entities.h:130-132, 163-164 (copy ctor/assign). dragon_game_controller.cc:393-398 (logo, burnL, burnR locals), 439-441 (pCnt1), 443-445 (pMenu), 530-532 (pBuy) AddOwnedVisualEntity/AddOwnedBoth with copies |
| 2 | SoundControls copyable; copy per controller via AddOwnedEventEntity | ✓ VERIFIED | entities.h:303-304 (copy ctor/assign). dragon_game_controller.cc:389-390 (bckgTemplate, noMusicTemplate), 480-487 (pCnt1, pMenu, pCut1-3, pCnt2, pCnt3), 502-504 (each LevelController) |
| 3 | All other StartUp leaf entities (pTrial, pBuyNow, pWin, pGolem, pSkeleton1-3, pMage, pGhost, pWhiteKnight, pHintText, pOptionText, pO, pPlu, pGen, pOver, pPluSnd, pClkSnd, pSlimeUpd) created with make_unique and added via AddOwned*; no smart_pointer in StartUp for these | ✓ VERIFIED | dragon_game_controller.cc:427-428 (pTrial), 455-456/452-453 (pHintText/pOptionText), 459-477 (pWin, pOver, pPlu, pGen, pPluSnd, pClkSnd), 530-555 (pBuy: logo, burn, SlimeUpd, pBuyNow, golem, skelly×3, mage, ghost, ghost_knight). No smart_pointer to these; only make_unique + AddOwned* or Set* |
| 4 | MenuDisplay: MenuController owns via explicit std::unique_ptr; included in GetNonOwned* overrides | ✓ VERIFIED | menu_controller.h:119 (std::unique_ptr<MenuDisplay> pMenuDisplay). dragon_game_controller.cc:434 (SetMenuDisplay). menu_controller.cc:80-87 (GetNonOwnedUpdateEntities), 89-96 (GetNonOwnedDrawEntities) push pMenuDisplay.get() |
| 5 | Build succeeds; ctest and simulation_test pass | ✓ VERIFIED | mingw32-make exit 0; ctest 6/6 passed; bin/simulation_test.exe 11 assertions, 2 test cases passed |

**Score:** 5/5 must-haves verified

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| code/game/entities.h | StaticImage, Animation, SoundControls copyable | ✓ | Copy ctors and assign at 130-132, 163-164, 303-304 |
| code/game/controller/dragon_game_controller.cc | StartUp: by-value logo/burn/SoundControls; make_unique + AddOwned* for leaf entities | ✓ | Lines 389-398, 422-555; no smart_pointer for leaf entities in StartUp |
| code/game/controller/menu_controller.h | std::unique_ptr<MenuDisplay> | ✓ | Line 119 |
| code/game/controller/menu_controller.cc | GetNonOwnedUpdateEntities, GetNonOwnedDrawEntities include pMenuDisplay | ✓ | Lines 80-96 |

### Key Link Verification

| From | To | Via | Status | Details |
|------|-----|-----|--------|---------|
| StartUp | pCnt1, pMenu, pBuy | AddOwnedVisualEntity(logo copy), AddOwnedBoth(burn copy) | ✓ | Each controller owns its copy |
| StartUp | pCnt1, pMenu, pCut1-3, pCnt2, pCnt3, each pAd | AddOwnedEventEntity(SoundControls copy) | ✓ | Per-controller SoundControls |
| StartUp | MenuController | SetMenuDisplay(std::make_unique<MenuDisplay>(...)) | ✓ | No AddOwnedBoth; explicit ownership |
| MenuController | draw/update loop | GetNonOwnedDrawEntities, GetNonOwnedUpdateEntities return pMenuDisplay.get(), pMenuCaret.get() | ✓ | Base loop draws/updates MenuDisplay |

### Success criteria (phase-level)

1. **Leaf game entities and animations with clear single owner migrated; SP_Info removed where no smart_pointer<T> remains.**  
   StartUp no longer holds smart_pointer to logo, burn, SoundControls, pTrial, pBuyNow, pWin, pGolem, skeletons, pMage, pGhost, pWhiteKnight, pHintText, pOptionText, pO/pPlu/pGen, pOver/pPluSnd/pClkSnd, pSlimeUpd; all are make_unique + AddOwned* or Set*. SP_Info remains on EventEntity/ScreenEntity (entities.h) for types still used in smart_pointer elsewhere (e.g. level/critters); for migrated StartUp leaf types no smart_pointer remains in StartUp.

2. **Build and all tests pass.**  
   Verified: build succeeded; ctest 6/6; simulation_test 11 assertions passed.

3. **Any type skipped (lifetime unclear) recorded for final summary.**  
   No skipped-type list found in phase dir; PLAN 03-01.2 scope (Task 1.2) did not require one. Broader Phase 3 PLAN/CHECK mention recording skipped types for DOC-01; not in 03-01.2 must_haves.

### Anti-Patterns Found

None that block the Phase 3 / Task 1.2 goal. smart_pointer still used for controllers (pMenuHolder, pCnt0_1, pBuy, pAd, etc.) and in level/critters/dragon — expected per phase scope.

### Human Verification Required

None required for the stated must-haves. Visual/gameplay checks (logo and burn display, menu, sounds) would be human-only if desired.

### Gaps Summary

None. All five must-haves from 03-01.2-PLAN.md are satisfied.

---

_Verified: 2025-02-21_
_Verifier: Claude (gsd-verifier)_
