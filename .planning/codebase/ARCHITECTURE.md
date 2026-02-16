# Architecture (code/)

## High-level pattern

- **Abstraction over platform**: Graphics and sound are abstracted in
  `code/wrappers/` (GuiGen, SuiGen) as template interfaces
  `GraphicalInterface<ImageHndl>` and `SoundInterface<SoundHndl>`. Concrete
  implementations: SDL in `code/wrappers_sdl_impl/`, mocks in
  `code/wrappers_mock/`. Game and simulation depend on these interfaces, not
  SDL directly.
- **Game loop / runner**: `GameRunner` in `code/game_utils/game_runner_interface.h`
  defines the main hook (`Update()`) and input hooks (KeyDown/Up, MouseMove,
  MouseClick, etc.). `GetGameRunner(ProgramEngine)` returns the active runner
  (e.g. `DragonGameRunner`). The SDL main loop lives in
  `code/wrappers_sdl_impl/sdl_game_runner_main.cc` and drives `Update()` and
  input forwarding.
- **Controllers**: In-game screens (menu, level, buy, etc.) are implemented as
  controllers deriving from `GameController` in
  `code/game/controller/game_controller_interface.h`. `DragonGameController`
  owns the current controller and forwards input/Update. Controllers have
  `GetControllerName()` for simulation/inspection.

## Data flow

- **Setup**: `ProgramEngine` holds exit event, graphical interface, sound
  interface, message writer, file manager, and screen resolution. Created once;
  passed into `DragonGameRunner` (and thus `TowerDataWrap`). Game gets
  `FilePath` and level data from `TowerDataWrap`.
- **Input**: SDL events → `sdl_game_runner_main` (or equivalent) → GameRunner
  virtual methods (KeyDown, MouseMove, …) → DragonGameRunner → current
  DragonGameController → active GameController (OnKey, OnMouse, etc.).
- **Output**: Game and controllers call into `GraphicalInterface<Index>` and
  `SoundInterface<Index>` (from ProgramEngine). No direct SDL in game code.
- **File I/O**: All file access goes through `FileManager*` (e.g. from
  ProgramEngine/TowerDataWrap). Simulation and tests inject
  InMemoryFileManager or CachingReadOnlyFileManager.

## Layers (code/)

1. **Utils** — Exception types, file_utils, smart_pointer, index, timer, etc.;
   no game or GUI logic.
2. **Wrappers** — Abstract GUI/sound types and generic implementations
   (GuiGen, SuiGen); geometry, color, font_writer, gui_key_type.
3. **Platform implementations** — wrappers_sdl_impl (SDL), wrappers_mock
   (mocks for tests).
4. **Game utilities** — Event, MessageWriter, Preloader, sound_sequence,
   draw_utils, game_runner_interface; used by game and runner.
5. **Game core** — Dragon, level, entities, fireball, critters, controllers,
   dragon_game_runner; depends on GameUtils and wrappers interfaces.
6. **Simulation** — Headless entry point and simulation_test; uses mocks and
  InMemory/Caching file managers.

## Abstractions and entry points

- **Entry points**: (1) `tower_defense` executable —
  `code/wrappers_sdl_impl/sdl_game_runner_main.cc` linked with GameCore,
  GuiSdl, SuiSdl. (2) `simulation_test` — `code/simulation/simulation_test.cc`
  (Catch2) runs headless simulation and asserts on level/menu/sound and
  smart-pointer balance.
- **Key abstractions**: `GameRunner`, `GameController`, `GraphicalInterface<>`,
  `SoundInterface<>`, `FileManager`, `Event`, `ProgramEngine`, `TowerDataWrap`.
- **Index handle**: GUI/sound use `Index` (from `code/utils/index.h`) as the
  handle type in production; mocks may use strings. Typedef used in
  `GraphicalInterface<Index>`, `SoundInterface<Index>`.

## Ownership and lifetimes

- **smart_pointer**: Custom ref-counted pointer in `code/utils/smart_pointer.h`;
  many game and runner objects are `smart_pointer<T>`. Objects inherit
  `SP_Info` for ref-counting. There is an ongoing migration (see
  `code/smart_pointer_burndown.md`) toward `std::unique_ptr` where ownership
  is single-owner.
- **unique_ptr**: Used for `FilePath`, stream handlers in file_utils, and
  some internal pointers (e.g. `pCnt` in TowerDataWrap). FileManager
  returns `unique_ptr` for stream handlers.
