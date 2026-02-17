# Roadmap: Smart Pointer Migration

**Created:** 2025-02-16
**Project:** .planning/PROJECT.md
**Requirements:** .planning/REQUIREMENTS.md

## Overview

| # | Phase | Goal | Requirements | Success criteria |
|---|-------|------|---------------|-------------------|
| 1 | Process and leaf (game_utils) | Establish process; migrate Event, MessageWriter, and leaf game_utils types | PROC-01, PROC-02, PROC-03, VER-01, VER-02 | 4 |
| 2 | Wrappers and platform | Migrate GUI/sound interfaces and implementations | BURN-01–03, CLEAN-01, VER-01, VER-02 | 4 |
| 3 | Game entities and animations | Migrate leaf game objects (entities, drawers, animations) | BURN-01–03, CLEAN-01, VER-01, VER-02 | 4 |
| 4 | Critters, generators, fireball, level | Migrate types that own leaf entities | BURN-01–03, CLEAN-01, VER-01, VER-02 | 4 |
| 5 | Controllers and runner | Migrate GameController implementations and DragonGameRunner | BURN-01–03, CLEAN-01, VER-01, VER-02 | 4 |
| 6 | Entry points, removal, and summary | Top-level ownership; remove smart_pointer; write unmigrated summary | REMOVE-01, REMOVE-02, DOC-01, VER-01, VER-02 | 5 |

All v1 requirements mapped to phases. Bottom-up order; do not migrate a class
if lifetime cannot be determined with confidence (record for DOC-01).

---

## Phase 1: Process and leaf (game_utils)

**Goal:** Establish migration process (bottom-up, one class at a time, no
migrate if lifetime unclear) and migrate leaf game_utils types whose lifetime
is clear (e.g. Event, MessageWriter, EmptyWriter, IoWriter). Run build and
tests after each class.

**Requirements:** PROC-01, PROC-02, PROC-03, VER-01, VER-02

**Success criteria:**
1. Process is documented or agreed: bottom-up order, per-step tests, skip
   and record when lifetime unclear.
2. At least one game_utils type (e.g. Event or MessageWriter) is migrated to
   unique_ptr at owner and T* elsewhere; SP_Info removed if no smart_pointer
   left.
3. Build succeeds and ctest (including simulation_test) passes after changes.
4. Any type skipped due to unclear lifetime is recorded for the final summary.

---

## Phase 2: Wrappers and platform types

**Goal:** Migrate GraphicalInterface, SoundInterface, FontWriter, and their
implementations (SDL, mocks) and proxies. Single owner at ProgramEngine or
entry; rest use T*.

**Requirements:** BURN-01, BURN-02, BURN-03, CLEAN-01, VER-01, VER-02

**Success criteria:**
1. Wrapper types that use smart_pointer and have clear lifetime are migrated;
   SP_Info removed where applicable.
2. Platform implementations (e.g. SdlGraphicalInterface, SdlSoundInterface,
   mocks) migrated or explicitly skipped and recorded.
3. Build and all tests pass.
4. Unmigrated types (if any) listed with rationale for DOC-01.

---

## Phase 3: Game entities and animations (leaf game objects)

**Goal:** Migrate leaf game types: Entity hierarchy, Drawer/NumberDrawer and
concrete drawers, AnimationOnce, StaticImage, Animation, simple entities.
Ownership lives in level, controllers, or critters.

**Requirements:** BURN-01, BURN-02, BURN-03, CLEAN-01, VER-01, VER-02

**Success criteria:**
1. Leaf game entities and animations with clear single owner are migrated;
   SP_Info removed where no smart_pointer<T> remains.
2. Build and all tests pass.
3. Any type skipped (lifetime unclear) recorded for final summary.

---

## Phase 4: Critters, generators, fireball, level

**Goal:** Migrate types that own leaf entities: critters, generators
(Princess, Trader, Knight, Mage, Skelly, Slime, etc.), fireball-related
types, level-owned structures. Bottom-up: these come after Phase 3.

**Requirements:** BURN-01, BURN-02, BURN-03, CLEAN-01, VER-01, VER-02

**Success criteria:**
1. Critter, generator, fireball, and level types with clear lifetime
   migrated; SP_Info removed where applicable.
2. Build and all tests pass.
3. Skipped types recorded.

---

## Phase 5: Controllers and runner

**Goal:** Migrate GameController implementations (MenuController,
LevelController, DragonGameController, etc.), DragonGameRunner, and
game_runner_interface_impl. Controllers own view entities; runner owns
current controller.

**Requirements:** BURN-01, BURN-02, BURN-03, CLEAN-01, VER-01, VER-02

**Success criteria:**
1. All controller and runner types with clear lifetime migrated; SP_Info
   removed where no smart_pointer uses remain.
2. Build and all tests pass.
3. Skipped types recorded.

---

## Phase 6: Entry points, removal, and summary

**Goal:** Migrate entry points (main, simulation_test, GetGameRunner) to hold
unique_ptr at top level. When no class uses smart_pointer, remove
implementation and all references. Write summary of unmigrated classes (or
state that all were migrated).

**Requirements:** REMOVE-01, REMOVE-02, DOC-01, VER-01, VER-02

**Success criteria:**
1. main and simulation_test (and any other entry points) use unique_ptr for
   top-level objects; no smart_pointer at entry.
2. If any class still uses smart_pointer (lifetime unclear), smart_pointer
   implementation remains; DOC-01 summary lists each such class with
   rationale. If none remain, implementation is removed per REMOVE-01,
   REMOVE-02.
3. DOC-01 deliverable: a document (e.g. .planning/smart_pointer_unmigrated.md
   or section in PROJECT.md) listing every class not migrated because
   lifetime could not be determined with confidence; or a statement that all
   classes were migrated.
4. All includes and references to smart_pointer/SP_Info removed from code
   (except in the summary doc if implementation is kept for unmigrated types).
5. Build and all tests pass.

---

## Phase ordering rationale

- **Bottom-up:** Phases 1–5 migrate leaf types first, then owners, so
  ownership is clear and we avoid rework (PROC-01).
- **Verification every phase:** VER-01, VER-02 applied in every phase.
- **DOC-01 in Phase 6:** Unmigrated summary is written at the end when the
  full set of skipped classes is known.
- **Removal only when safe:** REMOVE-01/REMOVE-02 in Phase 6; if any type
  remains on smart_pointer, keep implementation and document in summary.

---
*Roadmap created: 2025-02-16*
