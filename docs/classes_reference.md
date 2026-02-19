# Classes and Structs Reference

Table of all classes and structs: code location, kind (struct/class),
name, base types, and short description.

| Location | Kind | Name | Inherits from | Description |
|----------|------|------|---------------|-------------|
| game/controller/basic_controllers.h | struct | AutoAdvanceController | EntityListController | Advances (Next) when only background is left or on input (e.g. logo). |
| game/controller/basic_controllers.h | struct | Cutscene | EntityListController | Cutscene: chase with runner and chaser critters. |
| game/controller/basic_controllers.h | struct | DragonScoreController | EntityListController | Dragon score screen; exits on click or timer. |
| game/controller/basic_controllers.h | struct | EntityListController | GameController | Draw/update/consumable lists; Update runs Move, Update, then draw by priority. |
| game/controller/basic_controllers.h | struct | StartScreenController | EntityListController | Start screen; advance on key/click. |
| game/controller/buy_now_controller.h | struct | BuyNowController | EntityListController | Buy-now screen: slime animations and timer. |
| game/controller/buy_now_controller.h | struct | SlimeUpdater | VisualEntity | Updates slime animation for buy-now screen. |
| game/controller/dragon_game_controller.h | struct | DragonGameController | — | Global game state: levels, active controller, graphics/sound, score, savable options, music. |
| game/controller/game_controller_interface.h | struct | GameController | SP_Info | Base controller: pGl, rBound; virtual OnKey, OnMouse, Update, Fire. |
| game/controller/level_controller.cc | struct | AdNumberDrawer | VisualEntity | Draws numbers (e.g. ads) on screen; local to level. |
| game/controller/level_controller.cc | struct | BonusDrawer | VisualEntity | Draws bonus visuals; local to level. |
| game/controller/level_controller.h | struct | LevelController | EntityListController | Main level controller: castles, roads, dragon, generators, bonuses, input. |
| game/controller/menu_controller.h | struct | MenuController | EntityListController | Pause/main menu; MenuDisplay, resume position. |
| game/controller/menu_controller.h | struct | MenuDisplay | EventEntity, VisualEntity | In-game menu: entries, caret, mouse/key, submenus. |
| game/controller/menu_controller.h | struct | MenuEntry | SP_Info | One menu item: size, label, callback, disabled flag. |
| game/controller/menu_controller.h | struct | MenuEntryManager | — | Holds menu entries and current selection index. |
| game/critter_generators.h | struct | KnightGenerator | EventEntity | Spawns knight critters. |
| game/critter_generators.h | struct | MageGenerator | EventEntity | Spawns mage critters. |
| game/critter_generators.h | struct | PrincessGenerator | EventEntity | Spawns princess critters. |
| game/critter_generators.h | struct | SkellyGenerator | EventEntity | Spawns skeleton critters. |
| game/critter_generators.h | struct | TraderGenerator | EventEntity | Spawns trader critters. |
| game/critters.h | struct | Castle | Critter | Castle building; holds princesses, dragon reference. |
| game/critters.h | struct | FloatingSlime | SimpleVisualEntity | Non-physical slime visual. |
| game/critters.h | struct | Ghostiness | EventEntity | Ghost state/effect for knight. |
| game/critters.h | struct | Knight | Critter, ConsumableEntity | Knight unit; chases princess/castle, can become ghost. |
| game/critters.h | struct | Mage | Critter, ConsumableEntity | Mage unit; can summon slimes. |
| game/critters.h | struct | MegaSliminess | EventEntity | Mega slime visual/state effect. |
| game/critters.h | struct | MegaSlime | Critter, ConsumableEntity | Large slime enemy. |
| game/critters.h | struct | Princess | Critter, ConsumableEntity | Princess unit; captured by dragon. |
| game/critters.h | struct | Slime | Critter, ConsumableEntity | Slime enemy. |
| game/critters.h | struct | Sliminess | EventEntity | Slime visual/state effect. |
| game/critters.h | struct | Trader | Critter, ConsumableEntity | Trader unit; drops bonus. |
| game/dragon.h | struct | ButtonSet | — | Set of key/button codes for input (fire, steer, etc.). |
| game/dragon.h | struct | Dragon | Critter | Player dragon; bonuses, fireballs, steer/shoot, collision. |
| game/dragon.h | struct | DragonLeash | — | Dragon steering: velocity, tilt, speed, trackball params. |
| game/dragon_game_runner.h | class | DragonGameRunner | GameRunner | Top-level tower game; owns TowerDataWrap, runs level flow. |
| game/dragon_game_runner.h | class | TowerDataWrap | — | Holds exit event and graphics/sound interfaces for setup. |
| game/entities.h | struct | Animation | SimpleVisualEntity | Looping animation entity. |
| game/entities.h | struct | AnimationOnce | SimpleVisualEntity | One-shot animation; sets bExist false when finished. |
| game/entities.h | struct | BonusScore | EventEntity, VisualEntity | Floating "+N" score text at a point; animates then removes. |
| game/entities.h | struct | ConsumableEntity | PhysicalEntity | Can be collected/hit (GetType, OnHit, GetImage). |
| game/entities.h | struct | Critter | PhysicalEntity, SimpleVisualEntity | Moving unit: position, velocity, bounds, radius; Move steps and clamps. |
| game/entities.h | struct | Entity | SP_Info | Root entity; bExist flag, virtual dtor. |
| game/entities.h | struct | EventEntity | Entity | Entity that can Move and Update each frame. |
| game/entities.h | struct | FancyCritter | PhysicalEntity, SimpleVisualEntity | Critter that advances position and toggles frame on a timer. |
| game/entities.h | struct | HighScoreShower | VisualEntity | Draws high score in a rectangle. |
| game/entities.h | struct | IntroTextShower | VisualEntity | Shows intro text. |
| game/entities.h | struct | PhysicalEntity | ScreenEntity | Entity with position and radius for collision. |
| game/entities.h | struct | ScreenEntity | Entity | Entity with a screen position (GetPosition). |
| game/entities.h | struct | ScreenPos | — | Sort key for draw order: priority and height (operator<). |
| game/entities.h | struct | SimpleSoundEntity | EventEntity | Plays SoundSequence on a timer; sets bExist false when done. |
| game/entities.h | struct | SimpleVisualEntity | EventEntity, VisualEntity | VisualEntity with ImageSequence; timer or step animation. |
| game/entities.h | struct | SoundControls | EventEntity | Sound on/off control entity; drives BackgroundMusicPlayer. |
| game/entities.h | struct | StaticImage | VisualEntity | Draws a single image at a position. |
| game/entities.h | struct | StaticRectangle | VisualEntity | Draws a filled rectangle. |
| game/entities.h | struct | TextDrawEntity | VisualEntity | Draws multi-line text via NumberDrawer at a position. |
| game/entities.h | struct | VisualEntity | ScreenEntity | Entity that can Draw via ScalingDrawer; has priority. |
| game/fireball.h | struct | Chain | — | Chain reaction generation count or infinite; Evolve/IsLast. |
| game/fireball.h | struct | ChainExplosion | AnimationOnce, PhysicalEntity | Expanding explosion; hits consumables, spawns chain. |
| game/fireball.h | struct | CircularFireball | Fireball, TimedFireballBonus | Fireball that orbits at a radius. |
| game/fireball.h | struct | Fireball | Critter | Player fireball; bThrough, hits ConsumableEntities. |
| game/fireball.h | struct | FireballBonus | Entity | Power-up state: named float/unsigned/bool maps, nNum; += merges. |
| game/fireball.h | struct | FireballBonusAnimation | Animation, PhysicalEntity | Pick-up animation with radius; overlaps trigger collection. |
| game/fireball.h | struct | KnightOnFire | Critter | Knight on fire state; random velocity, timer. |
| game/fireball.h | struct | TimedFireballBonus | FireballBonus, EventEntity | FireballBonus that updates on a timer. |
| game/level.h | struct | BrokenLine | — | Polyline(s); vEdges, CloseLast, Add, AddLine, Join, I/O. |
| game/level.h | struct | FancyRoad | Road | Road that draws tiled image; uses LevelController. |
| game/level.h | struct | LevelLayout | — | One level: bounds, spawn line, castle positions, roads, timer. |
| game/level.h | struct | Road | VisualEntity | Level road segment; vertical/horizontal, bounds; Draw gray bar. |
| game/level.h | class | SegmentSimpleException | MyException | Exception for BrokenLine segment errors. |
| game/tutorial.h | struct | TutorialLevelOne | — | First tutorial level data/behavior. |
| game/tutorial.h | struct | TutorialLevelTwo | — | Second tutorial level data/behavior. |
| game/tutorial.h | struct | TutorialTextEntity | EventEntity, VisualEntity | Tutorial text on screen. |
| game_utils/draw_utils.h | struct | Drawer | SP_Info | Base for drawing an image at a point; ScalingDrawer adds scale and color key. |
| game_utils/draw_utils.h | struct | NumberDrawer | SP_Info | Draws digits/words from a font bitmap; CacheColor/DrawColorWord for recolored text. |
| game_utils/draw_utils.h | struct | ScalingDrawer | Drawer | Drawer with scale factor and transparency color key; LoadImage, Scale. |
| game_utils/event.h | class | CpSwitchEvent\<A,B\> | Event | Event that assigns objCurr = copy of objNew on Trigger. |
| game_utils/event.h | class | Event | SP_Info | Abstract event; Trigger() invoked by Trigger(). |
| game_utils/event.h | struct | SequenceOfEvents | Event | Runs a sequence of events on Trigger. |
| game_utils/event.h | class | SwitchEvent\<A,B\> | Event | Event that assigns objCurr = objNew on Trigger. |
| game_utils/event.h | class | TerminatorEvent\<T\> | Event | Event that calls pObject->Terminate() on Trigger. |
| game_utils/game_runner_interface.h | struct | ProgramEngine | — | Engine wiring: exit event, GI, sound, message writer, FileManager. |
| game_utils/game_runner_interface.h | struct | ProgramInfo | — | Program config: resolution, title, framerate, flags. |
| game_utils/game_runner_interface.h | class | GameRunner | SP_Info | Top-level input/update: Update, KeyDown, MouseMove, Fire, etc. |
| game_utils/image_sequence.h | struct | ImageSequence | SP_Info | Sequence of image indices with timing; Toggle/ToggleTimed. |
| game_utils/MessageWriter.h | class | EmptyWriter | MessageWriter | Writer that does nothing. |
| game_utils/MessageWriter.h | class | IoWriter | MessageWriter | Writes to an iostream. |
| game_utils/MessageWriter.h | class | MessageWriter | — | Writes messages; subclasses define where. |
| game_utils/mouse_utils.h | struct | MouseCursor | — | Cursor image and position; Draw/Update, click state. |
| game_utils/mouse_utils.h | struct | MouseTracker | — | Tracks mouse position and relative movement. |
| game_utils/mouse_utils.h | struct | PositionTracker | — | Tracks a position (e.g. for camera). |
| game_utils/mouse_utils.h | struct | TrackballTracker | — | Tracks trackball/touch input. |
| game_utils/Preloader.h | struct | ImageFlipper | — | Functor that flips an image horizontally. |
| game_utils/Preloader.h | struct | ImagePainter | — | Functor that copies image and applies color replacements. |
| game_utils/Preloader.h | class | Preloader | SP_Info | Loads and caches images, sequences, sounds by string key. |
| game_utils/Preloader.h | class | PreloaderException | MyException | Base exception for Preloader errors. |
| game_utils/Preloader.h | class | PreloaderExceptionAccess | PreloaderException | Thrown when accessing missing image/seq/sound by key. |
| game_utils/Preloader.h | class | PreloaderExceptionLoad | PreloaderException | Thrown when loading an image/sound file fails. |
| game_utils/sound_sequence.h | struct | SoundSequence | — | Sequence of sound indices with per-sound intervals. |
| game_utils/sound_utils.h | struct | BackgroundMusicPlayer | — | Plays level music from vThemes; SwitchTheme/StopMusic/ToggleOff control playback. |
| game_utils/sound_utils.h | class | SoundInterfaceProxy | SP_Info | Wraps sound interface; gates playback on bSoundOn (Toggle/Get). |
| utils/exception.h | class | MyException | — | Base exception: name, class, function stack; GetErrorMessage. |
| utils/exception.h | class | SimpleException | MyException | Exception with a single problem string. |
| utils/file_utils.cc | class | InMemoryOutStreamHandler | OutStreamHandler | OutStreamHandler backed by stringstream (in-memory). |
| utils/file_utils.h | class | CachingReadOnlyFileManager | FileManager | Caches read content; optional filter for underlying. |
| utils/file_utils.h | class | FileManager | — | Abstract: WriteFile, ReadFile, FileExists. |
| utils/file_utils.h | class | FilePath | — | Path: base, slash style, allowed chars; ReadFile/WriteFile. |
| utils/file_utils.h | class | InMemoryFileManager | FileManager | In-memory path → string; WriteFile/ReadFile via streams. |
| utils/file_utils.h | class | InStreamHandler | — | Owns an istream; exposes GetStream(). |
| utils/file_utils.h | class | OutStreamHandler | — | Owns an ostream; exposes GetStream(). |
| utils/file_utils.h | class | SavableVariable\<T\> | — | Holds value of type T; load/save to file via FilePath. |
| utils/file_utils.h | class | StdFileManager | FileManager | Uses given path for opening files. |
| utils/index.h | class | Index | — | Reference-counted handle to IndexKeeper slot. |
| utils/index.h | struct | IndexRemover | — | Interface: notified when an Index is destroyed. |
| utils/index.h | class | IndexKeeper\<T\> | — | Pool of T indexed by unsigned; GetNewIndex, GetElement, FreeElement. |
| utils/smart_pointer.h | class | SP_Info | — | Mix-in for reference counting; ref count. |
| utils/smart_pointer.h | class | smart_pointer\<T\> | — | Reference-counted smart pointer; uses SP_Info. |
| utils/timer.h | struct | Timer | — | Period-based timer: Tick, Check, UntilTick, NextTick. |
| wrappers/color.h | struct | Color | — | RGBA-style color (B, G, R, transparency). |
| wrappers/font_writer.h | struct | FontWriter | — | Renders text using font, bitmap symbols, GI; GetSize, DrawWord/DrawColorWord. |
| wrappers/geometry.h | struct | fPoint | — | 2D float point; Normalize, Length, ToPnt; to/from Point. |
| wrappers/geometry.h | struct | Point | — | 2D integer point (x, y); +=, -=, *=, +, -, *, ==. |
| wrappers/geometry.h | struct | Polar | — | Polar coordinates (radius, angle); converts to/from fPoint. |
| wrappers/geometry.h | struct | Rectangle | — | Axis-aligned rectangle (Point p, Size sz). |
| wrappers/geometry.h | struct | Size | — | 2D extent (x, y); Area(). |
| wrappers/GuiGen.h | class | CameraControl\<ImageHndl\> | — | View/camera: stack of Scale, translate/zoom, coordinate conversion. |
| wrappers/GuiGen.h | class | GraphicalInterface\<ImageHndl\> | SP_Info | Abstract drawing API: create/load/save images, draw, refresh. |
| wrappers/GuiGen.h | class | GraphicalInterfaceException | MyException | Base exception for graphical-interface errors. |
| wrappers/GuiGen.h | class | GraphicalInterfaceSimpleException | GraphicalInterfaceException | GI exception with single problem string. |
| wrappers/GuiGen.h | class | Image | — | Abstract image: size, SetPixel/GetPixel, transparency, etc. |
| wrappers/GuiGen.h | class | ImageException | MyException | Base exception for image errors. |
| wrappers/GuiGen.h | class | ImageMatrixException | ImageException | Image exception for matrix/out-of-range. |
| wrappers/GuiGen.h | class | ImageNullException | ImageException | Image exception for null pointer. |
| wrappers/GuiGen.h | struct | MatrixErrorInfo | — | Size and Point for matrix/rectangle out-of-range errors. |
| wrappers/GuiGen.h | class | MatrixException | MyException | Exception for matrix/rectangle out-of-range. |
| wrappers/GuiGen.h | class | NullPointerGIException | GraphicalInterfaceException | GI exception for null pointer. |
| wrappers/GuiGen.h | struct | Scale | — | View transform: offset, zoom, box size for coordinates. |
| wrappers/GuiGen.h | struct | SimpleGraphicalInterface\<ImageHndl\> | GraphicalInterface\<Index\>, IndexRemover | GI\<Index\> wrapper; delegates to real GI, IndexKeeper. |
| wrappers/GuiGen.h | struct | TransparencyGrid | — | Transparency grid data. |
| wrappers/GuiGen.h, GuiGen.cc | struct | BmpFileHdr | — | BMP file header (local to GuiLoadImage/save). |
| wrappers/GuiGen.h, GuiGen.cc | struct | BmpInfoHdr | — | BMP info header (local to GuiLoadImage/save). |
| wrappers/SuiGen.h | class | SimpleSoundInterface\<SndHndl\> | SoundInterface\<Index\>, IndexRemover | SoundInterface\<Index\> with Index-based handles. |
| wrappers/SuiGen.h | class | SoundInterface\<SndHndl\> | — | Abstract sound API: LoadSound, DeleteSound, PlaySound, StopSound, SetVolume. |
| wrappers_mock/GuiMock.h | class | MockGraphicalInterface | GraphicalInterface\<std::string\> | Mock GI for tests. |
| wrappers_mock/GuiMock.h | class | MockImage | Image | Mock Image for tests. |
| wrappers_mock/SuiMock.h | class | MockSoundInterface | SoundInterface\<std::string\> | Mock sound interface for tests. |
| wrappers_sdl_impl/GuiSdl.h | class | NullPointerSGIException | SdlGraphicalInterfaceException | SDL GI null pointer exception. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlGraphicalInterface | GraphicalInterface\<SdlImage *\> | SDL implementation of GraphicalInterface. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlGraphicalInterfaceException | GraphicalInterfaceException | SDL GI exception. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlImage | Image | SDL-backed Image implementation. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlImageException | ImageException | SDL-specific image exception. |
| wrappers_sdl_impl/GuiSdl.h | class | SimpleSdlImageException | SdlImageException | SDL image exception with single message. |
| wrappers_sdl_impl/GuiSdl.h | class | SimpleSGIException | SdlGraphicalInterfaceException | SDL GI exception with single message. |
| wrappers_sdl_impl/SuiSdl.h | class | SdlSoundInterface | SoundInterface\<Mix_Chunk *\> | SDL implementation of SoundInterface. |

