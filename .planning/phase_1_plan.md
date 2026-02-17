# Phase 1 Plan: Process and leaf (game_utils)

**Phase:** 1  
**Goal:** Establish process; migrate MessageWriter and Event (exit flow) to
unique_ptr at owner, raw pointers elsewhere; run build and tests after each
step.  
**Reference:** .planning/phase_1_discussion.md, .planning/ROADMAP.md

---

## Success criteria (from roadmap)

1. Process agreed: bottom-up, per-step tests, skip and record when lifetime unclear.
2. At least one game_utils type migrated and SP_Info removed (MessageWriter).
3. Build and ctest (including simulation_test) pass after changes.
4. Any type skipped due to unclear lifetime recorded for final summary.

---

## Plan overview

| Step | What | Verify |
|------|------|--------|
| 1 | Make ProgramEngine non-copyable and pass-by-const-ref | Build |
| 2 | Migrate MessageWriter to unique_ptr in ProgramEngine; entry points use make_unique + move | Build + ctest |
| 3 | Remove SP_Info from MessageWriter hierarchy | Build + ctest |
| 4 | Migrate Event (exit) to unique_ptr in ProgramEngine; TowerDataWrap holds Event*; add Trigger(Event*) | Build + ctest |
| 5 | Document process; note Event keeps SP_Info until SequenceOfEvents migrated | — |

---

## Step 1: ProgramEngine pass-by-const-reference

ProgramEngine will hold unique_ptr for Event and MessageWriter, so it must be
non-copyable. Switch all “take ProgramEngine by value” to “take by const
reference” first so the rest of the plan only adds unique_ptr and move semantics.

### 1.1 game_runner_interface.h

- Add `#include <memory>` if not present (for later unique_ptr; optional in step 1).
- After the two ProgramEngine constructors, add:
  - `ProgramEngine(ProgramEngine const&) = delete;`
  - `ProgramEngine& operator=(ProgramEngine const&) = delete;`
- Change declaration: `smart_pointer<GameRunner> GetGameRunner(ProgramEngine pe);`  
  → `smart_pointer<GameRunner> GetGameRunner(ProgramEngine const& pe);`

### 1.2 game_runner_interface_impl.cc

- Change definition: `GetGameRunner(ProgramEngine pe)` → `GetGameRunner(ProgramEngine const& pe)`.
- Call `DragonGameRunner(pe)` unchanged (pe is now const ref).

### 1.3 dragon_game_runner.h

- Change `TowerDataWrap(ProgramEngine pe);` → `TowerDataWrap(ProgramEngine const& pe);`
- Change `DragonGameRunner(ProgramEngine pe);` → `DragonGameRunner(ProgramEngine const& pe);`

### 1.4 dragon_game_runner.cc

- Change `DragonGameRunner::DragonGameRunner(ProgramEngine pe)` →  
  `DragonGameRunner::DragonGameRunner(ProgramEngine const& pe)`.
- Change `TowerDataWrap::TowerDataWrap(ProgramEngine pe)` →  
  `TowerDataWrap::TowerDataWrap(ProgramEngine const& pe)`.
- In TowerDataWrap ctor, keep `pExitProgram = pe.pExitProgram;` and `pWr = pe.pMsg.get();` for now (still smart_pointer / unique_ptr later).

### 1.5 Call sites (no signature change yet)

- simulation_test.cc (both TEST_CASE blocks): `ProgramEngine pe(...)` and `GetGameRunner(pe)` / `DragonGameRunner(pe)` — no change; pe is passed by ref now.
- simulation.cc: same.
- sdl_game_runner_main.cc: `GetGameRunner(pe)` — no change.

**Verify:** Build from `build/`: `mingw32-make`. Fix any compile errors (e.g. accidental copy of ProgramEngine).

---

## Step 2: Migrate MessageWriter to unique_ptr

### 2.1 game_runner_interface.h

- Add `#include <memory>`.
- Change `smart_pointer<MessageWriter> pMsg;` → `std::unique_ptr<MessageWriter> pMsg;`
- In first constructor: change parameter `smart_pointer<MessageWriter> pMsg_` →  
  `std::unique_ptr<MessageWriter> pMsg_`; initializer `pMsg(pMsg_)` → `pMsg(std::move(pMsg_))`.
