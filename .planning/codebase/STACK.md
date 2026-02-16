# Technology Stack (code/)

## Languages and runtime

- **C++14** — `code/CMakeLists.txt`: `set(CMAKE_CXX_STANDARD 14)` and
  `CMAKE_CXX_STANDARD_REQUIRED ON`.
- **Build**: CMake 3.14+, MinGW (e.g. `i686-w64-mingw32` or
  `x86_64-w64-mingw32`). Configure from a sibling `build/` with
  `cmake ../code -G "MinGW Makefiles"`; executables go to `../bin/`.

## Frameworks and libraries

- **Catch2 v2** — Unit and integration tests. Fetched via FetchContent in
  `code/CMakeLists.txt` (v2.13.10). Linked as static library; tests use
  `#include <catch2/catch.hpp>` and `TEST_CASE`, `REQUIRE`, `CHECK`, etc.
- **SDL2** — Graphics, windowing, input. Pre-built MinGW dev zip (e.g.
  SDL2 2.32.10) fetched and unpacked; `SDL_PATH` points to arch-specific
  include/lib. Used by `code/wrappers_sdl_impl/` (GuiSdl, SuiSdl) and
  `code/sdl_examples/`.
- **SDL2_image** — Image loading. Pre-built MinGW (e.g. 2.8.8); used for
  `SdlImage` and examples in `code/wrappers_sdl_impl/`, `code/sdl_examples/`.
- **SDL2_mixer** — Audio. Pre-built MinGW (e.g. 2.8.1); used for
  `SdlSoundInterface` and sound examples.

## Dependencies (in-repo)

- **Utils** — `code/utils/`: exception types, file_utils, index, math_utils,
  random_utils, smart_pointer, string_utils, timer, debug_log.
- **wrappers** — `code/wrappers/`: abstract GUI/sound (GuiGen, SuiGen), color,
  geometry, font_writer, gui_key_type.
- **game_utils** — `code/game_utils/`: event, MessageWriter, Preloader,
  sound_utils, sound_sequence, mouse_utils, draw_utils, image_sequence,
  game_runner_interface.
- **wrappers_sdl_impl** — SDL implementations of GUI and sound; **wrappers_mock**
  — mock implementations for tests.
- **game** — `code/game/`: dragon game logic, level, entities, controllers,
  dragon_game_runner; links GameUtils.
- **simulation** — `code/simulation/`: headless simulation entry and
  `simulation_test` (Catch2).

No package manager beyond CMake FetchContent; no Node/npm, Python venv, or
other runtimes in `code/`.

## Configuration

- **CMake**: Single top-level `code/CMakeLists.txt`; subdirs:
  `sdl_examples`, `utils`, `wrappers`, `game_utils`, `wrappers_sdl_impl`,
  `wrappers_mock`, `game`, `simulation`. CMP0169 set to OLD for
  FetchContent_Populate (SDL zips are pre-built, not built from source).
- **Output**: `CMAKE_RUNTIME_OUTPUT_DIRECTORY` = `../bin/`; custom target
  `copy_sdl_dlls` copies SDL2, SDL2_image, SDL2_mixer DLLs to bin.
- **Testing**: `enable_testing()`; CTest used to run tests from `build/`.

## Key files

| Purpose              | Path                          |
|----------------------|-------------------------------|
| Root CMake           | `code/CMakeLists.txt`         |
| C++ standard         | `code/CMakeLists.txt`         |
| Catch2 / SDL fetch   | `code/CMakeLists.txt`         |
| Game static lib      | `code/game/CMakeLists.txt`    |
| Main executable      | `code/game/CMakeLists.txt` (tower_defense) |
| Simulation test      | `code/simulation/CMakeLists.txt` |
