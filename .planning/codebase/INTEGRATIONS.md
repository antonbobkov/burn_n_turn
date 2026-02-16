# External integrations (code/)

Scope: `code/` only. No cloud APIs, auth providers, or webhooks; integrations
are local/system and optional file I/O.

## SDL2 (graphics, window, input, image, audio)

- **Where**: `code/wrappers_sdl_impl/` (GuiSdl, SuiSdl), `code/sdl_examples/`,
  and the `tower_defense` executable linking GuiSdl, SuiSdl, SDL2, SDL2_image,
  SDL2_mixer.
- **Usage**: SDL for window/renderer, image load (e.g. PNG/BMP via SDL_image),
  and sound (SDL_mixer). Headers: `SDL.h`, `SDL_image.h`, `SDL_mixer.h`.
- **Config**: Pre-built MinGW zips fetched by CMake; paths set in
  `code/CMakeLists.txt` (SDL_PATH, SDL_IMAGE_PATH, SDL_MIXER_PATH). DLLs
  copied to `bin/` via `copy_sdl_dlls`.

## File system and streams

- **Abstraction**: `FileManager` in `code/utils/file_utils.h`: `ReadFile`,
  `WriteFile`, `FileExists`. Implementations: `StdFileManager` (real paths),
  `InMemoryFileManager` (in-memory map), `CachingReadOnlyFileManager`
  (read cache + optional extension filter).
- **Usage**: Game and simulation use `FileManager*` (e.g. from
  `ProgramEngine`/`TowerDataWrap`); tests use `InMemoryFileManager` to avoid
  disk I/O. `FilePath` in `code/utils/file_utils.h` adds path helpers and
  slash/convention handling.
- **Persistence**: No database; state is file-based (e.g. `SavableVariable<T>`
  load/save in file_utils). Sound-on toggle in simulation writes a small file
  (e.g. `soundon.txt`) via `CachingReadOnlyFileManager`.

## No external services in code/

- No HTTP/REST clients, no auth providers, no webhooks.
- No config files from environment or network; program config is from
  `GetProgramInfo()` and `ProgramEngine` (e.g. `code/game_utils/game_runner_interface.h`).

## Test-only “integrations”

- **Mocks**: `code/wrappers_mock/` provides `MockGraphicalInterface` and
  `MockSoundInterface` so tests (e.g. `code/simulation/simulation_test.cc`)
  run without a real display or audio device.
- **In-memory files**: Tests use `InMemoryFileManager` and
  `CachingReadOnlyFileManager` over it to avoid touching the real filesystem.
