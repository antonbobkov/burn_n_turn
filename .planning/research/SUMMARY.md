# Project Research Summary

**Project:** Smart Pointer Migration (dragon game codebase)
**Domain:** C++ ownership migration (custom ref-counted → std::unique_ptr)
**Researched:** 2025-02-16
**Confidence:** HIGH

## Executive Summary

The migration from the codebase’s custom `smart_pointer` and `SP_Info` to
`std::unique_ptr` and raw pointers is a well-understood refactor. The
recommended approach is incremental and bottom-up: one class (or small
coherent group) at a time, with a single `unique_ptr` owner per object and
`T*` everywhere else, and tests passing after every step. Critical risks are
dangling raw pointers, double ownership (two unique_ptrs to the same
object), and changing types in the wrong order. Mitigations: strict
lifetime analysis per class (as in smart_pointer_burndown.md), bottom-up
phase order, and running build + tests after each change.

## Key Findings

### Recommended Stack

- **std::unique_ptr** for the single owning site per object; **raw pointer**
  for non-owning use. **std::make_unique** preferred where possible.
- **C++14** is sufficient; no stack change required.
- **Build + tests** (especially simulation_test) after every migration step.
- Optional: clang-tidy modernize-make-unique after unique_ptr is in use.

### Expected Features (Migration)

**Must have (table stakes):**
- Lifetime analysis per class and one unique_ptr owner.
- All other references as T* (or T&).
- Build and all tests pass after each step.
- Remove SP_Info when a class has no smart_pointer uses; remove
  smart_pointer implementation when no class uses it.

**Quality (differentiators):**
- Bottom-up order (leaf types first).
- make_unique for construction.

**Avoid:**
- Big-bang change; shared_ptr everywhere; skipping tests between steps.

### Architecture Approach

- **Order:** Leaf/value-like types first (e.g. simple entities, drawers),
  then types that own them (controllers, generators), then runners and
  ProgramEngine, then entry points (main, simulation_test).
- **Ownership flow:** Entry points hold unique_ptr to top-level objects;
  each layer holds unique_ptr for what it owns and passes T* or T&
  downward.
- **Exception:** Base-class arrays may keep smart_pointer per burndown doc;
  all other uses of a migrated type should be unique_ptr at owner and T*
  elsewhere.

### Critical Pitfalls

1. **Dangling T*** — Ensure the unique_ptr owner outlives every use of T*.
2. **Double unique_ptr** — Only one unique_ptr per object; use move for
   transfer.
3. **Wrong order** — Migrate dependencies before their owners (bottom-up).
4. **SP_Info removed too early** — Remove only when no smart_pointer<T>
   remains.
5. **Skipping tests** — Run build + tests after every step.

## Implications for Roadmap

Suggested phase structure (bottom-up):

### Phase 1: Leaf types (game_utils / simple game types)

**Rationale:** No ownership of other smart_pointer types; clear single owner
(ProgramEngine, runner, or one controller).
**Delivers:** Event, MessageWriter, and similar types migrated; SP_Info
removed where applicable.
**Avoids:** Pitfall 3 (wrong order).

### Phase 2: Wrappers and platform types

**Rationale:** GUI/sound interfaces and implementations often owned at
ProgramEngine or main.
**Delivers:** GraphicalInterface, SoundInterface implementations and mocks
migrated.
**Uses:** Same pattern (unique_ptr at creation, T* elsewhere).

### Phase 3: Game entities and animations (leaf game objects)

**Rationale:** AnimationOnce, BonusScore, simple entities, drawers—owned by
critters, controllers, or level.
**Delivers:** Leaf game types migrated; SP_Info removed.
**Avoids:** Pitfall 3.

### Phase 4: Critters, generators, fireball, level

**Rationale:** These own the leaf entities; migrate after leaves.
**Delivers:** Critters, generators, fireball, level types migrated.
**Implements:** Ownership in controllers/level.

### Phase 5: Controllers and runner

**Rationale:** Controllers own view entities; runner owns current controller.
**Delivers:** All GameController implementations and DragonGameRunner
migrated.

### Phase 6: Entry points and removal of smart_pointer

**Rationale:** main and simulation_test create top-level objects; after they
use unique_ptr, no smart_pointer uses remain.
**Delivers:** Entry points hold unique_ptr; delete smart_pointer.h/.cc,
smart_pointer_test.cc, SP_Info.

### Phase Ordering Rationale

- Bottom-up ensures each type’s dependents are already migrated or
  non–smart_pointer.
- Tests after each phase (and ideally after each class) limit regression
  scope.
- Final phase is the only one that removes the smart_pointer implementation.

### Research Flags

- **Phases 1–2:** Straightforward; ownership is documented in ARCHITECTURE.
- **Phases 3–4:** More types and ownership paths; verify lifetime for each
  class per burndown doc.
- **Phase 6:** Full codebase grep for smart_pointer and SP_Info before
  deletion.

## Confidence Assessment

| Area        | Confidence | Notes                              |
|-------------|------------|------------------------------------|
| Stack       | HIGH       | Standard C++14, Core Guidelines     |
| Features    | HIGH       | Aligned with burndown and PROJECT  |
| Architecture| HIGH       | Matches codebase layers and doc    |
| Pitfalls    | HIGH       | Common unique_ptr migration issues |

**Overall confidence:** HIGH.

### Gaps to Address

- Exact list of “leaf” vs “mid” types should be confirmed when planning
  each phase (e.g. from grep of make_smart and smart_pointer<T>).
- Base-class arrays: identify which containers keep smart_pointer and
  document in phase plans.

---
*Research completed: 2025-02-16*
*Ready for roadmap: yes*
