# Project State

**Project:** Smart Pointer Migration  
**See:** .planning/PROJECT.md (updated 2025-02-16)

**Core value:** The migration must preserve behavior: the project must keep
building and all tests (especially simulation_test) must pass after every
change.

**Current focus:** Phase 4 — Critters, generators, fireball, level

**Completed:** Phase 1 (Process and leaf game_utils). Phase 2 (Wrappers and
platform). Phase 3 (Game entities and animations): Plan 03 and Plan 01.2 done
— basic_controllers, MenuController (caret, MenuDisplay unique_ptr, GetNonOwned*),
LevelController (pTutorialText, GetNonOwned*), tutorial pTexter raw, entities
(HitDetection/SimpleSoundEntity raw), DGC StartUp (copyable logo/burn/SoundControls
by value, all leaf entities make_unique + AddOwned*, MenuDisplay in MenuController),
CLEAN-01, CleanUp sync, DOC-01, ImageSequence SP_Info removed.

**Next:** Phase 4 — Critters, generators, fireball, level.

---
Phase: 3 complete (6 phases total)
Last activity: 2025-02-21 — Phase 3 Plan 01.2 executed; verification passed

Progress: █████████████████████ (Phase 3 complete)

---
*Last updated: 2025-02-21 after Phase 3 execution and verification*
