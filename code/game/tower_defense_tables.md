# tower_defense.h — Classes/Structs and Global Functions

## Table 1: Classes and Structs

| Name | Parent(s) | Description |
|------|-----------|-------------|
| **Polar** | — | Polar coordinates (radius r, angle a); converts to/from fPoint, supports multiply for rotation. |
| **Drawer** | SP_Info | Base for drawing an image at a point; ScalingDrawer adds scale and color key. |
| **ScalingDrawer** | Drawer | Drawer that scales images by nFactor and uses cTr as transparency key. |
| **GameController** | SP_Info | Base controller: holds pGl, rBound; virtual input/Update (OnKey, OnMouse, Fire, etc.). |
| **NumberDrawer** | SP_Info | Draws digits/words from a font bitmap; CacheColor/DrawColorWord for recolored text. |
| **BackgroundMusicPlayer** | — | Plays level music from vThemes; SwitchTheme/StopMusic/ToggleOff control playback. |
| **SavableVariable\<T\>** | — | Persists a value in a file; Get/Set, optional load on construction and save on set. (template) |
| **SoundInterfaceProxy** | SP_Info | Wraps Soundic and gates playback on bSoundOn (Toggle/Get). |
| **TwrGlobalController** | SP_Info | Global game state: level storage, active controller, graphics/sound, score, savable options, music. |
| **SimpleController** | GameController | Controller that draws a single full-screen image and advances on key. |
| **FlashingController** | GameController | Controller that draws background + text that flashes every second. |
| **Entity** | SP_Info | Root entity; bExist flag, virtual dtor. |
| **EventEntity** | Entity | Entity that can Move and Update each frame. |
| **ScreenEntity** | Entity | Entity with a screen position (GetPosition). |
| **VisualEntity** | ScreenEntity | Base for drawable entities; Draw, GetPriority. |
| **TextDrawEntity** | VisualEntity | VisualEntity that draws multi-line text via NumberDrawer at a position. |
| **TutorialTextEntity** | EventEntity, VisualEntity | Scrolling tutorial text; SetText queues sNewText and Update scrolls between sText and sNewText. |
| **TutorialLevelOne** | — | First tutorial: tracks knight kill, flying, princess spawn/capture; GetText/Update drive TutorialTextEntity. |
| **TutorialLevelTwo** | — | Second tutorial: trader spawn/kill and bonus pickup; GetText/Update drive TutorialTextEntity. |
| **SimpleVisualEntity** | EventEntity, VisualEntity | VisualEntity with ImageSequence: draws current frame, Update toggles by timer or on position change. |
| **SimpleSoundEntity** | EventEntity | EventEntity that plays a SoundSequence on a timer; sets bExist false when sequence ends. |
| **Animation** | SimpleVisualEntity | SimpleVisualEntity with fixed position (no movement). |
| **AnimationOnce** | SimpleVisualEntity | Animation that runs once then sets bExist false (seq plays to end). |
| **StaticImage** | VisualEntity | VisualEntity that draws a single image at a fixed point. |
| **StaticRectangle** | VisualEntity | VisualEntity that draws a filled rectangle (no position). |
| **MenuEntry** | SP_Info | One menu item: size, label, callback (EvntPntr), disabled flag. |
| **MenuEntryManager** | — | Holds menu entries and current selection index (nMenuPosition). |
| **MenuDisplay** | EventEntity, VisualEntity | In-game menu: draws entries, caret, handles mouse/key; submenus and option toggles. |
| **Countdown** | VisualEntity, EventEntity | VisualEntity that draws a countdown number and sets bExist false when it reaches 0. |
| **PhysicalEntity** | ScreenEntity | ScreenEntity with radius for hit detection (HitDetection). |
| **TrackballTracker** | — | Tracks mouse for trackball-style steering (angle and fire). |
| **ConsumableEntity** | PhysicalEntity | PhysicalEntity that can be hit (OnHit), has type (GetType) and image (GetImage). |
| **Critter** | PhysicalEntity, SimpleVisualEntity | Moving unit: position, velocity, bounds, radius; Move() steps and clamps or kills on exit. |
| **FancyCritter** | PhysicalEntity, SimpleVisualEntity | Critter that advances position and toggles frame on a timer (tm). |
| **ScreenPos** | — | Sort key for draw order: priority and height (operator&lt;). |
| **BasicController** | GameController | GameController with draw/update/consumable lists; Update runs Move, Update, then draws by priority. |
| **MouseCursor** | — | Cursor image and position; Draw/Update for rendering and click state. |
| **MenuController** | BasicController | Controller for pause/main menu: MenuDisplay, resume position. |
| **StartScreenController** | BasicController | BasicController that advances (Next) on key/click and plays start_game sound. |
| **SlimeUpdater** | VisualEntity | VisualEntity that draws/updates slimes for BuyNowController. |
| **BuyNowController** | BasicController | Controller for buy-now screen: slime animations and timer. |
| **Cutscene** | BasicController | Controller for chase cutscene: black screen, one character running and one waiting, then chaser released. |
| **FireballBonus** | Entity | Power-up state: named float/unsigned/bool maps, nNum; += merges, Add accumulates. |
| **Chain** | — | Chain reaction generation count or infinite; Evolve decrements, IsLast when 0. |
| **ChainExplosion** | AnimationOnce, PhysicalEntity | Expanding explosion; hits ConsumableEntities and spawns child ChainExplosions via ch. |
| **KnightOnFire** | Critter | Knight set on fire (chain effect); timer and chain state, RandomizeVelocity, Update. |
| **BonusScore** | EventEntity, VisualEntity | Floating "+N" score text at a point; animates then removes. |
| **SoundControls** | EventEntity | EventEntity that calls SwitchTheme(nTheme) or StopMusic() when nTheme == -1 on Update. |
| **PositionTracker** | — | Tracks mouse position, last down, press state and counter for input. |
| **AdvancedController** | BasicController | Main game controller: castles, roads, dragon, generators, bonuses, input (trackball/key). |
| **HighScoreShower** | VisualEntity | Draws high score in a rectangle. |
| **IntroTextShower** | VisualEntity | Draws intro text in a rectangle. |
| **DragonScoreController** | BasicController | Controller that shows dragon score and exits on click or timer. |
| **Fireball** | Critter | Player fireball Critter; bThrough for passthrough, hits ConsumableEntities. |
| **TimedFireballBonus** | FireballBonus, EventEntity | FireballBonus that updates on a timer (e.g. temporary power-up). |
| **CircularFireball** | Fireball, TimedFireballBonus | Fireball that orbits at fRadius (circular motion). |
| **Castle** | Critter | Castle (tower) unit: holds princess count, dragon ref, draw/OnKnight. |
| **Road** | VisualEntity | Level road segment: vertical/horizontal, coordinate, bounds; Draw renders gray bar. |
| **FancyRoad** | Road | Road with tiled image; Draw renders tiled road along segment. |
| **Princess** | Critter, ConsumableEntity | Princess unit: Critter + ConsumableEntity, captured by dragon. |
| **SkellyGenerator** | EventEntity | Spawns skeleton knights on a timer at a position. |
| **Mage** | Critter, ConsumableEntity | Mage unit: can summon slimes, OnHit, GetType 'M'. |
| **FireballBonusAnimation** | Animation, PhysicalEntity | Pick-up animation with radius; overlaps ConsumableEntity trigger collection. |
| **Trader** | Critter, ConsumableEntity | Trader unit: drops bonus, bFirstBns ref for first-bonus logic. |
| **Knight** | Critter, ConsumableEntity | Knight unit: chases princess/castle, can become ghost (Ghostiness). |
| **MegaSlime** | Critter, ConsumableEntity | Large slime unit: splits or merges (MegaSlime logic). |
| **Ghostiness** | EventEntity | Ghost knight effect: timed visual at a position. |
| **Slime** | Critter, ConsumableEntity | Slime unit: moves toward target, timer for behavior. |
| **Sliminess** | EventEntity | Spawns slimes on a timer at a position. |
| **MegaSliminess** | EventEntity | Spawns MegaSlimes; holds position and controller. |
| **FloatingSlime** | SimpleVisualEntity | SimpleVisualEntity that moves with fPos/fVel (e.g. menu slime). |
| **SegmentSimpleException** | MyException | Exception for BrokenLine segment errors (e.g. invalid/empty segment). |
| **BrokenLine** | — | Polyline(s): vEdges is list of point sequences; CloseLast, Add, AddLine, Join, stream I/O. |
| **LevelLayout** | — | One level: bounds, knight spawn line, castle positions, roads, timer, spawn freqs; Convert scales coords. |
| **KnightGenerator** | EventEntity | Spawns knights along a path on a timer. |
| **PrincessGenerator** | EventEntity | Spawns princesses at a rate within bounds. |
| **MageGenerator** | EventEntity | Spawns mages at a rate (angry rate when bAngry). |
| **TraderGenerator** | EventEntity | Spawns traders at a rate within bounds. |
| **DragonLeash** | — | Steering state for dragon: tilt, speed, trackball scaling; ModifyTilt, GetNewVelocity. |
| **ButtonSet** | — | Set of key/button codes for input (e.g. fire, steer); GetPoint converts code to 8-direction offset. |
| **Dragon** | Critter | Player dragon: carries bonuses and fireballs, steer/shoot, collision with units. |
| **AlmostBasicController** | BasicController | BasicController that advances (Next) when only background is left or on input. |
| **TowerDataWrap** | — | Holds exit event and graphics/sound interfaces for tower game setup. |
| **TowerGameGlobalController** | GlobalController | Top-level tower game: owns TowerDataWrap, builds and runs level flow. |

