# Phase 2: Wrappers and platform types — Research

**Date:** 2025-02-16
**Status:** RESEARCH COMPLETE

## Objective

Answer: "What do I need to know to PLAN this phase well?" for migrating
GraphicalInterface, SoundInterface, FontWriter, and their implementations
to single ownership (unique_ptr at owner, T* elsewhere).

## Phase context (decisions)

- **FontWriter:** Single owner in **TowerDataWrap** (unique_ptr there).
- **GraphicalInterface and SoundInterface:** Single owners in **_main.cc**
  (each executable's main owns its own GUI and sound). Not in ProgramEngine.

## Current ownership and usage

### Creation sites (entry points)

| File | What is created | Passed to |
|------|-----------------|-----------|
| sdl_game_runner_main.cc | smart_pointer\<SdlGraphicalInterface\>, wrapped in SimpleGraphicalInterface; same for SdlSoundInterface | ProgramEngine ctor |
| simulation.cc (RunSimulation) | MockGraphicalInterface, MockSoundInterface, wrapped in Simple* | ProgramEngine ctor |
| simulation_test.cc | Same as simulation.cc (mocks) | ProgramEngine, DragonGameRunner |
| test_sdl.cc | SdlGraphicalInterface, SdlSoundInterface | Local use |
| gui_sdl_example.cc, sui_sdl_example.cc | Sdl* types | Local use |

Main (sdl_game_runner_main.cc) builds: `pGraph` (SdlGraphicalInterface),
`pGr` (SimpleGraphicalInterface wrapping pGraph), `pSound` (SdlSoundInterface),
`pSndMng` (SimpleSoundInterface wrapping pSound). Then
`ProgramEngine(..., pGr, pSndMng, ...)` and `GetGameRunner(pe)`.

### ProgramEngine and TowerDataWrap

- **game_runner_interface.h:** ProgramEngine holds
  `smart_pointer<GraphicalInterface<Index>> pGr` and
  `smart_pointer<SoundInterface<Index>> pSndMng`; constructor takes them
  by value (shared ownership).
- **dragon_game_runner.h:** TowerDataWrap holds `pGr`, `pSm` (same types);
  **dragon_game_runner.cc** in TowerDataWrap ctor does `pGr = pe.pGr; pSm =
  pe.pSndMng` (copy). TowerDataWrap also holds `smart_pointer<FontWriter>
  pFancyNum` and creates it with `make_smart(new FontWriter(..., pGr, 2))`.
  ScalingDrawer and NumberDrawer (Phase 3) also use pGr/pDr.

### Proxies and wrappers

- **GuiGen.h:** `SimpleGraphicalInterface<ImageHndl>` holds
  `smart_pointer<GraphicalInterface<ImageHndl>> pGr`; ctor takes it. Same
  pattern in `CameraControl`. `ConvertImage` takes
  `smart_pointer<GraphicalInterface<T>>`.
- **SuiGen.h:** `SimpleSoundInterface<SndHndl>` holds
  `smart_pointer<SoundInterface<SndHndl>> pSn`; ctor takes it.
- **sound_utils.h:** SoundInterfaceProxy, BackgroundMusicPlayer hold
  `smart_pointer<SoundInterface<Index>>` (non-owning; receive from caller).

### FontWriter and other consumers

- **font_writer.h / font_writer.cc:** FontWriter extends SP_Info; holds
  `smart_pointer<GraphicalInterface<Index>> pGr`; ctor takes it. Used only
  from TowerDataWrap (one creation site).
- **Preloader.h / Preloader.cc:** Preloader, ImageFlipper, ImagePainter hold
  `smart_pointer<GraphicalInterface<Index>>` and/or SoundInterface; take in
  ctor.
- **draw_utils.h / draw_utils.cc:** ScalingDrawer, NumberDrawer take/store
  smart_pointer to GraphicalInterface or ScalingDrawer (Phase 3).
- **game_utils/event.h:** MakeSoundEvent holds
  `smart_pointer<SoundInterface<Index>>` (Phase 1/Event scope).

### SP_Info usage

- `GraphicalInterface<ImageHndl>` and `SoundInterface<SndHndl>` extend
  `SP_Info` (GuiGen.h line 214, SuiGen.h line 7). So do Sdl* and Mock*
  implementations and Simple* (via base). After migration, no
  smart_pointer\<GraphicalInterface\> or smart_pointer\<SoundInterface\>
  remain, so CLEAN-01 allows removing SP_Info from these interfaces and
  their implementations (only if no other smart_pointer<T> points to them;
  GameRunner and others still use SP_Info).

## Target ownership (from context)

1. **GraphicalInterface / SoundInterface**
   - Owner: unique_ptr in the _main.cc that runs the app (sdl_game_runner_main.cc,
     and in simulation: RunSimulation / simulation_test create and own).
   - ProgramEngine: store raw pointers only; constructor takes
     GraphicalInterface<Index>*, SoundInterface<Index>* (or receives from
     caller who owns unique_ptrs).
   - TowerDataWrap: store raw pointers (from pe); no ownership.
   - SimpleGraphicalInterface / SimpleSoundInterface: **Option A** — main
     holds two unique_ptrs (backend + Simple*); Simple* ctor takes raw ptr
     to backend (non-owning). Pass Simple*.get() to ProgramEngine.
   - All other consumers (Preloader, draw_utils, sound_utils, FontWriter,
     CameraControl, ConvertImage, event): take/store raw pointers.

2. **FontWriter**
   - Owner: unique_ptr\<FontWriter\> in TowerDataWrap only.
   - FontWriter: store GraphicalInterface<Index>* (non-owning); ctor takes
     raw pointer. Remove SP_Info from FontWriter when no smart_pointer
     remains (CLEAN-01).

## Migration order (bottom-up)

- **Wave 1 — GUI and Sound:** Change entry points to create and own
  unique_ptrs (option: Simple* owns backend; main holds
  unique_ptr\<SimpleGraphicalInterface\>, unique_ptr\<SimpleSoundInterface\>).
  Change ProgramEngine to take and store raw pointers. Change TowerDataWrap,
  GuiGen (Simple*, CameraControl, ConvertImage), SuiGen (Simple*),
  Preloader, draw_utils, sound_utils, event, and font_writer to use raw
  pointers. Remove SP_Info from GraphicalInterface, SoundInterface, and
  their implementations (Sdl*, Mock*, Simple*) once no smart_pointer uses
  remain.
- **Wave 2 — FontWriter:** TowerDataWrap holds unique_ptr\<FontWriter\>;
  FontWriter ctor and member use GraphicalInterface<Index>*; remove SP_Info
  from FontWriter.

## Files to touch (summary)

- Entry: sdl_game_runner_main.cc, simulation.cc, simulation_test.cc,
  test_sdl.cc, gui_sdl_example.cc, sui_sdl_example.cc.
- Core: game_runner_interface.h, dragon_game_runner.h, dragon_game_runner.cc.
- Wrappers: GuiGen.h, GuiGen.cc, SuiGen.h, font_writer.h, font_writer.cc.
- Game utils: Preloader.h, Preloader.cc, draw_utils.h, draw_utils.cc,
  sound_utils.h, sound_utils.cc, event.h, event.cc.
- SDL/mock: GuiSdl.h, GuiSdl.cc, SuiSdl.h, SuiSdl.cc, GuiMock.*, SuiMock.*
  (only if we remove SP_Info from base/impl).
- game_runner_interface_impl.cc (GetGameRunner unchanged for Phase 2;
  ProgramEngine ctor signature change only).

## Risks and notes

- **GetGameRunner** returns smart_pointer\<GameRunner\>; Phase 2 does not
  change that (Phase 5). Main continues to hold that smart_pointer; only
  GUI/Sound/FontWriter ownership changes.
- **DragonGameController** and other controllers take TowerDataWrap* and use
  pGr, pSm, pFancyNum; they will receive raw pointers from TowerDataWrap
  (no change to their storage if they already use T* or need T*).
- **Build and test:** After each wave (or after each logical step within a
  wave), run build and ctest; run simulation_test.exe. VER-01, VER-02.
- **Skipped types:** If any wrapper/impl cannot be migrated (lifetime
  unclear), record for DOC-01 per success criteria.

## RESEARCH COMPLETE

Findings above are sufficient to produce an executable plan with waves and
tasks. No blockers.
