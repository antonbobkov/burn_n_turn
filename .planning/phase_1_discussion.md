# Phase 1 Discussion: Process and leaf (game_utils)

**Phase:** 1 — Process and leaf (game_utils)  
**Goal:** Establish migration process; migrate Event and MessageWriter (and
concrete writers) to unique_ptr at owner, raw pointers elsewhere; run build
and tests after each step.

---

## Context gathered

### Event

- **Defined:** `game_utils/event.h` (Event, EmptyEvent, SwitchEvent, …);
  `event.cc` (Trigger, SequenceOfEvents::Trigger, TwoEvents).
- **Inherits:** SP_Info.
- **Created at:** Entry points only:
  - `simulation_test.cc`, `simulation.cc`: `make_smart(NewSwitchEvent(b_exit, b_true))`
  - `sdl_game_runner_main.cc`: `make_smart(NewSwitchEvent(bExit, bTrue))`
- **Passed into:** ProgramEngine constructor (smart_pointer<Event>).
- **Stored in:**
  - ProgramEngine::pExitProgram (game_runner_interface.h)
  - TowerDataWrap::pExitProgram (dragon_game_runner.h) — copy of ProgramEngine’s
- **Used (non-owning):**
  - dragon_game_runner.cc: `Trigger(pData->pExitProgram)` (exit path)
  - menu_controller.cc: `Trigger(pMenuController->pGl->pWrp->pExitProgram)` (Exit())
- **Special:** SequenceOfEvents has `std::vector<smart_pointer<Event>>` and
  TwoEvents(smart_pointer<Event>, smart_pointer<Event>). No current call sites
  for TwoEvents in codebase; SequenceOfEvents is “container of events” (burndown
  allows keeping smart_pointer in base-class-like containers).

**Lifetime:** Clear. The exit event is created once at program start, given to
ProgramEngine, and lives until the process ends. ProgramEngine is the single
logical owner. Today ProgramEngine is copied (by value) into GetGameRunner and
DragonGameRunner, so TowerDataWrap holds a second ref-counted copy; with
unique_ptr, only one owner is possible.

### MessageWriter

- **Defined:** `game_utils/MessageWriter.h` (MessageWriter, EmptyWriter,
  IoWriter, FileWriter).
- **Inherits:** SP_Info.
- **Created at:** Entry points:
  - simulation_test.cc, simulation.cc: `make_smart(new EmptyWriter())`
  - sdl_game_runner_main.cc: `make_smart(new IoWriter())`
- **Stored in:** ProgramEngine::pMsg.
- **Used (non-owning):** TowerDataWrap copies pe.pMsg into… it doesn’t store
  it; it does `pWr = pe.pMsg.get()` (global MessageWriter* pWr). So only
  ProgramEngine “holds” the writer; others use a raw pointer already.

**Lifetime:** Clear. Same as Event: created at entry, owned by ProgramEngine
for program lifetime.

### ProgramEngine and copying

- GetGameRunner(ProgramEngine pe) and DragonGameRunner(ProgramEngine pe) take
  ProgramEngine by value; TowerDataWrap(ProgramEngine pe) copies pe’s members.
- So today: ref-counted copies of Event and MessageWriter in both
  ProgramEngine and TowerDataWrap.
- With unique_ptr: ProgramEngine cannot be copied. So we must:
  - Have ProgramEngine hold unique_ptr<Event> and unique_ptr<MessageWriter>,
  - Pass ProgramEngine by const reference to GetGameRunner, DragonGameRunner,
    TowerDataWrap,
  - TowerDataWrap stores Event* and MessageWriter* (from the referenced
    ProgramEngine), and the referenced ProgramEngine (in main/simulation) must
    outlive the runner. It does.

---

## Approach (recommended)

1. **Process**
   - One type (or one logical group) at a time; run build + ctest (including
     simulation_test) after each change.
   - If lifetime of a game_utils type is not clear, do not migrate it; record
     it for the final unmigrated summary (DOC-01).

2. **MessageWriter first (slightly simpler)**
   - ProgramEngine: pMsg becomes std::unique_ptr<MessageWriter>; constructors
     take MessageWriter by rvalue (std::unique_ptr<MessageWriter>).
   - Entry points: create with std::make_unique<EmptyWriter>() /
     make_unique<IoWriter>() and std::move into ProgramEngine.
   - TowerDataWrap: take ProgramEngine by const reference; set pWr =
     pe.pMsg.get() (no storage of smart_pointer/unique_ptr).
   - ProgramEngine: remove copy ctor / copy assign (or make move-only); pass
     ProgramEngine by const reference everywhere it’s used (GetGameRunner,
     DragonGameRunner, TowerDataWrap).
   - Remove SP_Info from MessageWriter and concrete writers once no
     smart_pointer<MessageWriter> remains.

