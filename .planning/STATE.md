# Project State

**Project:** Smart Pointer Migration  
**See:** .planning/PROJECT.md (updated 2025-02-16)

**Core value:** The migration must preserve behavior: the project must keep
building and all tests (especially simulation_test) must pass after every
change.

**Current focus:** Phase 3 — Game entities and animations

**Completed:** Phase 1 (Process and leaf game_utils). Phase 2 (Wrappers and
platform): GraphicalInterface, SoundInterface, FontWriter and their
implementations/proxies migrated to unique_ptr at entry (main) or
TowerDataWrap (FontWriter); rest use T*. SP_Info removed from GUI/Sound/FontWriter.
Build and all tests pass.

**Next:** /gsd-discuss-phase 3 or /gsd-plan-phase 3 for game entities and
animations.

---
*Last updated: 2025-02-16 after Phase 2 execution*
