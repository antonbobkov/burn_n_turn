# Game logic simulator — implementation summary

**Summary:** Program builds; `simulation_main` runs but currently fails in
`ReadLevels()` with "Cannot read levels at levels.txt" (no level parsed;
`vLvl.size() == 0`). To debug later: (1) Config `PATH .` is parsed by
`ParseGrabLine` so that `path_` becomes `"."` (space is skipped by `>>` before
`getline`), so InMemoryFileManager keys must use the dot prefix (e.g.
`.levels.txt`, `.dragonfont\\dragonfont.txt`). (2) `PopulateFont` was fixed so
it no longer prepends `"."` to the key (keys were `. .dragonfont\\...` and
`..dragonfont\\...`). (3) Level file is found but the minimal level string may
not parse as at least one `LevelLayout` (e.g. SPWN/BrokenLine format or stream
state); use `level.cpp` `operator>>(LevelLayout)` and BrokenLine format (e.g.
`&`-terminated) when fixing. FontWriter uses `std::ifstream` on the path
returned by `GetRelativePath`, not FileManager — simulator uses same path for
InMemoryFileManager so real file is never opened; if needed later, FontWriter
could be changed to use `fp->ReadFile()`.

---

## Step 1. Create implementation summary document

- **Deviations:** —
- **Issues:** —
- **Solutions:** —
- **Tips:** —

## Step 2. ProgramEngine always requires FileManager* (caller-owned)

- **Deviations:** —
- **Issues:** —
- **Solutions:** —
- **Tips:** —

## Step 3. Refactor game into a library

- **Deviations:** —
- **Issues:** —
- **Solutions:** —
- **Tips:** —

## Step 4. Simulation directory (library + main binary, no test)

- **Deviations:** —
- **Issues:** ProgramEngine expects `GraphicalInterface<Index>` / `SoundInterface<Index>`; simulation initially used `std::string`. `MyException` is in namespace `Gui`; simulation_main caught it without `Gui::` and failed to link.
- **Solutions:** Kept mock backends but assigned to `SP<GraphicalInterface<Index>>` and `SP<SoundInterface<Index>>` (built with `Simple*<std::string>(p_mock_...)`). Catch `Gui::MyException` in simulation_main.
- **Tips:** Level and font file keys depend on `path_` from config; see Step 5.

## Step 5. Minimal data for InMemoryFileManager

- **Deviations:** Populate both `" .*"` and `".*"` keys for levels and fonts so
  that either `path_` value works.
- **Issues:** (1) `ParseGrabLine("PATH", ifs, path)` uses `ifs >> c` which
  skips leading space, so with config `"PATH .\n"` the first character read is
  `'.'` and `path` becomes `"."`, not `" ."`. So the game looks up e.g.
  `.levels.txt`, `.dragonfont\\dragonfont.txt`. (2) `PopulateFont` originally
  did `key = "." + path_suffix`, so keys became `. .dragonfont\\...` and
  `..dragonfont\\...` and were never found. (3) Level file is found but
  `ReadLevels` throws because `vLvl.size() == 0` — the minimal level content
  may not match `operator>>(LevelLayout)` (e.g. SPWN/BrokenLine expects
  `&`-delimited format; FREQ/ROAD/TIME etc. must parse).
- **Solutions:** Use key as-is in `PopulateFont` (no prepended dot). Populate
  levels under ` .levels.txt`, `.levels.txt`, and `.levels_trial.txt`; fonts
  under ` .dragonfont\\...` and `.dragonfont\\...`.
- **Tips:** For future debugging: copy a minimal valid level from an existing
  `levels_trial.txt` or match `level.cpp` `operator>>` and BrokenLine
  `operator>>` (getline to `&`, then `|`-split parts and fPoint reads).

## Step 6. Build and test

- **Deviations:** —
- **Issues:** `simulation_main` exits with exception: "Cannot read levels at
  levels.txt" (no levels parsed). Left for later debugging.
- **Solutions:** —
- **Tips:** Build from `build/` with `mingw32-make`; run
  `../bin/simulation_main.exe`. Run existing tests and `tower_defense` to
  confirm no regressions.
