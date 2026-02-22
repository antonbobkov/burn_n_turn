# Project State

**Project:** Smart Pointer Migration  
**See:** .planning/PROJECT.md (updated 2025-02-16)

**Core value:** The migration must preserve behavior: the project must keep
building and all tests (especially simulation_test) must pass after every
change.

**Current focus:** Phase 3 — Game entities and animations (plan 03 executed)

**Completed:** Phase 1 (Process and leaf game_utils). Phase 2 (Wrappers and
platform). Phase 3 Plan 03: basic_controllers (AddBackground, score showers),
MenuController (caret + MenuDisplay owned, GetNonOwned*), LevelController
(pTutorialText unique_ptr, GetNonOwned*), tutorial pTexter raw, entities
(HitDetection raw, SimpleSoundEntity pSnd raw, DGC pSnd unique_ptr), CLEAN-01
(SP_Info off Entity/MenuEntry/ImageSequence; on Event/ScreenEntity/FireballBonus),
CleanUp sync comment, DOC-01 unmigrated list, ImageSequence SP_Info removed.
Task 1.2 (DGC StartUp full migration) deferred.

**Next:** Complete Task 1.2 (DGC StartUp) or proceed to Phase 4.

---
Phase: 3 of 3 (Game entities and animations)
Plan: 03 of 1 (remaining work)
Status: Plan 03 complete (Task 1.2 deferred)
Last activity: 2025-02-22 — Completed 03-03-PLAN.md

Progress: ████████████████████░ (Phase 3 plan 03 done)

Session continuity:
Last session: 2025-02-22
Stopped at: Completed 03-03-PLAN.md
Resume file: None

---
*Last updated: 2025-02-22 after Phase 3 plan 03 execution*
