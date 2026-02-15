---
name: Smart pointer rename and make_smart
overview: Rename SmartPointer to smart_pointer, disallow construction/assignment from raw pointer in favor of make_smart(), add unit tests, and follow how_to_plan.md with an implementation summary document.
todos: []
isProject: false
---

# Smart pointer rename, make_smart, and tests

## Scope

- **Rename files:** [utils/SmartPointer.h](utils/SmartPointer.h) → `smart_pointer.h`, [utils/SmartPointer.cpp](utils/SmartPointer.cpp) → `smart_pointer.cpp`.
- **Rename type:** Class `SP` → `smart_pointer` (template name and all references inside the header and in the codebase).
- **API change:** Remove public `SP(T* pPointTo_)` constructor and `SP<T>& operator=(T* pPointTo_)`. Add free function `template <class T> smart_pointer<T> make_smart(T* pPointTo_)` (friend of `smart_pointer`) that constructs from a raw pointer.
- **Tests:** New [utils/smart_pointer_test.cpp](utils/smart_pointer_test.cpp) with Catch2 unit tests.
- **Process:** Follow [how_to_plan.md](how_to_plan.md): create implementation summary doc first, update it after each step, finalize at the end.

## Key code in current header

- [utils/SmartPointer.h](utils/SmartPointer.h): `template <class T> class SP` with `SP(T* pPointTo_)`, `SP<T>& operator=(T* pPointTo_)`, `CONSTRUCT`/`EQUAL` helpers, and friend `template <class N> class SP` / `class SSP`. SSP and ASSP reference `SP` in constructors and `operator=` (they take `const SP<N>&`); these become `smart_pointer`.
- [utils/SmartPointer.cpp](utils/SmartPointer.cpp): Includes `"SmartPointer.h"`, uses `SP_Info` and `CleanIslandSeeded`/`CleanIsland`; no direct use of `SP` type name in declarations.

## Implementation steps

### Step 1: Create implementation summary document

- Add **smart_pointer_rename_implementation_summary.md** (same base name as plan, `_implementation_summary` suffix) under `.cursor/plans/` or project docs.
- Sections: Summary (to fill at end), then one section per step below. For each step: "Deviations", "Issues", "Solutions", "Tips".
- Reminder: after every later step, update this document.

### Step 2: Rename files and update include guard / includes

- Rename `utils/SmartPointer.h` → `utils/smart_pointer.h` and `utils/SmartPointer.cpp` → `utils/smart_pointer.cpp`.
- In `smart_pointer.h`: change include guard from `SMART_POINTER_HEADER_...` to e.g. `SMART_POINTER_H` (or `GUI_SMART_POINTER_H` if preferred for consistency).
- In `smart_pointer.cpp`: `#include "smart_pointer.h"`.
- In [utils/CMakeLists.txt](utils/CMakeLists.txt): replace `SmartPointer.h` and `SmartPointer.cpp` with `smart_pointer.h` and `smart_pointer.cpp` in the `add_library(Utils ...)` list.
- **Include updates (codebase):** Replace `#include "SmartPointer.h"` with `#include "smart_pointer.h"` everywhere. Known includes: [utils/SmartPointer.cpp](utils/SmartPointer.cpp), [game_utils/Preloader.h](game_utils/Preloader.h), [wrappers/GuiGenHdr.h](wrappers/GuiGenHdr.h), [wrappers/SuiGen.h](wrappers/SuiGen.h), [game_utils/MessageWriter.h](game_utils/MessageWriter.h). Any other file that includes one of these gets the header transitively; only direct includers need the string change.
- Build and fix any broken includes.

### Step 3: Rename class SP to smart_pointer and add make_smart

- In `smart_pointer.h`:
  - Replace template declaration `template <class T> class SP` with `template <class T> class smart_pointer`.
  - Replace all occurrences of `SP` as the class name (constructors, destructor, `operator=`, `EQUAL`, `CONSTRUCT`, `operator==`, `operator!=`, return types, and friend declarations) with `smart_pointer`. Keep `SP_Info`, `SSP_Base`, `SSP`, `ASSP` names unchanged; only the former `SP` type is renamed.
  - Friend declarations: `template <class N> friend class SSP` stays; change `template <class N> friend class SP` to `template <class N> friend class smart_pointer`.
  - Add **private** constructor: `smart_pointer(T* pPointTo_);` (body: call `CONSTRUCT(pPointTo_)`).
  - Remove the **public** constructor that takes `T*` (currently `SP(T* pPointTo_)`).
  - Remove `smart_pointer<T>& operator=(T* pPointTo_)`.
  - Declare and define friend factory:  
  `template <class T> smart_pointer<T> make_smart(T* pPointTo_) { return smart_pointer<T>(pPointTo_); }`  
  (friend of `smart_pointer<T>` so it can call the private constructor).
  - Out-of-class template definitions that reference `SP<T>` (e.g. `SP<T>::SP(const SSP<N>&)` and `SP<T>::operator=(const SSP<N>&)`) become `smart_pointer<T>::smart_pointer(...)` and `smart_pointer<T>::operator=(...)`.
- In `smart_pointer.cpp`: any use of type `SP` (e.g. in comments) rename to `smart_pointer` if present; the .cpp does not declare `SP` types, only `SP_Info` and functions taking `SP_Info*`.
- Build to ensure the header compiles.

### Step 4: Replace SP with smart_pointer and raw-pointer construction with make_smart (codebase)

