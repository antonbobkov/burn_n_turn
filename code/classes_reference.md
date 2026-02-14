# Classes and Structs Reference

Table of all classes and structs: code location, kind (struct/class),
name, base types, and short description.

| Location | Kind | Name | Inherits from | Description |
|----------|------|------|---------------|-------------|
| game/core.h | struct | Polar | — | Polar coordinates (radius, angle); converts to/from fPoint. |
| game/core.h | struct | Drawer | SP_Info | Base for drawing an image at a point. |
| game/core.h | struct | ScalingDrawer | Drawer | Drawer with scale factor and transparency color key. |
| game/core.h | struct | NumberDrawer | SP_Info | Draws digits/words from a font bitmap; supports recolored text. |
| game/core.h | class | SoundInterfaceProxy | SP_Info | Wraps sound interface; gates playback on bSoundOn (Toggle/Get). |
| game/critter_generators.h | struct | SkellyGenerator | EventEntity | Spawns skeleton critters. |
| game/critter_generators.h | struct | KnightGenerator | EventEntity | Spawns knight critters. |
| game/critter_generators.h | struct | PrincessGenerator | EventEntity | Spawns princess critters. |
| game/critter_generators.h | struct | MageGenerator | EventEntity | Spawns mage critters. |
| game/critter_generators.h | struct | TraderGenerator | EventEntity | Spawns trader critters. |
| game/critters.h | struct | Castle | Critter | Castle building; holds princesses, dragon reference. |
| game/critters.h | struct | Princess | Critter, ConsumableEntity | Princess unit; captured by dragon. |
| game/critters.h | struct | Mage | Critter, ConsumableEntity | Mage unit; can summon slimes. |
| game/critters.h | struct | Trader | Critter, ConsumableEntity | Trader unit; drops bonus. |
| game/critters.h | struct | Knight | Critter, ConsumableEntity | Knight unit; chases princess/castle, can become ghost. |
| game/critters.h | struct | MegaSlime | Critter, ConsumableEntity | Large slime enemy. |
| game/critters.h | struct | Ghostiness | EventEntity | Ghost state/effect for knight. |
| game/critters.h | struct | Slime | Critter, ConsumableEntity | Slime enemy. |
| game/critters.h | struct | Sliminess | EventEntity | Slime visual/state effect. |
| game/critters.h | struct | MegaSliminess | EventEntity | Mega slime visual/state effect. |
| game/critters.h | struct | FloatingSlime | SimpleVisualEntity | Non-physical slime visual. |
| game/dragon.h | struct | DragonLeash | — | Dragon steering: velocity, tilt, speed, trackball params. |
| game/dragon.h | struct | ButtonSet | — | Set of key/button codes for input (fire, steer, etc.). |
| game/dragon.h | struct | Dragon | Critter | Player dragon; bonuses, fireballs, steer/shoot, collision. |
| game/entities.h | struct | Entity | SP_Info | Root entity; bExist flag, virtual dtor. |
| game/entities.h | struct | EventEntity | Entity | Entity that can Move and Update each frame. |
| game/entities.h | struct | ScreenEntity | Entity | Entity with a screen position (GetPosition). |
| game/entities.h | struct | VisualEntity | ScreenEntity | Entity that can Draw via ScalingDrawer; has priority. |
| game/entities.h | struct | TextDrawEntity | VisualEntity | Draws multi-line text via NumberDrawer at a position. |
| game/entities.h | struct | SimpleVisualEntity | EventEntity, VisualEntity | VisualEntity with ImageSequence; timer or step animation. |
| game/entities.h | struct | SimpleSoundEntity | EventEntity | Plays SoundSequence on a timer; sets bExist false when done. |
| game/entities.h | struct | Animation | SimpleVisualEntity | Looping animation entity. |
| game/entities.h | struct | AnimationOnce | SimpleVisualEntity | One-shot animation; sets bExist false when finished. |
| game/entities.h | struct | StaticImage | VisualEntity | Draws a single image at a position. |
| game/entities.h | struct | StaticRectangle | VisualEntity | Draws a filled rectangle. |
| game/entities.h | struct | PhysicalEntity | ScreenEntity | Entity with position and radius for collision. |
| game/entities.h | struct | ConsumableEntity | PhysicalEntity | Can be collected/hit (e.g. by fireball or dragon). |
| game/entities.h | struct | Critter | PhysicalEntity, SimpleVisualEntity | Moving game unit (enemy or player); position, velocity, type. |
| game/entities.h | struct | FancyCritter | PhysicalEntity, SimpleVisualEntity | Critter with extra visual/behavior. |
| game/entities.h | struct | ScreenPos | — | Screen position helper. |
| game/fireball.h | struct | FireballBonus | Entity | Power-up state: named float/unsigned/bool maps; += merges. |
| game/fireball.h | struct | Chain | — | Chain reaction generation count or infinite; Evolve/IsLast. |
| game/fireball.h | struct | ChainExplosion | AnimationOnce, PhysicalEntity | Expanding explosion; hits consumables, spawns chain. |
| game/fireball.h | struct | KnightOnFire | Critter | Knight on fire state; random velocity, timer. |
| game/fireball.h | struct | Fireball | Critter | Player fireball; bThrough, hits ConsumableEntities. |
| game/fireball.h | struct | TimedFireballBonus | FireballBonus, EventEntity | FireballBonus that updates on a timer. |
| game/fireball.h | struct | CircularFireball | Fireball, TimedFireballBonus | Fireball that orbits at a radius. |
| game/fireball.h | struct | FireballBonusAnimation | Animation, PhysicalEntity | Pick-up animation with radius; overlaps trigger collection. |
| game/game.h | struct | GameController | SP_Info | Base controller: pGl, rBound; virtual input/Update. |
| game/game.h | struct | DragonGameControllerList | SP_Info | Global game state: levels, controller, graphics/sound, score, options. |
| game/game.h | struct | FancyRoad | Road | Road that draws tiled image; uses AdvancedController. |
| game/game.h | class | TowerDataWrap | — | Holds exit event and graphics/sound interfaces for setup. |
| game/game.h | class | DragonGameRunner | GlobalController | Top-level tower game; owns TowerDataWrap, runs level flow. |
| game/gameplay.h | struct | TrackballTracker | — | Tracks trackball/touch input. |
| game/gameplay.h | struct | BonusScore | EventEntity, VisualEntity | Displays bonus score popup. |
| game/gameplay.h | struct | SoundControls | EventEntity | Sound on/off control entity. |
| game/gameplay.h | struct | PositionTracker | — | Tracks a position (e.g. for camera). |
| game/gameplay.h | struct | AdvancedController | BasicController | Main level controller; dragon, fireballs, consumables, UI. |
| game/gameplay.h | struct | HighScoreShower | VisualEntity | Shows high score on screen. |
| game/gameplay.h | struct | IntroTextShower | VisualEntity | Shows intro text. |
| game/gameplay.cpp | struct | AdNumberDrawer | VisualEntity | Draws numbers (e.g. ads) on screen. |
| game/gameplay.cpp | struct | BonusDrawer | VisualEntity | Draws bonus visuals. |
| game/level.h | class | SegmentSimpleException | MyException | Exception for BrokenLine segment errors. |
| game/level.h | struct | BrokenLine | — | Polyline(s); vEdges, CloseLast, Add, AddLine, Join, I/O. |
| game/level.h | struct | Road | VisualEntity | Level road segment; vertical/horizontal, bounds; Draw gray bar. |
| game/level.h | struct | LevelLayout | — | One level: bounds, spawn line, castle positions, roads, timer. |
| game/screen_controllers.h | struct | SimpleController | GameController | Draws single full-screen image; advances on key. |
| game/screen_controllers.h | struct | FlashingController | GameController | Background + text that flashes every second. |
| game/screen_controllers.h | struct | MenuEntry | SP_Info | One menu item: size, label, callback, disabled flag. |
| game/screen_controllers.h | struct | MenuEntryManager | — | Holds menu entries and current selection index. |
| game/screen_controllers.h | struct | MenuDisplay | EventEntity, VisualEntity | In-game menu: entries, caret, mouse/key, submenus. |
| game/screen_controllers.h | struct | Countdown | VisualEntity, EventEntity | Draws countdown number; sets bExist false at 0. |
| game/screen_controllers.h | struct | BasicController | GameController | Draw/update/consumable lists; Update runs Move, Update, draw. |
| game/screen_controllers.h | struct | MouseCursor | — | Cursor image and position; Draw/Update, click state. |
| game/screen_controllers.h | struct | MenuController | BasicController | Pause/main menu; MenuDisplay, resume position. |
| game/screen_controllers.h | struct | StartScreenController | BasicController | Start screen; advance on key/click. |
| game/screen_controllers.h | struct | SlimeUpdater | VisualEntity | Updates slime animation for buy-now screen. |
| game/screen_controllers.h | struct | BuyNowController | BasicController | Buy-now screen: slime animations and timer. |
| game/screen_controllers.h | struct | Cutscene | BasicController | Cutscene controller. |
| game/screen_controllers.h | struct | DragonScoreController | BasicController | Dragon score screen controller. |
| game/screen_controllers.h | struct | AlmostBasicController | BasicController | Controller almost like BasicController (variant). |
| game/tutorial.h | struct | TutorialTextEntity | EventEntity, VisualEntity | Tutorial text on screen. |
| game/tutorial.h | struct | TutorialLevelOne | — | First tutorial level data/behavior. |
| game/tutorial.h | struct | TutorialLevelTwo | — | Second tutorial level data/behavior. |
| game_utils/event.h | class | Event | SP_Info | Abstract event; Trigger() invoked by Trigger(). |
| game_utils/event.h | class | EmptyEvent | Event | Event that does nothing on Trigger. |
| game_utils/event.h | class | WriteEvent | Event | Event that writes text to cout on Trigger. |
| game_utils/event.h | class | TerminatorEvent\<T\> | Event | Event that calls pObject->Terminate() on Trigger. |
| game_utils/event.h | class | SwitchEvent\<A,B\> | Event | Event that assigns objCurr = objNew on Trigger. |
| game_utils/event.h | class | CpSwitchEvent\<A,B\> | Event | Event that assigns objCurr = copy of objNew on Trigger. |
| game_utils/event.h | struct | SequenceOfEvents | Event | Runs a sequence of events on Trigger. |
| game_utils/event.h | class | MakeSoundEvent | Event | Event that plays a sound on Trigger. |
| game_utils/game_runner_interface.h | struct | ProgramInfo | — | Program config: resolution, title, framerate, flags. |
| game_utils/game_runner_interface.h | struct | ProgramEngine | — | Engine wiring: exit event, GI, sound, message writer, FileManager. |
| game_utils/game_runner_interface.h | class | GlobalController | SP_Info | Top-level input/update: Update, KeyDown, MouseMove, etc. |
| game_utils/MessageWriter.h | class | MessageWriter | SP_Info | Writes messages; subclasses define where. |
| game_utils/MessageWriter.h | class | EmptyWriter | MessageWriter | Writer that does nothing. |
| game_utils/MessageWriter.h | class | IoWriter | MessageWriter | Writes to an iostream. |
| game_utils/MessageWriter.h | class | FileWriter | MessageWriter | Writes to a file. |
| game_utils/Preloader.h | struct | SoundSequence | — | Sequence of sound indices with per-sound intervals. |
| game_utils/Preloader.h | struct | ImageSequence | SP_Info | Sequence of image indices with timing; Toggle/ToggleTimed. |
| game_utils/Preloader.h | struct | ImageFlipper | — | Functor that flips an image horizontally. |
| game_utils/Preloader.h | struct | ImagePainter | — | Functor that copies image and applies color replacements. |
| game_utils/Preloader.h | class | PreloaderException | MyException | Base exception for Preloader errors. |
| game_utils/Preloader.h | class | PreloaderExceptionAccess | PreloaderException | Thrown when accessing missing image/seq/sound by key. |
| game_utils/Preloader.h | class | PreloaderExceptionLoad | PreloaderException | Thrown when loading an image/sound file fails. |
| game_utils/Preloader.h | class | Preloader | SP_Info | Loads and caches images, sequences, sounds by string key. |
| utils/exception.h | class | MyException | — | Base exception: name, class, function stack; GetErrorMessage. |
| utils/exception.h | class | SimpleException | MyException | Exception with a single problem string. |
| utils/file_utils.h | class | OutStreamHandler | — | Owns an ostream; exposes GetStream(). |
| utils/file_utils.h | class | InStreamHandler | — | Owns an istream; exposes GetStream(). |
| utils/file_utils.h | class | FileManager | — | Abstract: WriteFile, ReadFile, FileExists. |
| utils/file_utils.h | class | StdFileManager | FileManager | Uses given path for opening files. |
| utils/file_utils.h | class | InMemoryFileManager | FileManager | In-memory path → string; WriteFile/ReadFile via streams. |
| utils/file_utils.h | class | CachingReadOnlyFileManager | FileManager | Caches read content; optional filter for underlying. |
| utils/file_utils.h | class | FilePath | — | Path: base, slash style, allowed chars; ReadFile/WriteFile. |
| utils/file_utils.h | class | SavableVariable\<T\> | — | Holds value of type T; load/save to file via FilePath. |
| utils/file_utils.cpp | class | InMemoryOutStreamHandler | OutStreamHandler | OutStreamHandler backed by stringstream (in-memory). |
| utils/index.h | struct | IndexRemover | — | Interface: notified when an Index is destroyed. |
| utils/index.h | class | Index | — | Reference-counted handle to IndexKeeper slot. |
| utils/index.h | class | IndexKeeper\<T\> | — | Pool of T indexed by unsigned; GetNewIndex, GetElement, FreeElement. |
| utils/smart_pointer.h | class | SP_Info | — | Mix-in for reference counting; ref count, SSP sets. |
| utils/smart_pointer.h | class | SSP_Base | — | Tracks non-owning “permanent” pointers (SSP/ASSP). |
| utils/smart_pointer.h | class | smart_pointer\<T\> | — | Reference-counted smart pointer; uses SP_Info. |
| utils/smart_pointer.h | struct | Inspector | — | Cycle/island cleanup for smart pointers. |
| utils/timer.h | struct | Timer | — | Period-based timer: Tick, Check, UntilTick, NextTick. |
| wrappers/GuiGenHdr.h | struct | Color | — | RGBA-style color (B, G, R, transparency). |
| wrappers/GuiGenHdr.h | struct | Point | — | 2D integer point (x, y); +=, -=, *=, +, -, *, ==. |
| wrappers/GuiGenHdr.h | struct | fPoint | — | 2D float point; Normalize, Length, to/from Point. |
| wrappers/GuiGenHdr.h | struct | Size | — | 2D extent (x, y); Area(). |
| wrappers/GuiGenHdr.h | struct | Rectangle | — | Axis-aligned rectangle (Point p, Size sz). |
| wrappers/GuiGenHdr.h | struct | MatrixErrorInfo | — | Size and Point for matrix/rectangle out-of-range errors. |
| wrappers/GuiGenHdr.h | class | MatrixException | MyException | Exception for matrix/rectangle out-of-range. |
| wrappers/GuiGenHdr.h | struct | TransparencyGrid | — | Transparency grid data. |
| wrappers/GuiGenHdr.h | class | ImageException | MyException | Base exception for image errors. |
| wrappers/GuiGenHdr.h | class | ImageMatrixException | ImageException | Image exception for matrix/out-of-range. |
| wrappers/GuiGenHdr.h | class | ImageNullException | ImageException | Image exception for null pointer. |
| wrappers/GuiGenHdr.h | class | Image | — | Abstract image: size, SetPixel/GetPixel, transparency, etc. |
| wrappers/GuiGenHdr.h | class | GraphicalInterfaceException | MyException | Base exception for graphical-interface errors. |
| wrappers/GuiGenHdr.h | class | GraphicalInterfaceSimpleException | GraphicalInterfaceException | GI exception with single problem string. |
| wrappers/GuiGenHdr.h | class | NullPointerGIException | GraphicalInterfaceException | GI exception for null pointer. |
| wrappers/GuiGenHdr.h | class | GraphicalInterface\<ImageHndl\> | SP_Info | Abstract drawing API: create/load/save images, draw, refresh. |
| wrappers/GuiGenHdr.h | struct | SimpleGraphicalInterface\<ImageHndl\> | GraphicalInterface\<Index\>, IndexRemover | GI\<Index\> wrapper; delegates to real GI, IndexKeeper. |
| wrappers/GuiGenHdr.h | struct | Scale | — | View transform: offset, zoom, box size for coordinates. |
| wrappers/GuiGenHdr.h | class | CameraControl\<ImageHndl\> | — | View/camera: stack of Scale, translate/zoom, coordinate conversion. |
| wrappers/GuiGenHdr.h | struct | FontWriter | SP_Info | Renders text using font, bitmap symbols, GI; GetSize, DrawWord. |
| wrappers/GuiGenHdr.h | struct | MouseTracker | — | Tracks mouse position and relative movement. |
| wrappers/GuiGen.h, GuiGen.cpp | struct | BmpFileHdr | — | BMP file header (local to GuiLoadImage/save). |
| wrappers/GuiGen.h, GuiGen.cpp | struct | BmpInfoHdr | — | BMP info header (local to GuiLoadImage/save). |
| wrappers/SuiGen.h | class | SoundInterface\<SndHndl\> | SP_Info | Abstract sound API: LoadSound, PlaySound, StopSound, etc. |
| wrappers/SuiGen.h | class | SimpleSoundInterface\<SndHndl\> | SoundInterface\<Index\>, IndexRemover | SoundInterface\<Index\> with Index-based handles. |
| wrappers_mock/GuiMock.h | class | MockImage | Image | Mock Image for tests. |
| wrappers_mock/GuiMock.h | class | MockGraphicalInterface | GraphicalInterface\<std::string\> | Mock GI for tests. |
| wrappers_mock/SuiMock.h | class | MockSoundInterface | SoundInterface\<std::string\> | Mock sound interface for tests. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlImageException | ImageException | SDL-specific image exception. |
| wrappers_sdl_impl/GuiSdl.h | class | SimpleSdlImageException | SdlImageException | SDL image exception with single message. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlImage | Image | SDL-backed Image implementation. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlGraphicalInterfaceException | GraphicalInterfaceException | SDL GI exception. |
| wrappers_sdl_impl/GuiSdl.h | class | SimpleSGIException | SdlGraphicalInterfaceException | SDL GI exception with single message. |
| wrappers_sdl_impl/GuiSdl.h | class | NullPointerSGIException | SdlGraphicalInterfaceException | SDL GI null pointer exception. |
| wrappers_sdl_impl/GuiSdl.h | class | SdlGraphicalInterface | GraphicalInterface\<SdlImage *\> | SDL implementation of GraphicalInterface. |
| wrappers_sdl_impl/SuiSdl.h | class | SdlSoundInterface | SoundInterface\<Mix_Chunk *\> | SDL implementation of SoundInterface. |

## Notes

- **Forward declarations** (e.g. `AdvancedController`, `BasicController`, `MenuDisplay`, `FilePath`) are not listed; the table shows the defining location.
- **Templates** are listed with their parameter(s), e.g. `GraphicalInterface<ImageHndl>`, `smart_pointer<T>`.
- **SSP** and **ASSP** are typedefs/helpers for the smart-pointer system, not separate classes.
- **Graphic** and **Soundic** are typedefs (e.g. for SimpleGraphicalInterface and SimpleSoundInterface), not classes in this table.
- **LevelStorage** is a typedef (std::vector\<LevelLayout\>), not a class.