## Expanded descriptions (complex classes)

- **DragonGameController** — Holds the active screen index (vCnt, nActive), level storage (vLvl, vLevelPointers), menu (pMenu), graphics (pGraph, pDr, pNum, pBigNum, pFancyNum), sound (pSndRaw, pSnd), score (nScore, nHighScore), and savable options (snProgress, sbSoundOn, sbMusicOn, sbTutorialOn, sbFullScreen, sbCheatsOn, sbCheatsUnlocked). Preloader (pr) loads assets by key. StartUp/Next/Restart/Menu drive screen flow; GetImg/GetImgSeq/GetSnd/GetSndSeq delegate to Preloader.

- **LevelController** — Main play screen. Owns castles (vCs), roads (vRd), dragon (vDr), bonus pick-ups (lsBonus), slimes (lsSlimes), sliminess effects (lsSliminess). TrackballTracker (tr) and ButtonSet drive dragon steering. Timers (t, tBlink, tStep, tLoseTimer) handle countdown, blink, step, and lose delay. KnightGenerator (pGr) and MageGenerator (pMgGen) spawn enemies; SoundControls (pSc) and tutorial (tutOne, tutTwo, pTutorialText) are optional. Update runs physics, collisions, and draw; OnKey/OnMouseDown/Fire feed input.