3. **Event second**
   - ProgramEngine: pExitProgram becomes std::unique_ptr<Event>; constructors
     take Event by rvalue (std::unique_ptr<Event>).
   - Entry points: create with std::make_unique<...>(NewSwitchEvent(...)) and
     std::move into ProgramEngine.
   - TowerDataWrap: pExitProgram becomes Event* (set from pe.pExitProgram.get()).
   - Trigger: add overload void Trigger(Event* pE) for non-owning use; keep
     void Trigger(smart_pointer<Event> pE) for SequenceOfEvents (vector still
     smart_pointer<Event> per burndown exception, or we can leave that for a
     later phase).
   - Call sites: dragon_game_runner.cc and menu_controller.cc already call
     Trigger with the stored event; they will pass Event*.
   - Remove SP_Info from Event (and Event subclasses that have no other
     smart_pointer uses) only when no smart_pointer<Event> remains. SequenceOfEvents
     still has vector<smart_pointer<Event>> — so we either keep that and don’t
     remove SP_Info from Event in Phase 1, or we migrate SequenceOfEvents to
     vector<unique_ptr<Event>> and then remove SP_Info from Event. Recommendation:
     keep vector<smart_pointer<Event>> in Phase 1 (burndown exception “array of
     base classes”); then SP_Info stays on Event until that container is
     migrated in a later phase. So Phase 1: Event is migrated at ProgramEngine
     and TowerDataWrap and call sites; SP_Info on Event is removed only when
     we also migrate SequenceOfEvents (could be Phase 1 if we do it, or defer).

   Actually: if we keep vector<smart_pointer<Event>>, we still have
   smart_pointer<Event> in the codebase, so we cannot remove SP_Info from
   Event. So for Phase 1 we have two sub-options:
   - **A)** Migrate only the “exit event” flow to unique_ptr + Event*; leave
     SequenceOfEvents as-is. Then SP_Info stays on Event (because
     SequenceOfEvents still uses smart_pointer<Event>). Phase 1 success
     criterion “at least one game_utils type … SP_Info removed” might then
     be satisfied by MessageWriter only.
   - **B)** Also migrate SequenceOfEvents to vector<unique_ptr<Event>> and
     TwoEvents to take unique_ptr, so that no smart_pointer<Event> remains,
     and then remove SP_Info from Event in Phase 1.

   Recommendation: **A** for Phase 1 (smaller, clear). Migrate exit-event flow
   and MessageWriter; leave SequenceOfEvents for a later phase; remove SP_Info
   from MessageWriter (and concrete writers) in Phase 1, and from Event only
   when SequenceOfEvents is migrated later.

4. **Order of code changes**
   - Make ProgramEngine take and store unique_ptr<MessageWriter>; make
     ProgramEngine non-copyable and pass by const reference; update all
     call sites and TowerDataWrap; then remove SP_Info from MessageWriter
     hierarchy. Build + test.
   - Make ProgramEngine take and store unique_ptr<Event> for exit event;
     TowerDataWrap holds Event*; add Trigger(Event*); update call sites; keep
     SequenceOfEvents as smart_pointer<Event>; do not remove SP_Info from
     Event yet. Build + test.
   - Optionally document “Event: SP_Info to be removed when SequenceOfEvents
     is migrated” in STATE or a small migration log.

---

## Risks and mitigations

| Risk | Mitigation |
|------|------------|
| ProgramEngine passed by value in many places | Grep all ProgramEngine uses; switch to const ProgramEngine& in one pass; build to catch copies. |
| Trigger(smart_pointer<Event>) vs Trigger(Event*) | Add overload Trigger(Event*); call sites that pass Event* use it; SequenceOfEvents still calls Trigger(smart_pointer<Event>) with vEv[i]. |
| Forgetting to run tests | Per PROC-02: run build + ctest after MessageWriter migration and after Event migration. |

---

## Open questions

1. **SequenceOfEvents in Phase 1 or later?** Recommended: leave as-is in Phase 1
   (vector<smart_pointer<Event>>), so Event keeps SP_Info until a later phase.
   Alternative: migrate SequenceOfEvents in Phase 1 and remove SP_Info from
   Event now.
2. **MakeSoundEvent** holds smart_pointer<SoundInterface<Index>> — out of
   scope for Phase 1 (Phase 2 wrappers). No change in Phase 1.
3. **GameRunner / GetGameRunner** still return smart_pointer<GameRunner>;
   Phase 5. No change in Phase 1.

---

## Summary

- **Lifetime:** Event (exit) and MessageWriter are created at entry and owned
  by ProgramEngine for the whole run. Clear.
- **Owner:** ProgramEngine. It must hold unique_ptr<Event> and
  unique_ptr<MessageWriter> and be non-copyable; pass by const reference
  everywhere.
- **Non-owners:** TowerDataWrap and call sites use Event* and MessageWriter*.
- **Scope Phase 1:** MessageWriter (+ concrete writers) and exit-Event flow;
  remove SP_Info from MessageWriter; leave Event’s SP_Info until
  SequenceOfEvents is migrated.
- **Process:** One group at a time (MessageWriter then Event), build + test
  after each; record any type skipped due to unclear lifetime.

---
*Discussion for Phase 1; use when running /gsd-plan-phase 1*
