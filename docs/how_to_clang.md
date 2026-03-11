# Running clang-tidy

The project uses `CMAKE_EXPORT_COMPILE_COMMANDS=YES`, so `build/debug/compile_commands.json`
is generated automatically after configuring. `run-clang-tidy` reads it to lint all files.

## Prerequisites

Build the project first so `compile_commands.json` exists:

```bash
cmake --preset debug
cmake --build --preset debug -j4
```

## Run clang-tidy on all project source files

```bash
# Project source files only (excludes Catch2 fetched into build/_deps/)
run-clang-tidy -p build/debug -source-filter '.*\\\\dragongame\\\\code\\\\.*' -quiet
```

**Windows path note:** `run-clang-tidy` uses `os.path.abspath` which produces backslash
paths on Windows (`C:\code\dragongame\code\...`). Git Bash eats one level of backslash
escaping even inside single quotes, so you need `\\\\` in the shell to get `\\` (a single
backslash) into the regex. The pattern `.*\\\\dragongame\\\\code\\\\.*` matches all files
under `dragongame\code\` and nothing in `build\_deps\`.

## Get a concise warning summary

```bash
run-clang-tidy -p build/debug -source-filter '.*\\\\dragongame\\\\code\\\\.*' -quiet 2>&1 | grep " warning: " | sed 's/.*warning: //' | sort | uniq -c | sort -rn
```

## Run on a single file

```bash
clang-tidy -p build/debug code/utils/index.cc
```

## Known findings (as of March 2026)

| Count | Warning | File | Assessment |
|-------|---------|------|------------|
| 5+1+1 | Virtual call in ctor/dtor bypasses dispatch | `GuiSdl.cc` | Low risk — SDL impl only |
| 2 | Use of memory after free | `index.cc`, `smart_pointer.h` | See below |
| 1 | Null pointer dereference | `dragon_game_controller.cc:84` | Real bug |
| 1 | Memory leak of `pPointToSPInfo` | `simulation.cc` | False positive |

**`index.cc:21` — `Index::operator=` self-assignment**: if assigned to itself and the
ref-count hits zero, `pCounter` is deleted then immediately dereferenced via `i.pCounter`.
Fix: add `if (this == &i) return *this;` guard.

**`dragon_game_controller.cc:84`**: the member initializer list uses `pDr_ ? pDr_->pGr : 0`
(implying `pDr_` could be null), but line 84 unconditionally dereferences `pDr_->pGr` in
the `Preloader` constructor argument. Real null-deref if `pDr_` is null.

**`smart_pointer.h:118` / `simulation.cc:149`**: the analyzer can't model the custom
ref-count logic fully; these are false positives.
