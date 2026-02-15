# Smart pointer rename implementation summary

## Summary

All steps are complete. **Deviations:** AdvancedController::Init was given a
second parameter `const LevelLayout &lvl` because setup code that uses lvl
(vFreq, vRoadGen, vCastleLoc, nTimer, etc.) was moved into Init; call site
is `pAd->Init(pAd, vLvl[i])`. **Issues:** (1) Bulk replace SP< produced
ASsmart_pointer/Ssmart_pointer; reverted to ASSP/SSP. (2) Build error: `lvl`
not in scope inside Init; fixed by passing lvl into Init. **Solutions:** Type
rename and make_smart usage across codebase; two-phase init with pSelf and
lvl for AdvancedController; smart_pointer_test.cpp added with Catch2 tests.
**Tips:** When moving constructor logic into Init(), ensure all data (e.g.
LevelLayout) is passed in or stored in the constructor. After bulk SP<
replace, grep for ASsmart_pointer and Ssmart_pointer.

---

## Step 1: Create implementation summary document

- **Deviations:** None.
- **Issues:** None.
- **Solutions:** N/A.
- **Tips:** N/A.

---

## Step 2: Rename files and update include guard / includes

- **Deviations:** Include guard set to `SMART_POINTER_H` (not
  `SMART_POINTER_HEADER_...`).
- **Issues:** None.
- **Solutions:** Created new `utils/smart_pointer.h` and `utils/smart_pointer.cpp`;
  updated `utils/CMakeLists.txt`; replaced `#include "SmartPointer.h"` with
  `#include "smart_pointer.h"` in Preloader.h, GuiGenHdr.h, SuiGen.h,
  MessageWriter.h; deleted old SmartPointer.h and SmartPointer.cpp.
- **Tips:** N/A.

---

## Step 3: Rename class SP to smart_pointer and add make_smart

- **Deviations:** None.
- **Issues:** Large block replace of the whole SP class failed; did smaller
  edits (forward decl, SP_Info friend, class body, SSP friends/params,
  out-of-class defs, comment).
- **Solutions:** In `smart_pointer.h`: forward declaration and SP_Info friend
  changed to `smart_pointer`; class renamed to `smart_pointer` with private
  `explicit smart_pointer(T*)`; public `SP(T*)` and `operator=(T*)` removed;
  friend `make_smart` added; `template <class T> smart_pointer<T> make_smart(T*
  pPointTo_) { return smart_pointer<T>(pPointTo_); }` added after class; SSP
  friend and all `const SP<N>&` → `const smart_pointer<N>&`; out-of-class
  definitions updated to `smart_pointer<T>::...`. Comment “SP:” → “smart_pointer:”.
- **Tips:** Keep `SSP` and `ASSP` names unchanged; only type `SP` becomes
  `smart_pointer`.

---

## Step 4: Replace SP with smart_pointer and raw-pointer construction with make_smart

- **Deviations:** Two-phase init added for AdvancedController
  (Init(pSelf, const LevelLayout &lvl)) and TwrGlobalController
  (StartUp(pSelf)) so that child types that take `smart_pointer<...>` can be
  given a self pointer; Init also takes lvl because setup uses it.
- **Issues:** (1) Global replace of `SP<` turned `ASSP<` into `ASsmart_pointer<`;
  fixed by replacing `ASsmart_pointer` with `ASSP` in game headers/sources.
  (2) Same for `SSP<` → `Ssmart_pointer<` in several headers; fixed by
  replacing `Ssmart_pointer` with `SSP`. (3) No public ctor from T*, so
  initializations like `smart_pointer<T> x = new T(...)` and `x = new T(...)`
  must use `make_smart(new T(...))`. (4) Comparisons like `p != 0` or `p == 0`
  for smart_pointer/SSP fail; use `p.GetRawPointer() != 0` (or `== 0`). (5)
  Default init: use `pSc()` or `pTexter()` instead of `pSc(0)` / `pTexter(0)`.
  (6) Assigning “release” use `p = smart_pointer<T>()` instead of `p = 0`. (7)
  Event.cpp Trigger(pE): null check via GetRawPointer(). (8) GetGlobalController
  must return make_smart(new ...). (9) AdvancedController and TwrGlobalController
  constructors that take “this” for child controllers now need a
  smart_pointer to self: TwrGlobalController stores pSelf and StartUp(pSelf);
  AdvancedController has Init(pSelf, lvl) and core.cpp calls
  pAd->Init(pAd, vLvl[i]). (10) Init() used lvl (vFreq, vRoadGen, etc.) which
  was only in constructor scope; fixed by adding const LevelLayout &lvl to
  Init() and passing vLvl[i] at call site.
- **Solutions:** Replaced `SP<` with `smart_pointer<` in all relevant files
  (wrappers, game_utils, game, simulation, event.h/cpp, tutorial, critters,
  fireball, dragon, game_runner_interface_impl.cc, test_sdl, gui_sdl_example,
  sui_sdl_example). Wrapped every “= new” and “(new” for owning pointers with
  make_smart(...). Fixed ASSP/Ssmart_pointer/ASsmart_pointer back to ASSP/SSP.
  Added pSelf parameter to StartUp and stored in TwrGlobalController; Restart()
  calls StartUp(pSelf).   Added Init(smart_pointer<AdvancedController> pSelf,
  const LevelLayout &lvl) to AdvancedController and moved generator/road/
  castle/dragon/tutorial setup into Init; core.cpp calls
  pAd->Init(pAd, vLvl[i]). Replaced null
  checks with GetRawPointer() != 0 (or == 0) in event.cpp, tutorial.cpp,
  screen_controllers.cpp, critters.cpp, dragon.cpp. Replaced pSc(0), pTexter(0),
  BackgroundMusicPlayer pSnd(0) with default ctors or pSelf.
- **Tips:** Grep for `= new` and `(new` in files that use smart_pointer to find
  every place that needs make_smart. After bulk SP< → smart_pointer<, grep for
  ASsmart_pointer and Ssmart_pointer and revert to ASSP and SSP.

---

## Step 5: Add smart_pointer_test.cpp and register in CMake

- **Deviations:** None.
- **Issues:** None.
- **Solutions:** Added utils/smart_pointer_test.cpp with a minimal TestObj
  (inherits SP_Info) and Catch2 tests: default construction, make_smart,
  copy constructor, assignment, assign empty (release), operator==/!=. Note
  in comment that raw-pointer construction is disallowed. Registered
  smart_pointer_test executable in utils/CMakeLists.txt, linked Utils and
  Catch2::Catch2WithMain, add_test(NAME smart_pointer_test ...).
- **Tips:** Use a small SP_Info-derived type for tests; keep test names and
  lines under 80 chars to match project style.

---

## Step 6: Finalize implementation summary

- **Deviations:** None.
- **Issues:** N/A.
- **Solutions:** Filled Summary with deviations (Init(lvl)), issues (bulk
  replace, lvl scope), solutions, and tips. Updated Step 4 with issue (10)
  and Init(pSelf, lvl) solution. Filled Step 5 and Step 6 subsections.
- **Tips:** N/A.