- In second constructor: same parameter and initializer change.

### 2.2 Entry points: create and move MessageWriter

- **simulation_test.cc** (both places where ProgramEngine is constructed):
  - Change `smart_pointer<MessageWriter> p_msg = make_smart(new EmptyWriter());`  
    → `auto p_msg = std::make_unique<EmptyWriter>();`
  - Change `ProgramEngine pe(p_exit_ev, p_gr, p_snd, p_msg, ...)`  
    → `ProgramEngine pe(p_exit_ev, p_gr, p_snd, std::move(p_msg), ...)`  
  - Add `#include <memory>` if not present.
- **simulation.cc**:
  - Same: `auto p_msg = std::make_unique<EmptyWriter>();` and `std::move(p_msg)` in ProgramEngine ctor.
- **sdl_game_runner_main.cc**:
  - Change `make_smart(new IoWriter())` to `std::make_unique<IoWriter>()` and pass it into ProgramEngine constructor (move). Constructor call: pass `std::move(...)` for the message writer argument.
  - Add `#include <memory>` if not present.

### 2.3 TowerDataWrap

- TowerDataWrap(ProgramEngine const& pe) already uses `pWr = pe.pMsg.get();` — no change (pMsg is now unique_ptr, .get() still valid).

**Verify:** Build; run `ctest --output-on-failure` from build/, including simulation_test.

---

## Step 3: Remove SP_Info from MessageWriter hierarchy

### 3.1 MessageWriter.h

- Remove `#include "utils/smart_pointer.h"` if it exists and is only used for SP_Info.
- Change `class MessageWriter : virtual public SP_Info` → `class MessageWriter {` (or keep virtual if needed for other diamond inheritance; if no other base, drop SP_Info only).
- Remove `get_class_name()` override if it was only for SP_Info / smart_pointer tooling. If it’s used elsewhere, keep the method but remove the override of the SP_Info-style name (or leave a simple get_class_name() for debugging).
- Check EmptyWriter, IoWriter, FileWriter: remove `get_class_name()` if it was only for SP_Info; remove SP_Info from inheritance.

### 3.2 Grep and fix

- `grep -r "smart_pointer<MessageWriter>\|make_smart.*MessageWriter\|make_smart.*EmptyWriter\|make_smart.*IoWriter" code/` — should find no remaining uses after Step 2.
- Any file that included MessageWriter.h only for SP_Info: no change needed once SP_Info is removed.

**Verify:** Build; ctest (including simulation_test).

---

## Step 4: Migrate Event (exit) to unique_ptr

### 4.1 game_runner_interface.h

- Change `smart_pointer<Event> pExitProgram;` → `std::unique_ptr<Event> pExitProgram;`
- In both constructors: parameter `smart_pointer<Event> pExitProgram_` →  
  `std::unique_ptr<Event> pExitProgram_`; initializer `pExitProgram(pExitProgram_)` →  
  `pExitProgram(std::move(pExitProgram_))`.

### 4.2 event.h

- Add overload: `void Trigger(Event* pE);` (for non-owning use). Keep `void Trigger(smart_pointer<Event> pE);` for SequenceOfEvents.

### 4.3 event.cc

- Add:
  ```cpp
  void Trigger(Event* pE) {
    if (pE)
      pE->Trigger();
  }
  ```
  Keep existing `Trigger(smart_pointer<Event> pE)` for use from SequenceOfEvents.

### 4.4 Entry points: create and move Event

- **simulation_test.cc** (both ProgramEngine constructions):
  - Change `smart_pointer<Event> p_exit_ev = make_smart(NewSwitchEvent(b_exit, b_true));`  
    → `auto p_exit_ev = std::make_unique<SwitchEvent<bool, bool>>(b_exit, b_true);`  
    (NewSwitchEvent returns `SwitchEvent<A,B>*`; SwitchEvent is in event.h.)
  - Change ProgramEngine ctor to pass `std::move(p_exit_ev)` for the first argument.
- **simulation.cc**: same.
- **sdl_game_runner_main.cc**:
  - Replace `make_smart(NewSwitchEvent(bExit, bTrue))` with  
    `std::make_unique<SwitchEvent<bool, bool>>(bExit, bTrue)` and pass `std::move(...)` as first argument to ProgramEngine constructor.

