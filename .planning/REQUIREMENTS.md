# Requirements: Smart Pointer Migration

**Defined:** 2025-02-16
**Core Value:** The migration must preserve behavior: the project must keep
building and all tests (especially simulation_test) must pass after every
change.

## v1 Requirements

Requirements for the migration. Each maps to roadmap phases.

### Process

- [ ] **PROC-01**: Migration is performed in bottom-up order: leaf types (no
  ownership of other smart_pointer types) are migrated before types that own
  them.
- [ ] **PROC-02**: Migration is done one class (or small coherent group) at a
  time; build and tests run after each step.
- [ ] **PROC-03**: A class is not migrated if its object lifetime cannot be
  determined with confidence; such classes remain on smart_pointer and are
  recorded for the final summary.

### Burndown (per class)

- [ ] **BURN-01**: For every class T that currently uses smart_pointer,
  either: (a) object lifetime is determined with confidence and a single
  owning site is identified (per smart_pointer_burndown.md), and then T is
  migrated, or (b) lifetime cannot be determined with confidence, in which
  case T is not migrated and is added to the unmigrated summary.
- [ ] **BURN-02**: For every class T that is migrated, the owning site holds
  std::unique_ptr<T>; all other uses hold T* or T& (exception: base-class
  arrays may keep smart_pointer per burndown doc).
- [ ] **BURN-03**: Where possible, construction uses std::make_unique<T>(...)
  instead of unique_ptr<T>(new T(...)).

### Cleanup (per class)

- [ ] **CLEAN-01**: For every class T that has no remaining smart_pointer<T>
  uses, SP_Info is removed as a base class of T.

### Removal (end state)

- [ ] **REMOVE-01**: When no class uses smart_pointer, the implementation is
  removed: smart_pointer.h, smart_pointer.cc, smart_pointer_test.cc, and
  ref-count machinery (SP_Info, globals such as g_smart_pointer_count /
  nGlobalSuperMegaCounter, CHECK_DELETION, DELETE_REGULAR_POINTER).
- [ ] **REMOVE-02**: All includes and references to smart_pointer or SP_Info
  are removed from the codebase (except any kept in docs/comments for
  history).

### Documentation (end state)

- [ ] **DOC-01**: At the end of the migration, a summary document is written
  listing every class that was not migrated because object lifetime could
  not be determined with confidence, with brief rationale per class where
  applicable (e.g. "shared across callbacks", "stored in container with
  unclear owner"). If all classes were migrated, the summary states that.

### Verification

- [ ] **VER-01**: After every migration step, the project builds successfully
  (e.g. from build dir: cmake ../code -G "MinGW Makefiles"; mingw32-make).
- [ ] **VER-02**: After every migration step, all tests pass (ctest
  --output-on-failure from build dir; simulation_test in particular).

## v2 Requirements

Deferred; not in current roadmap.

(None — migration is the sole initiative.)

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature or change              | Reason |
|--------------------------------|--------|
| Changing game design or adding | Migration only; no new features. |
| new game features               | |
| Migrating to C++17/20 or other | Stack stays C++14, MinGW. |
| language                       | |
| Introducing std::shared_ptr for| Burndown prescribes unique_ptr  |
| current smart_pointer use      | + raw ptr; shared ownership out. |
| Big-bang migration (all at once)| Incremental, one class at a time. |
| Keeping smart_pointer code     | Remove implementation when no uses. |
| "just unused"                  | |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status   |
|-------------|-------|----------|
| PROC-01     | —     | Pending  |
| PROC-02    | —     | Pending  |
| BURN-01    | —     | Pending  |
| BURN-02    | —     | Pending  |
| BURN-03    | —     | Pending  |
| CLEAN-01   | —     | Pending  |
| REMOVE-01  | —     | Pending  |
| REMOVE-02  | —     | Pending  |
| DOC-01     | —     | Pending  |
| VER-01     | —     | Pending  |
| VER-02     | —     | Pending  |

**Coverage:**
- v1 requirements: 12 total
- Mapped to phases: 0
- Unmapped: 12 ⚠️ (to be filled by roadmap)

---
*Requirements defined: 2025-02-16*
*Last updated: 2025-02-16 after adjustment (no migrate if lifetime unclear; end summary)*