- **EntityListController** — Base for screens that maintain lists of drawables (lsDraw), updatables (lsUpdate), and consumables (lsPpl). Also has owned_entities / owned_visual_entities / owned_event_entities for controller-owned lifetime. AddV/AddE/AddBoth/AddBackground add entities; Update() cleans dead, runs Move/Update on all, then draws by priority. Subclasses (LevelController, MenuController, StartScreenController, Cutscene, DragonScoreController, BuyNowController, AutoAdvanceController) add screen-specific behavior.

- **GameController** — Base for any screen: pGl (DragonGameController*), rBound (Rectangle). Virtuals: Update(), OnKey(), OnMouse(), OnMouseDown(), OnMouseUp(), DoubleClick(), Fire(), GetControllerName(). DragonGameRunner calls these from its input/update loop.

- **Entity hierarchy** — Entity (bExist) → EventEntity (Move, Update) and ScreenEntity (GetPosition). ScreenEntity → VisualEntity (Draw, GetPriority) and PhysicalEntity (position, radius). VisualEntity + EventEntity → SimpleVisualEntity (ImageSequence, timer/step). PhysicalEntity → ConsumableEntity (GetType, OnHit, GetImage). Critter = PhysicalEntity + SimpleVisualEntity (fPos, fVel, Move, radius). Fireball/Castle/Princess/etc. extend Critter and optionally ConsumableEntity.

