# Project State

**Project:** Smart Pointer Migration  
**See:** .planning/PROJECT.md (updated 2025-02-16)

**Core value:** The migration must preserve behavior: the project must keep
building and all tests (especially simulation_test) must pass after every
change.

**Current focus:** Phase 2 — Wrappers and platform types

**Completed:** Phase 1 (Process and leaf game_utils). MessageWriter migrated
to unique_ptr, SP_Info removed. Event (exit) migrated to unique_ptr at
ProgramEngine, Event* elsewhere; Event keeps SP_Info until SequenceOfEvents
migrated. Build and all tests pass.

**Next:** /gsd-discuss-phase 2 or /gsd-plan-phase 2 for wrappers and platform
types.

---
*Last updated: 2025-02-16 after Phase 1 execution*
