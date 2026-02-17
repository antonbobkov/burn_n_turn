# Smart Pointer Migration

## What This Is

This project is the dragon game codebase (a C++14 tower-defense-style game with
SDL graphics/sound, headless simulation, and a controller-based game loop). The
current initiative is to migrate away from the custom ref-counted `smart_pointer`
and `SP_Info` by applying the burndown process to every class that uses them:
one `std::unique_ptr` owner per object, raw pointers elsewhere, then remove
`SP_Info` from each class when it has no smart-pointer uses left, and finally
remove the `smart_pointer` implementation.

## Core Value

The migration must preserve behavior: the project must keep building and all
tests (especially `simulation_test`) must pass after every change.

## Requirements

### Validated

- ✓ Platform abstraction — Graphics and sound behind
  `GraphicalInterface<>` / `SoundInterface<>`; SDL and mocks implement them.
- ✓ Game loop and runner — `GameRunner`, `Update()`, input hooks; SDL main
  loop drives `DragonGameRunner`.
- ✓ Controllers — In-game screens (menu, level, buy, etc.) as
  `GameController` implementations; `DragonGameController` owns current
  controller.
- ✓ Headless simulation — `simulation_test` (Catch2) runs without SDL and
  asserts on level/menu/sound and smart-pointer balance.
- ✓ File I/O via `FileManager` — Simulation and tests inject in-memory or
  caching file managers.
- ✓ Existing use of `std::unique_ptr` — FilePath, stream handlers, some
  internal pointers; pattern is established where ownership is single-owner.

### Active

- [ ] Apply burndown to all classes that use `smart_pointer`: for each, one
  `std::unique_ptr` owner for the object’s lifetime, `target_class*`
  everywhere else (exception: base-class arrays may keep `smart_pointer` per
  burndown doc).
- [ ] For each class with no remaining `smart_pointer` uses, remove `SP_Info`
  as base class.
- [ ] When no class uses `smart_pointer`, remove the implementation: delete
  `smart_pointer.h`, `smart_pointer.cc`, `smart_pointer_test.cc`, and
  ref-count machinery (`SP_Info`, globals, etc.).

### Out of Scope

- Changing game design or adding new features as part of this migration.
- Migrating to C++17/20 or other language; stack stays C++14, MinGW.
- Introducing shared ownership (`std::shared_ptr`) where the burndown
  prescribes unique ownership and raw pointers.

## Context

- Codebase is layered: utils → wrappers → game_utils → game / wrappers_sdl_impl
  / wrappers_mock → simulation. Many types in game, game_utils, and wrappers
  use `smart_pointer` and inherit `SP_Info`.
- Process is documented in `code/smart_pointer_burndown.md`: determine
  lifetime, place one `unique_ptr`, use raw pointers elsewhere; if lifetime
  cannot be determined, do not change code and report back.
- `.planning/codebase/` contains ARCHITECTURE.md, STACK.md, STRUCTURE.md, and
  related docs for reference during planning.

## Constraints

- **Tech stack**: C++14, CMake, MinGW Makefiles; build and bin are siblings of
  code (workspace rules).
- **Correctness**: After any change, build must succeed and
  `ctest --output-on-failure` (and especially `simulation_test`) must pass.
- **Code style**: Lines under 80 characters; simple non-technical comments;
  new headers must be included explicitly where declarations are used.

## Key Decisions

| Decision            | Rationale                    | Outcome   |
|---------------------|------------------------------|-----------|
| Migrate bottom-up   | Leaf types first, then       | — Pending |
|                     | owners; clearer ownership    |           |
|                     | and less rework.             |           |
| Remove smart_pointer| No long-term keep of unused  | — Pending |
| when done           | code; clean end state.       |           |

---
*Last updated: 2025-02-16 after initialization*