- **FireballBonus / Chain / ChainExplosion** — FireballBonus holds named maps (fMap, uMap, bMap) and nNum; += merges bonuses. Chain tracks generation count or infinite; Evolve() decrements; IsLast() true when done. ChainExplosion is an expanding AnimationOnce+PhysicalEntity; each frame it grows, hits consumables (except golems/mega slimes), and can spawn child ChainExplosions for chain reactions.

- **Preloader** — Loads images, ImageSequences, and sounds by string key (from FilePath + asset files). Caches results. Used by DragonGameController to serve GetImg/GetImgSeq/GetSnd/GetSndSeq to levels and UI.

- **FilePath** — Immutable path (base, slash style, allowed chars). Created via Create() or CreateFromStream(); holds FileManager* for ReadFile/WriteFile/FileExists. Used by Preloader, FontWriter, and SavableVariable.

- **smart_pointer\<T\>** — Reference-counted pointer; pointee must inherit SP_Info. Copies increment count; destruction decrements and deletes when 0. make_smart(T*) allocates and wraps.

- **Index / IndexKeeper\<T\>** — Index is a reference-counted handle to a slot in an IndexKeeper. When the last Index to a slot is destroyed, IndexRemover::DeleteIndex is called so the backend can free the resource. IndexKeeper is a pool of T (e.g. image/sound handles) with GetNewIndex, GetElement, FreeElement.

