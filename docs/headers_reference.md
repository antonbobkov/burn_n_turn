# Header reference: structs, classes, and global functions

| File | Structs / classes | Global functions |
|------|-------------------|------------------|
| game/controller/basic_controllers.h | SimpleController, FlashingController, EntityListController, StartScreenController, Cutscene, DragonScoreController, AutoAdvanceController | — |
| game/controller/buy_now_controller.h | BuyNowController, SlimeUpdater | — |
| game/controller/dragon_game_controller.h | DragonGameController | — |
| game/controller/game_controller_interface.h | GameController | — |
| game/controller/level_controller.h | LevelController | — |
| game/controller/menu_controller.h | MenuEntry, MenuEntryManager, MenuDisplay, MenuController | OnOffString, SoundString, MusicString, TutorialString, FullTextString |
| game/common.h | — | nSlimeMax, pWr (extern) |
| game/core.h | TowerDataWrap, MenuController, DragonGameController, GameController | sFullScreenPath (extern) |
| game/critter_generators.h | SkellyGenerator, KnightGenerator, PrincessGenerator, MageGenerator, TraderGenerator | — |
| game/critters.h | Castle, Princess, Mage, Trader, Knight, MegaSlime, Ghostiness, Slime, Sliminess, MegaSliminess, FloatingSlime | RandomBonus, GetBonusImage, SummonSkeletons, GetTimeUntillSpell |
| game/dragon_constants.h | — | — |
| game/dragon.h | DragonLeash, ButtonSet, Dragon | — |
| game/dragon_game_runner.h | TowerDataWrap, DragonGameRunner | — |
| game/dragon_macros.h | — | — |
| game/entities.h | Entity, EventEntity, ScreenEntity, VisualEntity, TextDrawEntity, SimpleVisualEntity, SimpleSoundEntity, Animation, AnimationOnce, StaticImage, StaticRectangle, PhysicalEntity, ConsumableEntity, Critter, FancyCritter, ScreenPos, BonusScore, SoundControls, HighScoreShower, IntroTextShower | CleanUp |
| game/fireball.h | FireballBonus, Chain, ChainExplosion, KnightOnFire, Fireball, TimedFireballBonus, CircularFireball, FireballBonusAnimation | operator<<(FireballBonus), GetFireballRaduis, GetSizeSuffix, GetExplosionInitialRaduis, GetExplosionExpansionRate, GetFireballChainNum |
| game/game.h | — | — |
| game/gameplay.h | — | — |
| game/level.h | SegmentSimpleException, BrokenLine, Road, FancyRoad, LevelLayout | operator<<(BrokenLine), operator>>(BrokenLine), operator<<(Road), operator>>(Road), ReadLevels, operator<<(LevelLayout), operator>>(LevelLayout) |
| game/tutorial.h | TutorialTextEntity, TutorialLevelOne, TutorialLevelTwo | — |
| game_utils/draw_utils.h | Drawer, ScalingDrawer, NumberDrawer | — |
| game_utils/event.h | Event, EmptyEvent, WriteEvent, TerminatorEvent, SwitchEvent, CpSwitchEvent, SequenceOfEvents, MakeSoundEvent | Trigger, NewTerminatorEvent, NewSwitchEvent, NewCpSwitchEvent, TwoEvents |
| game_utils/game_runner_interface.h | FileManager, ProgramInfo, ProgramEngine, GameRunner | GetProgramInfo, GetGameRunner |
| game_utils/image_sequence.h | ImageSequence | Reset, ForEachImage |
| game_utils/MessageWriter.h | MessageWriter, EmptyWriter, IoWriter, FileWriter | WriteType (enum) |
| game_utils/mouse_utils.h | MouseTracker, TrackballTracker, PositionTracker, MouseCursor | — |
| game_utils/Preloader.h | ImageFlipper, ImagePainter, PreloaderException, PreloaderExceptionAccess, PreloaderExceptionLoad, Preloader | — |
| game_utils/sound_sequence.h | SoundSequence | — |
| game_utils/sound_utils.h | SoundInterfaceProxy, BackgroundMusicPlayer | BG_MUSIC_CHANNEL, BG_BACKGROUND, etc. (enums) |
| simulation/simulation.h | — | RunSimulation |
| utils/debug_log.h | — | EnableDebugLog, DebugLog |
| utils/exception.h | MyException, SimpleException | crefString (typedef) |
| utils/file_utils.h | OutStreamHandler, InStreamHandler, FileManager, StdFileManager, InMemoryFileManager, CachingReadOnlyFileManager, FilePath, SavableVariable | ParsePosition, ParseGrabNext, ParseGrabLine, GetFileContent, WriteContentToFile, operator<<(FilePath), BoolToggle, Separate |
| utils/index.h | IndexRemover, Index, IndexKeeper | — |
| utils/math_utils.h | — | Gmin, Gmax, Gabs |
| utils/random_utils.h | — | GetRandTimeFromRate, GetRandNum, GetRandFromDistribution |
| utils/smart_pointer.h | SP_Info, smart_pointer | nGlobalSuperMegaCounter, g_smart_pointer_count (extern), CHECK_DELETION, DELETE_REGULAR_POINTER, make_smart |
| utils/string_utils.h | — | S, BreakUpString |
| utils/timer.h | Timer | — |
| wrappers/color.h | Color | operator==(Color), operator<(Color), ColorToString |
| wrappers/font_writer.h | FontWriter | — |
| wrappers/geometry.h | Point, fPoint, Polar, Size, Rectangle | Point operators, fPoint operators, Dot, ComposeDirection, GetWedgeAngle, RandomAngle, DiscreetAngle, Size operators, Rectangle operators, InsideRectangle, Center, RandomPnt, Intersect, RectangleToString |
| wrappers/GuiGen.h | MatrixErrorInfo, MatrixException, TransparencyGrid, ImageException, ImageMatrixException, ImageNullException, GraphicalInterfaceException, GraphicalInterfaceSimpleException, NullPointerGIException, Image, GraphicalInterface, SimpleGraphicalInterface, Scale, CameraControl | PointerAssert, AdjustImageOverlap, GuiLoadImage, GuiSaveImage, GetBmpPos, DrawColorOnto, ExplDrawColorOnto, ConvertImage |
| wrappers/gui_key_type.h | — | GuiKeyType (enum) |
| wrappers_sdl_impl/GuiSdl.h | SdlImageException, SimpleSdlImageException, SdlImage, SdlGraphicalInterfaceException, SimpleSGIException, NullPointerSGIException, SdlGraphicalInterface | — |
| wrappers_mock/GuiMock.h | MockImage, MockGraphicalInterface | — |
| wrappers/SuiGen.h | SoundInterface, SimpleSoundInterface, DummySoundInterface | — |
| wrappers_sdl_impl/SuiSdl.h | SdlSoundInterface | — |
| wrappers_mock/SuiMock.h | MockSoundInterface | — |