---

## Table 2: Global (non-member) functions

| Name | Description |
|------|-------------|
| **ComposeDirection** | Turns two direction keys (e.g. left, up) into a single diagonal direction; used for steering or aiming. |
| **GetWedgeAngle** | Picks one direction within a spread (for multi-shot fire). |
| **RandomAngle** | Random direction near fDir within fRange * 2π. |
| **BreakUpString** | Split string on newlines into a vector of lines (appends \n to s). |
| **OnOffString** | Return "on" or "off" for menu toggles. |
| **SoundString** | Return "sound: ". |
| **MusicString** | Return "music: ". |
| **TutorialString** | Return "tutorial: ". |
| **FullTextString** | Return "full screen: ". |
| **BoolToggle** | Flip the boolean in a SavableVariable and persist it. |
| **operator<<(FireballBonus)** | Write FireballBonus to ostream. |
| **Reset (ImageSequence)** | Reset ImageSequence (set nActive = 0). |
| **DiscreetAngle** | Map angle a to one of nDiv discrete directions. |
| **GetFireballRaduis** | Get fireball radius from FireballBonus. |
| **GetSizeSuffix** | Get size suffix string from FireballBonus. |
| **GetExplosionInitialRaduis** | Get explosion initial radius from FireballBonus. |
| **GetExplosionExpansionRate** | Get explosion expansion rate from FireballBonus. |
| **Center (Size)** | Return center point of a Size (Point(sz.x/2, sz.y/2)). |
| **GetRandTimeFromRate** | Return random time (frames) from rate (1 + random in [0.25,1.75]*fRate). |
| **SummonSkeletons** | Summon skeleton knights at a position for AdvancedController. |
| **GetTimeUntillSpell** | Return random time until next spell (8–11 seconds in frames). |
| **RandomBonus** | Pick a random power-up type; some types more likely; when in tower, ring fireball can appear. |
| **GetBonusImage** | Get ImageSequence for a bonus type index from Preloader. |
| **operator<<(BrokenLine)** | Write BrokenLine to ostream (segments separated by \|, end with &). |
| **operator>>(BrokenLine)** | Read BrokenLine from istream (text up to '&' split by '|' into segments of points). |
| **operator<<(LevelLayout)** | Write LevelLayout to ostream (LEVEL, FREQ, SPWN, CSTL, ROAD, TIME). |
| **operator>>(LevelLayout)** | Read level from istream; level number, spawn rates, spawn path, castles, roads, timer. |
| **GetRandNum** | Return random unsigned in [0, nRange) using float(rand())/(RAND_MAX+1)*nRange. |
| **GetRandFromDistribution** | Pick index from distribution given vector of weights (throws if bad range). |
| **GetFireballChainNum** | Get chain count from FireballBonus (uMap["fireballchainnum"]; if !=0 then +1). |
| **operator<<(Road)** | Write Road (bVertical, nCoord) to ostream. |
| **operator>>(Road)** | Read Road from istream. |

**Note:** Template functions (e.g. `CleanUp`, `CopyASSP`, `CopyArrayASSP`, `Union`, `Out`, `PushBackASSP`) and inline/template overloads are not listed as they are not plain global functions in the same sense. `Union` and `Out` are templates; the non-template free functions above are the ones with a single signature.