- **GraphicalInterface\<ImageHndl\> / CameraControl** — GI is the abstract drawing API (load image, draw, refresh). CameraControl wraps a GI with a stack of Scale (offset, zoom, box); Translate/Zoom modify the current scale; fromF/toF/toR/fromR convert between coordinate systems. Used for view/camera in level rendering.

- **DragonGameRunner** — Top-level GameRunner. Builds TowerDataWrap from ProgramEngine (exit event, GI, sound, MessageWriter, FileManager), creates DragonGameController with levels and options, and runs the game loop. Forwards KeyDown/KeyUp/MouseMove/MouseDown/MouseUp/DoubleClick/Fire to the active controller. GetTowerController/GetActiveControllerIndex/GetControllerCount/GetActiveControllerName support simulation and inspection.

## Notes

- **Forward declarations** (e.g. LevelController, DragonGameController, MenuDisplay, FilePath) are not listed; the table shows the defining location.
- **Templates** are listed with their parameter(s), e.g. GraphicalInterface<ImageHndl>, smart_pointer<T>.
- **SSP** and **ASSP** are not present as classes in the current codebase.
- **Graphic** and **Soundic** are typedefs (e.g. for SimpleGraphicalInterface and SimpleSoundInterface), not classes in this table.
- **LevelStorage** is a typedef (e.g. std::vector<LevelLayout>), not a class.
- **Removed or relocated since last doc:** game/core.h (Polar→geometry.h; Drawer/ScalingDrawer/NumberDrawer→draw_utils.h; SoundInterfaceProxy→sound_utils.h). game/game.h and game/gameplay.h split into game/controller/*.h, game/entities.h, game_utils/mouse_utils.h. BasicController→EntityListController, AdvancedController→LevelController. DragonGameControllerList→DragonGameController. GlobalController→GameRunner. SimpleController, FlashingController, Countdown, AlmostBasicController, EmptyEvent, WriteEvent, MakeSoundEvent, FileWriter removed. wrappers/GuiGenHdr.h split into GuiGen.h, geometry.h, color.h, font_writer.h.
<｜tool▁calls▁begin｜><｜tool▁call▁begin｜>
StrReplace