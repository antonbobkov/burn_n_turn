# Phase 2: Wrappers and platform types — Plan

---
wave: 1
depends_on: []
files_modified:
  - code/game_utils/game_runner_interface.h
  - code/game/dragon_game_runner.h
  - code/game/dragon_game_runner.cc
  - code/wrappers/GuiGen.h
  - code/wrappers/SuiGen.h
  - code/wrappers/font_writer.h
  - code/wrappers/font_writer.cc
  - code/game_utils/Preloader.h
  - code/game_utils/Preloader.cc
  - code/game_utils/draw_utils.h
  - code/game_utils/draw_utils.cc
  - code/game_utils/sound_utils.h
  - code/game_utils/sound_utils.cc
  - code/game_utils/event.h
  - code/wrappers_sdl_impl/sdl_game_runner_main.cc
  - code/simulation/simulation.cc
  - code/simulation/simulation_test.cc
  - code/wrappers_sdl_impl/test_sdl.cc
  - code/wrappers_sdl_impl/gui_sdl_example.cc
  - code/wrappers_sdl_impl/sui_sdl_example.cc
autonomous: false
---

**Phase:** 2 — Wrappers and platform types  
**Goal (ROADMAP):** Migrate GraphicalInterface, SoundInterface, FontWriter,
and their implementations (SDL, mocks) and proxies. Single owner at entry
(main) for GUI/Sound; single owner in TowerDataWrap for FontWriter. Rest use
T*.

## must_haves (goal-backward verification)

- [ ] GraphicalInterface and SoundInterface: single owner in each _main.cc
  (sdl_game_runner_main, simulation RunSimulation, simulation_test); all
  other sites use T*.
- [ ] FontWriter: single owner in TowerDataWrap (unique_ptr); all other uses
  FontWriter*.
- [ ] SP_Info removed from GraphicalInterface, SoundInterface, and their
  implementations (Sdl*, Mock*, Simple*) and from FontWriter where no
  smart_pointer use remains (CLEAN-01).
- [ ] Build succeeds; ctest and simulation_test pass (VER-01, VER-02).
- [ ] Any type skipped (lifetime unclear) recorded for DOC-01.

---

## Wave 1: GraphicalInterface and SoundInterface to unique_ptr at main

Execute in order. Run build and tests after each task (or after 1.3 and
1.6).

### Task 1.1 — ProgramEngine and GameRunner interface to raw pointers

- In `game_runner_interface.h`: Change ProgramEngine to store
  `GraphicalInterface<Index>* pGr` and `SoundInterface<Index>* pSndMng`
  (non-owning). Constructor parameters: accept `GraphicalInterface<Index>*`,
  `SoundInterface<Index>*` (callers will pass `.get()` from unique_ptrs).
- Do not change GetGameRunner return type (Phase 5).
- Update all ProgramEngine construction sites so the project builds: in
  sdl_game_runner_main.cc, simulation.cc, and simulation_test.cc, pass raw
  pointers into ProgramEngine (e.g. from the current smart_pointer or, once
  switched, from unique_ptr .get()). Full switch to unique_ptr at entry is
  completed in Task 1.3 (SDL) and Task 1.6 (simulation).
- Build and run ctest; run simulation_test.

### Task 1.2 — TowerDataWrap to raw pointers for GUI and Sound

- In `dragon_game_runner.h`: Change `pGr` and `pSm` to
  `GraphicalInterface<Index>*` and `SoundInterface<Index>*`.
- In `dragon_game_runner.cc`: TowerDataWrap ctor assigns
  `pGr = pe.pGr; pSm = pe.pSndMng` (already raw pointers after 1.1). Leave
  pDr, pNum, pBigNum, pFancyNum as-is for this task (smart_pointer; FontWriter
  in Wave 2).
- Build and run ctest; run simulation_test.

### Task 1.3 — Entry points: SDL main and examples to unique_ptr

- In `sdl_game_runner_main.cc`: Use **Option A**: main holds
  unique_ptr\<SdlGraphicalInterface\> and
  unique_ptr\<SimpleGraphicalInterface\<SdlImage*\>\> (Simple* ctor takes
  raw ptr to Sdl*); pass pGr.get() to ProgramEngine. Same for Sound: main
  holds unique_ptr\<SdlSoundInterface\> and
  unique_ptr\<SimpleSoundInterface\<Mix_Chunk*\>\> (Simple* ctor takes raw
  ptr), pass .get() to ProgramEngine. Replace make_smart with
  std::make_unique where ownership is taken.
- In `test_sdl.cc`, `gui_sdl_example.cc`, `sui_sdl_example.cc`: Switch to
  unique_ptr for locally owned GUI/Sound; pass raw pointers where needed.
- Build and run ctest; run simulation_test.

### Task 1.4 — Proxies (Simple*, CameraControl, ConvertImage, Sound) to raw or owned

