# Phase 1 process and deferrals

**Phase 1 completed:** Process and leaf (game_utils)

## Process followed (PROC-01, PROC-02, PROC-03)

- **Bottom-up:** MessageWriter (no ownership of other smart_pointer types) was
  migrated first, then Event (exit flow). Both are leaf game_utils types.
- **One group at a time:** MessageWriter migration (Steps 2–3) was done, then
  build + ctest run; then Event migration (Step 4), then build + ctest.
- **Skip and record when lifetime unclear:** No type was skipped in Phase 1;
  Event and MessageWriter lifetimes are clear (created at entry, owned by
  ProgramEngine for program lifetime). Any type whose lifetime cannot be
  determined with confidence in future phases will be recorded in
  .planning/smart_pointer_unmigrated.md for the final summary (DOC-01).

## Deferral: Event and SP_Info

- **Event** still derives from **SP_Info** after Phase 1.
- **Reason:** SequenceOfEvents uses `std::vector<smart_pointer<Event>>` (and
  TwoEvents takes smart_pointer<Event>). Per burndown doc, base-class-like
  containers may keep smart_pointer; we left that for a later phase.
- **Plan:** Remove SP_Info from Event when SequenceOfEvents is migrated (e.g.
  to vector<unique_ptr<Event>>) in a later phase.

---
*Phase 1 execution: 2025-02-16*
