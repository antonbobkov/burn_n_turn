# Phase 2: Wrappers and platform types — Execution Summary

**Phase:** 2  
**Plan:** 02-PLAN.md  
**Status:** Complete

## Wave 1: GraphicalInterface and SoundInterface

- **Task 1.1:** ProgramEngine now stores and accepts raw pointers
  (GraphicalInterface<Index>*, SoundInterface<Index>*). All call sites
  (sdl_game_runner_main, simulation.cc, simulation_test.cc) pass .get().
- **Task 1.2:** TowerDataWrap pGr and pSm changed to raw pointers.
- **Task 1.4:** SimpleGraphicalInterface, SimpleSoundInterface, CameraControl,
  ConvertImage, SoundInterfaceProxy, BackgroundMusicPlayer use raw pointers.
  sui_sdl_example fixed to pass pSnd.get() to SimpleSoundInterface.
- **Task 1.5:** Preloader, draw_utils (Drawer, ScalingDrawer), event
  (MakeSoundEvent), font_writer use raw pointers. DragonGameController and
  DrawStuff updated to raw pointers; level_controller pGraph.get() removed.
- **Task 1.3:** sdl_game_runner_main, test_sdl, gui_sdl_example, sui_sdl_example
  use std::unique_ptr for GUI/Sound (Option A: main holds two unique_ptrs per
  subsystem).
- **Task 1.6:** simulation.cc and simulation_test.cc use std::unique_ptr for
  mocks and Simple* wrappers; pass .get() to ProgramEngine.
- **Task 1.7:** SP_Info and get_class_name removed from GraphicalInterface,
  SoundInterface, SimpleGraphicalInterface, SimpleSoundInterface,
  SdlGraphicalInterface, SdlSoundInterface, MockGraphicalInterface,
  MockSoundInterface, DummySoundInterface. GuiGen.h no longer includes
  smart_pointer.h; SuiGen.h includes <string> instead. font_writer.h kept
  smart_pointer.h only for FontWriter (until Task 2.1).

## Wave 2: FontWriter

- **Task 2.1:** TowerDataWrap holds std::unique_ptr<FontWriter> pFancyNum;
  constructed with std::make_unique<FontWriter>(..., pGr, 2). DragonGameController
  takes and stores FontWriter*; font_writer.h: SP_Info and get_class_name
  removed from FontWriter; smart_pointer.h include removed.

## Verification

- Build: mingw32-make from build dir — success.
- ctest --output-on-failure — all 5 tests passed.
- simulation_test — passed.

## Commits

Per-task commits as per workflow; phase completion commit to follow.
