# Dead Code Summary

This document lists classes and functions that appear to be unused in the
codebase (no references other than their definitions). They are candidates for
removal or for reviving via tests/documentation.

---

## Unused classes / structs

### `SimpleController` (game/controller)

- **Location:** `code/game/controller/basic_controllers.h`, `.cc`
- **Description:** Controller that draws a single full-screen image and advances
  on key.
- **Finding:** Never instantiated. `dragon_game_controller.cc` builds the game
  flow with `AutoAdvanceController`, `StartScreenController`, `Cutscene`,
  `DragonScoreController`, etc., but never uses `SimpleController`.

### `FlashingController` (game/controller)

- **Location:** `code/game/controller/basic_controllers.h`, `.cc`
- **Description:** Controller that draws background plus text that flashes every
  second.
- **Finding:** Never instantiated; same as above.

### `FileWriter` (game_utils)

- **Location:** `code/game_utils/MessageWriter.h`, `MessageWriter.cc`
- **Description:** `MessageWriter` subclass that writes to a file; has
  `AddFile()` and overrides `Write()`.
- **Finding:** Only `EmptyWriter` and `IoWriter` are used (simulation and
  SDL main). No code creates or uses `FileWriter`.

### `EmptyEvent` (game_utils)

- **Location:** `code/game_utils/event.h`
- **Description:** Event subclass that does nothing on `Trigger()`.
- **Finding:** No code instantiates `EmptyEvent` (e.g. no `new EmptyEvent()` or
  `make_smart(new EmptyEvent())`).

### `WriteEvent` (game_utils)

- **Location:** `code/game_utils/event.h`
- **Description:** Event that prints text to stdout on `Trigger()`.
- **Finding:** No instantiations found.

### `MakeSoundEvent` (game_utils)

- **Location:** `code/game_utils/event.h`
- **Description:** Event that plays a sound on `Trigger()`.
- **Finding:** No instantiations found.

---

## Unused functions

### `Trigger(Event* pE)` overload (game_utils)

- **Location:** `code/game_utils/event.h`, `event.cc`
- **Description:** Free function overload that takes a raw `Event*` and calls
  `Trigger()`.
- **Finding:** All call sites use `Trigger(smart_pointer<Event>)`. The raw
  pointer overload is never called.

### `TwoEvents(smart_pointer<Event>, smart_pointer<Event>)` (game_utils)

- **Location:** `code/game_utils/event.h`, `event.cc`
- **Description:** Returns a `SequenceOfEvents*` combining two events.
- **Finding:** Only defined; no callers.

### `EnableDebugLog()` / `DebugLog(std::string)` (utils)

- **Location:** `code/utils/debug_log.h`, `debug_log.cc`
- **Description:** Toggle and log to stdout when debug is enabled.
- **Finding:** No other file calls these functions.

### `Center(Rectangle r)` (wrappers/geometry)

- **Location:** `code/wrappers/geometry.h`, `geometry.cc`
- **Description:** Returns the center point of a rectangle.
- **Finding:** Only the inline `Center(Size sz)` overload is used; no call to
  `Center(Rectangle)`.

### `Dot(const fPoint &f1, const fPoint &f2)` (wrappers/geometry)

- **Location:** `code/wrappers/geometry.h`, `geometry.cc`
- **Description:** Dot product of two float points.
- **Finding:** No call sites.

### `RandomPnt(Rectangle r)` (wrappers/geometry)

- **Location:** `code/wrappers/geometry.h`, `geometry.cc`
- **Description:** Returns a random point inside a rectangle.
- **Finding:** No call sites.

### `FileWriter::AddFile(WriteType, std::string)` (game_utils)

- **Location:** `code/game_utils/MessageWriter.h`, `MessageWriter.cc`
- **Description:** Registers a file for a write type (used by `FileWriter`).
- **Finding:** Only used inside the unused `FileWriter` class.

---

## Summary table

| Item                         | Type     | Location (main)        |
|------------------------------|----------|-------------------------|
| SimpleController             | class    | game/controller         |
| FlashingController           | class    | game/controller         |
| FileWriter                   | class    | game_utils              |
| EmptyEvent                   | class    | game_utils              |
| WriteEvent                   | class    | game_utils              |
| MakeSoundEvent               | class    | game_utils              |
| Trigger(Event*)              | function | game_utils/event       |
| TwoEvents(...)               | function | game_utils/event       |
| EnableDebugLog / DebugLog    | function | utils/debug_log         |
| Center(Rectangle)            | function | wrappers/geometry      |
| Dot(fPoint, fPoint)          | function | wrappers/geometry      |
| RandomPnt(Rectangle)         | function | wrappers/geometry      |
| FileWriter::AddFile          | function | game_utils (via above) |

---

## Notes

- **Event classes:** `EmptyEvent`, `WriteEvent`, and `MakeSoundEvent` may have
  been intended for scripting or debugging; they could be kept and used from
  tests or tooling instead of removed.
- **SimpleController / FlashingController:** Might have been used in an older
  flow or for alternate builds; worth confirming with history or product needs
  before deleting.
- **DebugLog:** Useful for future debugging; consider calling from a
  command-line flag or leaving for manual use.
- **Geometry helpers:** `Center(Rectangle)`, `Dot`, and `RandomPnt` are small
  utilities; safe to remove if unused, or to keep if planned for future use.

Analysis was done by searching the `code/` tree for definitions and references;
tests and examples under `code/` were included in the search.