- **Type renames:** In all source and headers that use the type, replace `SP<...>` with `smart_pointer<...>`. This touches many files under `game/`, `game_utils/`, `wrappers/`, `simulation/`, `utils/` (see grep counts: ~40+ files with `SP<`).
- **Construction from raw pointer:** Every initialization of the form `SP<T> var = new T(...)` or `SP<T> var(new T(...))` becomes `smart_pointer<T> var = make_smart<T>(new T(...))` (or `make_smart(new T(...))` where the compiler can deduce `T`; prefer explicit `make_smart<T>` where deduction might be ambiguous). Do **not** add `make_smart` for copy/move construction or assignment from another `smart_pointer` or `SSP`/`ASSP`.
- **Assignment from raw pointer:** Any `var = new T(...)` where `var` is a `smart_pointer<T>` must be changed to `var = make_smart<T>(new T(...))`. Grep for `= new` in files that use `SP`/`smart_pointer` to find these.
- **SSP/ASSP:** These still take `const SP<N>&` in the current header; after the rename they take `const smart_pointer<N>&`. No change at call sites except that the type name is now `smart_pointer`.
- **common.h / General.h:** [game/common.h](game/common.h) pulls in GuiGen and Preloader (which pull in the pointer header). No direct include of SmartPointer in common.h; type name changes in headers that common.h includes will propagate.
- Build and run existing tests (e.g. file_utils_test, wrappers_mock_test, simulation_test) to confirm nothing broke.

### Step 5: Add smart_pointer_test.cpp and register it in CMake

- **New file** [utils/smart_pointer_test.cpp](utils/smart_pointer_test.cpp):
  - Use Catch2 (same style as [utils/file_utils_test.cpp](utils/file_utils_test.cpp)).
  - Include `smart_pointer.h`. Define a minimal ref-counted type (inheriting `SP_Info`) for tests.
  - **Test cases (examples):**
    - Default construction: `smart_pointer<T> p;` then `REQUIRE(p.GetRawPointer() == nullptr)` (or equivalent).
    - Construction via `make_smart`: `auto p = make_smart<T>(new T(...));` then `REQUIRE(p->... )`, `REQUIRE(p.GetRawPointer() != nullptr)`.
    - Copy constructor: two pointers to same object; after one is reset/destroyed, the other still valid; ref count behavior (if observable) or at least no double-free.
    - Assignment from another `smart_pointer`: `a = b;` then check both point to same object.
    - Disallow raw-pointer construction: verify that `smart_pointer<T> p(new T());` does not compile (optional; can be a commented compile-fail or a short note in the summary).
    - GetRawPointer, operator->, operator* (and const versions if exposed).
  - Keep tests focused and lines under 80 chars where reasonable; follow project style.
- In [utils/CMakeLists.txt](utils/CMakeLists.txt): add an executable for `smart_pointer_test.cpp`, link `Utils` and `Catch2::Catch2WithMain`, and `add_test(NAME smart_pointer_test COMMAND ...)`.
- Build and run `smart_pointer_test`.

### Step 6: Finalize implementation summary

- Fill the top-level Summary section of **smart_pointer_rename_implementation_summary.md** with: deviations from the plan, issues hit, how they were solved, and tips for future similar refactors.
- Ensure each step’s subsection is updated with the same structure (Deviations, Issues, Solutions, Tips).

## Dependency and order

- Step 1 (doc) first.
- Step 2 (rename files + includes) before changing API so that renames are consistent.
- Step 3 (class rename + make_smart, remove raw ctor/assign) next so the API is fixed in one place.
- Step 4 (codebase call sites) must follow Step 3 so that all usages use `smart_pointer` and `make_smart`.
- Step 5 (tests) can be done after Step 4, or the test file can be added earlier and updated in Step 4 to use `smart_pointer` and `make_smart`.
- Step 6 last.

## Files to touch (non-exhaustive)


| Area         | Files                                                                                                                                                                                                                                                                 |
| ------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Utils        | smart_pointer.h (was SmartPointer.h), smart_pointer.cpp, smart_pointer_test.cpp (new), CMakeLists.txt, any utils file that included SmartPointer.h                                                                                                                    |
| Game         | core.h/cpp, game.h, gameplay.h/cpp, screen_controllers.h/cpp, level.h/cpp, entities.h/cpp, critters.h/cpp, dragon.h/cpp, fireball.h/cpp, tutorial.h/cpp, critter_generators.h/cpp, game_runner_interface_impl.cc, and others that use SP<> or include headers that do |
| Game utils   | Preloader.h/cpp, MessageWriter.h, game_runner_interface.h, event.h/cpp                                                                                                                                                                                                |
| Wrappers     | GuiGenHdr.h, GuiGen.cpp, SuiGen.h                                                                                                                                                                                                                                     |
| Wrappers SDL | sdl_game_runner_main.cpp, gui_sdl_example.cpp, test_sdl.cpp, sui_sdl_example.cc, GuiSdl.cpp                                                                                                                                                                           |
| Simulation   | simulation.cpp, simulation_test.cpp                                                                                                                                                                                                                                   |
| Docs         | .cursor/plans/smart_pointer_rename_implementation_summary.md (new)                                                                                                                                                                                                    |


## Risk / notes

- **SSP/ASSP** still have constructors and `operator=` taking raw `T*`; the plan only changes `SP` → `smart_pointer` and the `SP(T*)` / `SP::operator=(T*)` removal. No change to SSP/ASSP API unless you want to do that in a follow-up.
- **Forward declarations:** Any `template <class T> class SP;` must become `template <class T> class smart_pointer;` where it appears (e.g. in the same header before use).
- **Line length:** Keep new and edited lines under 80 characters per project rules.