### 4.5 dragon_game_runner.h

- Change `smart_pointer<Event> pExitProgram;` → `Event* pExitProgram;` in TowerDataWrap.

### 4.6 dragon_game_runner.cc

- In TowerDataWrap ctor: change `pExitProgram = pe.pExitProgram;` →  
  `pExitProgram = pe.pExitProgram.get();`
- Trigger(pData->pExitProgram) and menu_controller’s Trigger(...pExitProgram) now use Event*; Trigger(Event*) handles them.

### 4.7 menu_controller.cc

- No signature change; `Trigger(pMenuController->pGl->pWrp->pExitProgram)` now passes Event*; ensure event.h is included so Trigger(Event*) is visible.

### 4.8 Event and SP_Info

- Do **not** remove SP_Info from Event in Phase 1 (SequenceOfEvents still has `vector<smart_pointer<Event>>`). Document in Step 5.

**Verify:** Build; ctest (including simulation_test).

---

## Step 5: Document process and deferrals

### 5.1 Process (PROC-01, PROC-02, PROC-03)

- In STATE.md or a short .planning/phase_1_process.md: state that Phase 1 followed bottom-up order, one logical group at a time (MessageWriter then Event), and that build + ctest were run after each group. State that types whose lifetime is unclear are not migrated and are recorded for the final summary (DOC-01).

### 5.2 Deferral

- Note: “Event: SP_Info not removed in Phase 1 because SequenceOfEvents still uses vector<smart_pointer<Event>>. To be removed when SequenceOfEvents is migrated (later phase).”

### 5.3 Unmigrated summary (if any)

- If any game_utils type was considered but skipped (lifetime unclear), add one line to .planning/smart_pointer_unmigrated.md or similar: class name and brief reason. Phase 1 scope had clear lifetime for Event and MessageWriter, so this may be empty.

---

## Verification checklist

- [ ] After Step 1: `cd build; mingw32-make` succeeds.
- [ ] After Step 2: build succeeds; `ctest --output-on-failure` passes (including simulation_test).
- [ ] After Step 3: build succeeds; ctest passes; no smart_pointer<MessageWriter> in codebase; MessageWriter has no SP_Info.
- [ ] After Step 4: build succeeds; ctest passes; exit event flow uses unique_ptr at ProgramEngine and Event* elsewhere; Trigger(Event*) used for exit path.
- [ ] After Step 5: Phase 1 process and Event/SequenceOfEvents deferral documented.

---

## Files touched (summary)

| File | Changes |
|------|--------|
| code/game_utils/game_runner_interface.h | unique_ptr for Event + MessageWriter; delete copy; GetGameRunner(const&); include <memory> |
| code/game_utils/game_runner_interface_impl.cc | GetGameRunner(ProgramEngine const& pe) |
| code/game_utils/event.h | Trigger(Event*) overload |
| code/game_utils/event.cc | Trigger(Event*) impl |
| code/game_utils/MessageWriter.h | Remove SP_Info (and get_class_name if only for SP_Info) from MessageWriter, EmptyWriter, IoWriter, FileWriter |
| code/game/dragon_game_runner.h | TowerDataWrap/DragonGameRunner(ProgramEngine const&); TowerDataWrap::pExitProgram → Event* |
| code/game/dragon_game_runner.cc | TowerDataWrap/DragonGameRunner const&; pExitProgram = pe.pExitProgram.get() |
| code/game/controller/menu_controller.cc | (no change if Trigger(Event*) visible via event.h) |
| code/simulation/simulation_test.cc | make_unique EmptyWriter + move; make_unique SwitchEvent<bool,bool> + move; ProgramEngine(..., std::move(...)) |
| code/simulation/simulation.cc | same |
| code/wrappers_sdl_impl/sdl_game_runner_main.cc | make_unique IoWriter + move; make_unique SwitchEvent<bool,bool> + move |
| .planning/STATE.md or phase_1_process.md | Process and Event/SequenceOfEvents note |

---
*Phase 1 plan created for execution. Run build and ctest after Steps 2, 3, and 4.*
