## Build system

CMake + MinGW Makefiles, compiler: `clang++`. Presets defined in `CMakePresets.json`.

```bash
# Configure (first time or after CMakeLists changes)
cmake --preset debug

# Build
cmake --build --preset debug

# Run integration tests (unit + simulation, no SDL) — use this to validate changes
ctest --preset integration
```

## Conventions

- All include paths are relative to the source file's own directory.
- Member variables: trailing underscore (e.g. `config_`, `fp_`), parameters: no underscore.
- Test framework: Catch2 v2 (`TEST_CASE` / `REQUIRE`).
- Integer types: use `int` (not bare `unsigned`) throughout.
