# Technical debt and concerns (code/)

Scope: `code/` only. Items that may need attention when changing or
refactoring.

## smart_pointer migration

- **Custom ref-counting**: `code/utils/smart_pointer.h` and `SP_Info` are used
  widely. The project is gradually moving to `std::unique_ptr` where
  ownership is single-owner (`code/smart_pointer_burndown.md`). New code
  should prefer `unique_ptr` when lifetime is clear; leave
  `smart_pointer` in arrays of base classes if needed.
- **Leak checks**: `simulation_test` asserts `nGlobalSuperMegaCounter == 0`
  and empty `g_smart_pointer_count` after run; failing this indicates
  leftover references or leaks. Any new `smart_pointer`/SP_Info usage should
  keep these checks passing.

## CMake and dependencies

- **CMP0169 OLD**: Used because SDL is fetched as pre-built zips and
  FetchContent_Populate is used only to unpack; MakeAvailable would try to
  build them. If CMake removes Populate in the future, another way to
  download/unpack (e.g. ExternalProject or script) will be needed.
- **MinGW-specific**: SDL paths assume MinGW arch (`i686-w64-mingw32` or
  `x86_64-w64-mingw32`). Non-MinGW or other platforms would need different
  SDL handling.

## Includes and coupling

- **Narrow includes**: Project aims for minimal, explicit includes
  (`code/identify-dependencies.md`, `code/remove-dependencies.md`). Adding a
  new `.h` requires updating all files that use its declarations (workspace
  rule). Broad or transitive includes can make builds and refactors fragile.

## Exception and error handling

- **Custom exception types**: Multiple classes (MatrixException,
  ImageException, SdlImageException, etc.) carry context. When adding new
  failure paths, follow the same pattern and provide useful debug info;
  inconsistent handling could make debugging harder.

## Testing and build

- **simulation_test as gate**: Workspace rule says build and all tests must
  pass, with simulation_test as the most important. Changes that break it
  (e.g. controller flow, smart_pointer lifetime, file manager behavior) must
  be fixed even if unrelated to the change.
- **No real files in tests**: Unit tests avoid real file I/O; tests that
  write to the actual filesystem would be fragile and environment-dependent.

## Platform and integrations

- **SDL DLLs**: Executables depend on SDL2, SDL2_image, SDL2_mixer DLLs in
  `bin/`. The copy_sdl_dlls target copies them; if FetchContent paths or
  versions change, DLL layout might need updating.
- **No network or external APIs**: All integrations are local (SDL, file
  system). Adding network or external services would introduce new failure
  modes and testability concerns.

## Code style and consistency

- **80 columns / simple comments**: Enforced by workspace rules. Long lines
  or overly technical comments may slip in during edits.
- **Avoid downcasts and unsigned**: Stated in how_to_code; existing code may
  still contain both; new code should avoid them where possible.