- In `GuiGen.h`: SimpleGraphicalInterface: hold
  `GraphicalInterface<ImageHndl>*` and take in ctor (non-owning; main holds
  two unique_ptrs per Option A). CameraControl: change `pGr` to
  `GraphicalInterface<ImageHndl>*`. ConvertImage: change parameter to
  `GraphicalInterface<T>*`.
- In `SuiGen.h`: SimpleSoundInterface: hold `SoundInterface<SndHndl>*` and
  take in ctor (non-owning; same as Option A for GUI).
- In `sound_utils.h`: SoundInterfaceProxy and BackgroundMusicPlayer: change
  to store `SoundInterface<Index>*` (non-owning).
- Build and run ctest; run simulation_test.

### Task 1.5 — Preloader, draw_utils, event, font_writer to raw pointers

- In `Preloader.h` and `Preloader.cc`: ImageFlipper, ImagePainter, Preloader
  — change `pGr` and `pSn` to raw pointers (GraphicalInterface<Index>*,
  SoundInterface<Index>*); ctors take raw pointers.
- In `draw_utils.h` and `draw_utils.cc`: ScalingDrawer, NumberDrawer — change
  any smart_pointer\<GraphicalInterface\> to raw pointer; keep
  smart_pointer\<ScalingDrawer\> for now (Phase 3).
- In `event.h` (and event.cc if needed): MakeSoundEvent — change to
  `SoundInterface<Index>*` (non-owning).
- In `font_writer.h` and `font_writer.cc`: Change FontWriter to store
  `GraphicalInterface<Index>* pGr`; ctor takes raw pointer. Do not yet
  change TowerDataWrap to own unique_ptr\<FontWriter\> (Wave 2). Leave
  FontWriter as smart_pointer in TowerDataWrap for this task so that only
  GUI/Sound ownership is changed in Wave 1; FontWriter still receives pGr
  as raw ptr.
- Build and run ctest; run simulation_test.

### Task 1.6 — Simulation and simulation_test to unique_ptr

- In `simulation.cc` (RunSimulation): Create MockGraphicalInterface and
  MockSoundInterface (and Simple* wrappers) with std::unique_ptr; pass
  .get() to ProgramEngine. Ensure ProgramEngine and DragonGameRunner receive
  raw pointers only.
- In `simulation_test.cc`: Same change — own GUI/Sound with unique_ptr,
  pass raw pointers to ProgramEngine and any other consumers. Update test
  setup so that unique_ptrs outlive the run.
- Build and run ctest; run simulation_test.

### Task 1.7 — Remove SP_Info from GUI and Sound hierarchy (CLEAN-01)

- After no smart_pointer\<GraphicalInterface\> or smart_pointer\<SoundInterface\>
  remain: Remove SP_Info base from GraphicalInterface, SoundInterface,
  SimpleGraphicalInterface, SimpleSoundInterface, and from SdlGraphicalInterface,
  SdlSoundInterface, MockGraphicalInterface, MockSoundInterface (and any
  other impls). Remove get_class_name overrides if they only served
  smart_pointer/SP_Info. Ensure GameRunner and other types that still use
  SP_Info are unchanged.
- Remove #include "utils/smart_pointer.h" from files that no longer use
  smart_pointer for GUI/Sound.
- Build and run ctest; run simulation_test.

---

## Wave 2: FontWriter to unique_ptr in TowerDataWrap

### Task 2.1 — FontWriter ownership and SP_Info removal

- In `dragon_game_runner.h`: Change `smart_pointer<FontWriter> pFancyNum` to
  `std::unique_ptr<FontWriter> pFancyNum`.
- In `dragon_game_runner.cc`: Construct FontWriter with
  std::make_unique<FontWriter>(..., pGr, 2) (pGr already raw pointer).
  Pass pFancyNum.get() or pFancyNum.get() to DragonGameController and
  anywhere else that needs FontWriter* (e.g. existing pFancyNum usage:
  ensure all are raw pointer or reference).
- In `font_writer.h` and `font_writer.cc`: FontWriter already uses
  GraphicalInterface<Index>* from Wave 1. Remove SP_Info base from
  FontWriter (CLEAN-01). Remove #include "utils/smart_pointer.h" from
  font_writer.h if no longer needed.
- Update dragon_game_controller and any code that stores or takes
  FontWriter* to use raw pointer only (no smart_pointer<FontWriter>).
- Build and run ctest; run simulation_test.

---

## Verification (every wave / task)

- From project root (code): sibling `build` dir. Run: `cd build; cmake
  ../code -G "MinGW Makefiles"; mingw32-make`. Build must succeed.
- Run: `ctest --output-on-failure` from build. All tests pass.
- Run: `cd bin; .\simulation_test.exe` (or equivalent). simulation_test
  passes.

## PLANNING COMPLETE

Phase 2 plan: 2 waves. Wave 1 = GUI/Sound to unique_ptr at main and raw
elsewhere; Wave 2 = FontWriter to unique_ptr in TowerDataWrap. Execute in
order; verify after each task or at wave boundaries.
