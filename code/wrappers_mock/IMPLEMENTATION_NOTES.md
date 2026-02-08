# wrappers_mock Implementation Notes

Summary of issues encountered while implementing `GuiMock` and `SuiMock`, how they were solved, and tips to avoid similar problems in the future.

---

## 1. PointerAssert and non-pointer handle types

**Issue:** `GraphicalInterface<std::string>` could not be instantiated when using `GuiGen.h`. The base class default implementations call `PointerAssert<...>(..., pImg)`. The original `PointerAssert` did `if (!pnt)` and threw; for `std::string` there is no `operator!`, so compilation failed.

**Solution:** In `GuiGenHdr.h`, `PointerAssert` was changed to two overloads: (1) A default template that takes any `T` and is a no-op (just returns `pnt`). (2) A pointer overload `template <class NullExc, class P> P* PointerAssert(..., P* pnt)` that checks `if (!pnt)` and throws. Overload resolution picks the pointer version when the argument is a pointer, and the default for `std::string` or other handle types. The mock can then include `GuiGen.h` directly; no separate `GuiGenStringHandle.h` is needed.

**Tip:** When abstracting over a “handle” type (pointer, index, string), avoid pointer-only checks in a single shared template. Use a pointer overload for the check and a default no-op so non-pointer types work without specialization.

---

## 2. Double inclusion of GuiGenHdr.h

**Issue:** After adding `GuiGenStringHandle.h`, we got redefinition errors for `struct Color`, `operator==(const Color&, const Color&)`, and other symbols from `GuiGenHdr.h`. The sequence was: our header included `GuiGenHdr.h`, then `GuiGen.h`; `GuiGen.h` also includes `GuiGenHdr.h`. So `GuiGenHdr.h` was included twice in the same translation unit.

**Solution:** `GuiGenHdr.h` had no include guard. We added one:

```c
#ifndef GUIGENHDR_ALREADY_INCLUDED
#define GUIGENHDR_ALREADY_INCLUDED
// ... existing content ...
#endif /* GUIGENHDR_ALREADY_INCLUDED */
```

The second include (from `GuiGen.h`) then becomes a no-op and redefinitions are avoided.

**Tip:** Every header that defines types, functions, or inline/template code should be wrapped in an include guard (or use `#pragma once` if that’s your project standard). That allows the same header to be included from multiple places (e.g. another header and its users) without errors.

---

## 3. Undefined reference to base class methods

**Issue:** With only `GuiGenHdr.h` (no `GuiGen.h`), the mock implemented all required methods, but the linker reported undefined references to `GraphicalInterface<std::string>::CopyImage`, `FlipImage`, `ScaleImage`, `LoadImage`, `SaveImage`, `DrawImage(Point, string, bool)`, `RectangleOnto`, and `ImageOnto`. The vtable for the base part of `MockGraphicalInterface` still had slots pointing to these base-class implementations, which are defined in `GuiGen.h`. Without including `GuiGen.h`, those symbols were never instantiated.

**Solution:** Use `GuiGen.h` so that the default implementations are instantiated for `ImageHndl = std::string`; the “invalid handle” check is now a no-op for non-pointers (see §1). The mock only overrides what must behave differently: `LoadImage` (return path without opening a file), `SaveImage` (no-op), `DrawImage(Point, string, Rectangle, bool)`, `DrawRectangle`, `RefreshAll`, plus the pure virtuals.

**Tip:** When implementing a template interface with a type that isn’t the “native” one (e.g. string instead of pointer), either ensure all needed template instantiations are compiled (e.g. by including the header that defines them) or override every method that would otherwise be taken from the base so the linker doesn’t need the base versions.

---

## 4. Missing override implementation

**Issue:** Linker error: undefined reference to `MockGraphicalInterface::DrawImage(Point, std::string, bool)`. The header declared this override, but the .cpp only had the four-argument `DrawImage(Point, string, Rectangle, bool)`. The two-argument overload had been removed by mistake during edits.

**Solution:** Re-added the implementation of `DrawImage(Point p, std::string pImg, bool bRefresh)` in `GuiMock.cpp` (no-op body).

**Tip:** When a class has multiple overloads of a virtual (or overridden) function, keep a checklist: each declared overload should have exactly one definition in the .cpp. A quick grep for the function name in the .cpp helps catch missing or duplicate definitions.

---

## 5. SuiMock

**Issue:** None. `SoundInterface<std::string>` only has pure virtuals and `SetVolume` with an empty default. No pointer-style checks on the handle, so a mock that returns the path string and no-ops the rest compiles and links without extra tricks.

**Tip:** Interfaces that avoid pointer-specific checks in shared code make it easier to plug in non-pointer handle types (string, index, id) and test with mocks.

---

## 6. file_utils_test failures (OutStreamHandler destructor)

**Observed:** `file_utils_test` failed: after `WriteFile("a.txt")` and writing to the stream, once the `unique_ptr<OutStreamHandler>` went out of scope, `FileExists("a.txt")` was false and `GetFileContents("a.txt")` was empty. Content is supposed to be stored in the map when `InMemoryOutStreamHandler` is destroyed (its destructor copies `stream_->str()` into the map).

**Cause:** `OutStreamHandler` had no virtual destructor. When `unique_ptr<OutStreamHandler>` was destroyed, it called `~OutStreamHandler()`, not `~InMemoryOutStreamHandler()`, so the derived destructor never ran and the map was never updated.

**Solution:** Add a virtual destructor to `OutStreamHandler` in `file_utils.h` (e.g. `virtual ~OutStreamHandler() {}`). Then deleting through the base pointer correctly invokes `~InMemoryOutStreamHandler()` and all tests pass.

**Tip:** If a base class is used polymorphically (e.g. held as `unique_ptr<Base>` pointing to a derived type), the base must have a virtual destructor so that the derived destructor runs when the object is destroyed through the base pointer.

---

## Checklist for similar mocks

- [ ] Identify all places in the base/interface that assume a pointer-like handle (null checks, `!pnt`, etc.) and either add overloads/traits for the new handle type or override those methods in the mock.
- [ ] Ensure every header that can be included from more than one place has an include guard.
- [ ] If the base provides default implementations in a header, include that header so the linker gets the symbols, or override every such method in the mock.
- [ ] For each virtual/overridden function, confirm every declared overload has one corresponding definition in the implementation file.
- [ ] Prefer `std::unique_ptr` and `std::make_unique` over raw `new`/`delete`; keep member variables with trailing underscore and parameters without underscore per project style.
