# Testing (code/)

Scope: `code/` only. All tests use Catch2 v2; CTest runs them from `build/`.

## Framework

- **Catch2**: Fetched in `code/CMakeLists.txt` (v2.13.10), static library.
  Include: `#include <catch2/catch.hpp>`. Use `TEST_CASE`, `REQUIRE`, `CHECK`,
  `REQUIRE_FALSE`, `REQUIRE_THROWS`, `REQUIRE_THROWS_AS`, etc.
- **Execution**: From project root: build in `build/`, then
  `cd build; ctest --output-on-failure`. Critical test:
  `cd bin; .\simulation_test.exe` (workspace rule).

## Test layout

| Test executable       | Source(s)                    | Focus |
|-----------------------|------------------------------|--------|
| file_utils_test       | `code/utils/file_utils_test.cc` | InMemoryFileManager, FilePath, CachingReadOnlyFileManager, SavableVariable |
| smart_pointer_test    | `code/utils/smart_pointer_test.cc` | smart_pointer ref-count, make_smart, cleanup, g_smart_pointer_count |
| wrappers_mock_test    | `code/wrappers_mock/wrappers_mock_test.cc` | MockGraphicalInterface, MockSoundInterface (LoadImage, GetImage, DeleteImage, etc.) |
| test_sdl              | `code/wrappers_sdl_impl/test_sdl.cc` | SDL init, window/renderer, image load, SdlGraphicalInterface::LoadImage, SdlSoundInterface::LoadSound |
| simulation_test       | `code/simulation/simulation_test.cc` | Full headless game run: reach level/menu, sound toggle file, cheats, game over; smart_pointer balance |

## Patterns

- **No real I/O in unit tests**: File tests use `InMemoryFileManager`;
  simulation_test uses `InMemoryFileManager` and `CachingReadOnlyFileManager`
  so no disk access is required.
- **Mocks for GUI/sound**: Simulation and any headless test use
  `MockGraphicalInterface` and `MockSoundInterface` from
  `code/wrappers_mock/` so no display or audio device is needed.
- **Tags**: Tests use tags like `[simulation][integration]`, `[file_utils]`,
  `[smart_pointer]`, `[sdl]` for filtering.
- **Determinism**: Simulation test uses fixed seed (`srand(12345)`) for
  reproducible behavior.

## Simulation test specifics

- **Location**: `code/simulation/simulation_test.cc`.
- **Setup**: Builds `ProgramEngine` with mock GUI/sound, exit event,
  MessageWriter (EmptyWriter), `CachingReadOnlyFileManager` over
  InMemoryFileManager. Creates `DragonGameRunner`, runs for a fixed number
  of frames, simulates key presses at specific frames.
- **Assertions**: Reaching level screen, menu screen, sound toggle writing to
  file; optional second test for cheats, load chapter, game over. Verifies
  `nGlobalSuperMegaCounter == 0` and no leftover ref-counts in
  `g_smart_pointer_count` to catch leaks.
- **Critical**: Workspace rule requires that after code changes, build and
  tests pass; “simulator_test” (simulation_test) must pass.

## Coverage and mocking

- No coverage tooling referenced in `code/`. Mock layer is
  `code/wrappers_mock/`; no other mock framework. Stubs are minimal (e.g.
  EmptyWriter for MessageWriter).
