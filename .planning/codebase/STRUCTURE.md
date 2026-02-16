# Directory structure (code/)

Scope: `code/` only. Sibling dirs such as `build/` and `bin/` are build
artifacts (see project README/workspace rules).

## Top-level layout

```
code/
├── CMakeLists.txt          # Root: Catch2, SDL fetch, subdirs, copy_sdl_dlls
├── how_to_code.md          # Class/style and coding guidelines
├── how_to_plan.md
├── identify-dependencies.md
├── remove-dependencies.md
├── smart_pointer_burndown.md
├── headers_reference.md
├── classes_reference.md
├── README.md
├── sdl_examples/           # Standalone SDL examples (SDL_ex1–4)
├── utils/                  # Shared utilities and tests
├── wrappers/               # Abstract GUI/sound and generic types
├── game_utils/             # Event, MessageWriter, Preloader, draw/sound helpers
├── wrappers_sdl_impl/      # SDL implementation of wrappers + examples
├── wrappers_mock/          # Mock GUI/sound for tests
├── game/                   # Dragon game core and tower_defense executable
└── simulation/             # Headless simulation and simulation_test
```

## Subdirectories

### `code/utils/`

- **Libraries**: Exception (`exception.h/.cc`), file_utils, index, math_utils,
  random_utils, smart_pointer, string_utils, timer, debug_log. Built as
  static lib `Utils`.
- **Tests**: `file_utils_test.cc`, `smart_pointer_test.cc` (Catch2); built as
  executables. No game or SDL dependency.

### `code/wrappers/`

- **Headers/sources**: `color`, `geometry`, `font_writer`, `gui_key_type`,
  `GuiGen`, `SuiGen`. Defines `GraphicalInterface<>`, `SoundInterface<>`,
  image/sound handle types, and generic implementations (e.g.
  `SimpleGraphicalInterface`). Depends on `utils/` (exception, file_utils,
  index, smart_pointer).

### `code/game_utils/`

- **Library**: Event, MessageWriter, Preloader, sound_utils, sound_sequence,
  mouse_utils, draw_utils, image_sequence, `game_runner_interface.h`. Static
  lib `GameUtils`. Depends on `wrappers/` and `utils/`.

### `code/wrappers_sdl_impl/`

- **Libraries**: GuiSdl, SuiSdl (SDL-backed GUI and sound). **Executables**:
  `gui_sdl_example`, `sui_sdl_example`, `test_sdl` (Catch2). Main game
  executable is not here; it is in `game/` and links these libs.

### `code/wrappers_mock/`

- **Libraries**: GuiMock, SuiMock. **Executable**: `wrappers_mock_test`
  (Catch2). Used by `simulation_test` and any test that needs headless
  GUI/sound.

### `code/game/`

- **Library**: GameCore (static) — dragon_game_runner, level, entities,
  tutorial, fireball, critters, critter_generators, dragon, and controllers
  (basic_controllers, dragon_game_controller, menu_controller,
  buy_now_controller, level_controller), plus `game_runner_interface_impl.cc`.
- **Executable**: `tower_defense` — source is
  `code/wrappers_sdl_impl/sdl_game_runner_main.cc`; linked with GameCore,
  GameUtils, GuiSdl, SuiSdl, SDL libs. Built via `game/CMakeLists.txt`.
- **Controllers**: Subfolder `game/controller/` for all controller .h/.cc
  files.

### `code/simulation/`

- **Library**: Simulation (just `simulation.cc` with `RunSimulation()`).
- **Executable**: `simulation_test` — Catch2 test that runs headless
  simulation (mocks, InMemoryFileManager/CachingReadOnlyFileManager),
  asserts level/menu/sound and smart-pointer cleanup.

## Key file locations

| What                | Path |
|---------------------|------|
| Root CMake          | `code/CMakeLists.txt` |
| Game lib + exe      | `code/game/CMakeLists.txt` |
| SDL GUI/sound impl  | `code/wrappers_sdl_impl/GuiSdl.*`, `SuiSdl.*` |
| Game runner         | `code/game/dragon_game_runner.h/.cc` |
| Runner entry        | `code/wrappers_sdl_impl/sdl_game_runner_main.cc` |
| Game controller API | `code/game/controller/game_controller_interface.h` |
| Program engine      | `code/game_utils/game_runner_interface.h` |
| File abstraction    | `code/utils/file_utils.h` |
| Simulation test     | `code/simulation/simulation_test.cc` |

## Naming conventions

- **CMake targets**: PascalCase for libs (e.g. GameCore, GuiSdl); lowercase
  for executables (tower_defense, simulation_test, file_utils_test).
- **Source files**: snake_case `.cc` and `.h` (e.g. `dragon_game_runner.cc`).
- **Test files**: `*_test.cc`; often named after the module (e.g.
  `file_utils_test.cc`, `simulation_test.cc`).
