# Phase 3: Game entities and animations — Research

**Date:** 2025-02-19
**Status:** RESEARCH COMPLETE

## Objective

Answer: "What do I need to know to PLAN this phase well?" for migrating leaf
game types (Entity hierarchy, Drawer/NumberDrawer, Animation, AnimationOnce,
StaticImage, simple entities) to single ownership (unique_ptr at owner, T*
elsewhere).

## Phase context (from ROADMAP)

- **Goal:** Migrate leaf game types: Entity hierarchy, Drawer/NumberDrawer and
  concrete drawers, AnimationOnce, StaticImage, Animation, simple entities.
  Ownership lives in level, controllers, or critters.
- **Success criteria:** (1) Leaf game entities and animations with clear
  single owner migrated; SP_Info removed where no smart_pointer<T> remains.
  (2) Build and all tests pass. (3) Any type skipped (lifetime unclear)
  recorded for final summary.
- **Out of scope for Phase 3:** Critters, generators, fireball types, level-
  owned complex types (Dragon, Castle, Road, etc.) — those are Phase 4.

## Current ownership and usage

### Drawer / ScalingDrawer / NumberDrawer

| Type | Extends SP_Info | Holds smart_pointer | Creation site(s) |
|------|-----------------|---------------------|------------------|
| Drawer | Yes | No (pGr is raw) | — (base) |
| ScalingDrawer | Yes (via Drawer) | No | TowerDataWrap ctor (dragon_game_runner.cc) |
| NumberDrawer | Yes | smart_pointer<ScalingDrawer> pDr | TowerDataWrap ctor (dragon_game_runner.cc) |

**ScalingDrawer creation:** `dragon_game_runner.cc` TowerDataWrap::TowerDataWrap:
- `pDr = make_smart(new ScalingDrawer(pGr, nScale));`
- `smart_pointer<ScalingDrawer> pBigDr = make_smart(new ScalingDrawer(pGr, nScale * 2));`
- pBigDr is passed to NumberDrawer ctor for pBigNum; pDr passed for pNum.

**NumberDrawer creation:** Same file:
- `pNum = make_smart(new NumberDrawer(pDr, fp_.get(), ...));`
- `pBigNum = make_smart(new NumberDrawer(pBigDr, fp_.get(), ...));`

**Who holds ScalingDrawer/NumberDrawer:**
- **TowerDataWrap** (dragon_game_runner.h): pDr, pNum, pBigNum (smart_pointer).
- **DragonGameController** (dragon_game_controller.h): pDr, pNum, pBigNum
  (smart_pointer); receives from TowerDataWrap via ctor and stores copy.
- **GetDrawer() / GetNumberDrawer() / GetBigNumberDrawer()** return
  smart_pointer; callers use .get() or pass to Draw().
- **NumberDrawer** is also held by: TextDrawEntity (pNum), TutorialTextEntity
  (pNum), MenuDisplay (pNum). All receive from controller (GetNumberDrawer() or
  ctor arg).

**Draw(ScalingDrawer) usage:** Virtual method on VisualEntity and overrides take
`smart_pointer<ScalingDrawer> pDr`. Called from basic_controllers.cc
`entry.second->Draw(pGl->GetDrawer())`, menu_controller.cc, level_controller.cc
(AdNumberDrawer, BonusDrawer), entities.cc, critters.cc, fireball.cc, dragon.cc,
buy_now_controller.cc, tutorial.cc, level.cc (Road, FancyRoad). All need to
switch to ScalingDrawer* after migration.

### Entity hierarchy (SP_Info and smart_pointer usage)

| Type | Extends SP_Info | Holds smart_pointer | Creation sites |
|------|-----------------|---------------------|----------------|
| Entity | Yes | — | (base) |
| VisualEntity | (Entity) | — | (base) |
| TextDrawEntity | (Entity) | smart_pointer<NumberDrawer> pNum | dragon_game_controller.cc, menu |
| SimpleVisualEntity | (Entity) | — | (base for Animation, etc.) |
| SimpleSoundEntity | (Entity) | smart_pointer<SoundInterfaceProxy> pSnd | dragon_game_controller.cc |
| Animation | SimpleVisualEntity | — | dragon_game_controller.cc, buy_now_controller.cc |
| AnimationOnce | SimpleVisualEntity | — | dragon_game_controller, critters, fireball, level_controller, critter_generators |
| StaticImage | VisualEntity | — | dragon_game_controller.cc, basic_controllers (none; TRIAL pTrial, pBuyNow) |
| StaticRectangle | VisualEntity | — | basic_controllers.cc AddBackground |
| BonusScore | EventEntity, VisualEntity | — | critters.cc, level_controller (none direct) |
| HighScoreShower | VisualEntity | — | basic_controllers.cc DragonScoreController ctor |
| IntroTextShower | VisualEntity | — | basic_controllers.cc DragonScoreController ctor |
| AdNumberDrawer | VisualEntity | — | level_controller.cc Init (local struct) |
| BonusDrawer | VisualEntity | — | level_controller.cc Init (local struct) |
| TutorialTextEntity | EventEntity, VisualEntity | smart_pointer<NumberDrawer> pNum | level_controller.cc Init, tutorial.h (TutorialLevelOne/Two hold smart_pointer<TutorialTextEntity>) |
| MenuDisplay | EventEntity, VisualEntity | pNum, smart_pointer<Animation> pMenuCaret | dragon_game_controller.cc |
| SoundControls | EventEntity | — | level_controller (pSc) |
| PhysicalEntity | (Entity) | — | HitDetection(smart_pointer<PhysicalEntity>) param |
| FancyCritter | PhysicalEntity, SimpleVisualEntity | — | basic_controllers.cc Cutscene (Phase 4) |

**Creation sites (leaf / simple entities):**

- **StaticRectangle:** basic_controllers.cc AddBackground — make_smart(new
  StaticRectangle(...)); AddV(pBkg). Owner: EntityListController (via lsDraw).
- **StaticImage:** dragon_game_controller.cc (pTrial, pBuyNow); AddV/AddBoth.
  Owner: controller that receives (pCnt1, pBuy).
- **Animation:** dragon_game_controller.cc (pMenuCaret, pWin, pBurnL, pBurnR,
  pGolem, pSkeleton1–3, pMage, pGhost, pWhiteKnight); buy_now_controller.cc
  (mSlimes). pMenuCaret is also stored in MenuDisplay and added via
  pMenu->AddBoth(pMenuCaret). Owners: DragonGameController (via controller
  lists), MenuController, BuyNowController (mSlimes vector).
- **AnimationOnce:** dragon_game_controller, level_controller (none direct),
  critters.cc (Princess, Trader, Knight, Mage, Slime, etc.), fireball.cc,
  critter_generators.cc. Owners: controllers (AddBoth/AddV), critters (local
  then passed to level AddBoth), LevelController lists.
- **TextDrawEntity:** dragon_game_controller (pHintText, pOptionText); given
  to pMenu or pCnt1. Owner: MenuController or StartScreenController.
- **SimpleSoundEntity:** dragon_game_controller (pOver, pPluSnd, pClkSnd);
  AddE to various controllers. Owner: controller.
- **HighScoreShower / IntroTextShower:** basic_controllers.cc
  DragonScoreController ctor; AddV. Owner: DragonScoreController.
- **BonusScore:** critters.cc (Princess, Trader, Knight, Slime, etc.);
  pAc->AddV(pB) or AddBoth. Owner: LevelController (lsDraw/lsUpdate).
- **AdNumberDrawer / BonusDrawer:** level_controller.cc Init; make_smart, AddV.
  Owner: LevelController.
- **TutorialTextEntity:** level_controller.cc Init; pTutorialText = pTT;
  TutorialLevelOne/Two hold pTexter (same entity). Owner: LevelController;
  tutorial objects hold non-owning reference.
- **MenuDisplay:** dragon_game_controller; pMenu->AddBoth(pMenuDisplay),
  pMenu->pMenuDisplay = pMenuDisplay. Owner: MenuController (list + pMenuDisplay).
- **SoundControls:** level_controller (pSc = pBckgMusic or similar). Check: pSc
  is smart_pointer<SoundControls> in level_controller.h; creation in
  level_controller.cc. Owner: LevelController.

**Entity list ownership (current):** EntityListController has
lsDraw (list<smart_pointer<VisualEntity>>), lsUpdate
(list<smart_pointer<EventEntity>>), lsPpl (list<smart_pointer<ConsumableEntity>>)
and already has owned_entities (list<unique_ptr<Entity>>),
owned_visual_entities (list<VisualEntity*>), owned_event_entities
(list<EventEntity*>). AddV/AddE take smart_pointer; AddOwnedVisualEntity,
AddOwnedEventEntity, AddOwnedBoth take unique_ptr. Draw/Update loops iterate
both lsDraw/lsUpdate and owned_* lists. CleanUp is applied to all; for
owned_* the template CleanUp(list<RawPtr>) erases when !bExist (object still
owned in owned_entities — cleanup semantics for owned list must remove from
owned_entities and keep raw lists in sync).

### ImageSequence

- **image_sequence.h:** ImageSequence extends SP_Info. No smart_pointer<
  ImageSequence> in codebase; used by value (e.g. SimpleVisualEntity::seq).
- Optional cleanup: remove SP_Info from ImageSequence once no ref-count use
  (CLEAN-01).

### Dependencies on Phase 2

- **TowerDataWrap / ProgramEngine:** Phase 2 leaves pGr and pSm as raw
  pointers (GraphicalInterface<Index>*, SoundInterface<Index>*). FontWriter is
  already unique_ptr (pFancyNum). So TowerDataWrap now has: pGr, pSm (raw),
  pFancyNum (unique_ptr), pDr, pNum, pBigNum (still smart_pointer).
- **DragonGameController** ctor takes FontWriter* (pFancyNum_), SoundInterface<
  Index>* (pSndRaw_); holds pDr, pNum, pBigNum as smart_pointer. After Phase 3
  drawer migration, it should hold raw pointers to drawer/number drawer (from
  TowerDataWrap) or own nothing (TowerDataWrap remains sole owner of drawers).
- **SoundInterfaceProxy:** Still extends SP_Info; DragonGameController holds
  smart_pointer<SoundInterfaceProxy> pSnd. If Phase 2 did not migrate
  SoundInterfaceProxy, SimpleSoundEntity will keep holding
  smart_pointer<SoundInterfaceProxy> until that type is migrated; Phase 3 can
  still migrate SimpleSoundEntity to unique_ptr at controller and
  SoundInterfaceProxy* in SimpleSoundEntity if SoundInterfaceProxy ownership is
  clarified (e.g. single owner in DragonGameController, raw elsewhere).

## Target ownership

1. **ScalingDrawer**
   - Owner: TowerDataWrap only. Hold `std::unique_ptr<ScalingDrawer> pDr` and
     a second unique_ptr for the “big” drawer used by pBigNum (e.g.
     pBigDr or owned inside a wrapper). Both created in TowerDataWrap ctor.
   - DragonGameController: store ScalingDrawer* (from TowerDataWrap); GetDrawer()
     returns ScalingDrawer*.
   - All Draw(ScalingDrawer*) and other call sites: take ScalingDrawer*.
   - NumberDrawer ctor and member: take/store ScalingDrawer* (non-owning).
   - Remove SP_Info from Drawer/ScalingDrawer when no smart_pointer<
     ScalingDrawer> remains (CLEAN-01).

2. **NumberDrawer**
   - Owner: TowerDataWrap only. Hold `std::unique_ptr<NumberDrawer> pNum`,
     `std::unique_ptr<NumberDrawer> pBigNum`.
   - DragonGameController: store NumberDrawer* for pNum, pBigNum; getters
     return NumberDrawer*.
   - TextDrawEntity, TutorialTextEntity, MenuDisplay: store NumberDrawer*
     (ctor/SetText take raw pointer).
   - Remove SP_Info from NumberDrawer when no smart_pointer<NumberDrawer>
     remains (CLEAN-01).

3. **Leaf entities (Animation, AnimationOnce, StaticImage, StaticRectangle,
   TextDrawEntity, SimpleSoundEntity, HighScoreShower, IntroTextShower,
   BonusScore, AdNumberDrawer, BonusDrawer, TutorialTextEntity, MenuDisplay,
   SoundControls)**
   - Owner: the controller (or level/critter) that creates and adds them:
     EntityListController (and subclasses), LevelController, BuyNowController,
     etc. Use unique_ptr at that owner; add to draw/update via existing
     owned_* + raw list pattern (AddOwnedVisualEntity / AddOwnedEventEntity /
     AddOwnedBoth) and store raw pointers in lsDraw/lsUpdate, or migrate
     lsDraw/lsUpdate to raw pointers and a single owned list.
   - Entities that hold a reference to another entity type (e.g. TextDrawEntity
     holds NumberDrawer*, MenuDisplay holds NumberDrawer* and Animation*):
     non-owning raw pointers.
   - PhysicalEntity::HitDetection: change to HitDetection(PhysicalEntity*)
     (raw pointer).
   - Remove SP_Info from Entity and subclasses only when no smart_pointer<T>
     points to that class (CLEAN-01). Entity is base of many; removal of SP_Info
     from Entity happens when all entity types using smart_pointer are
     migrated.

4. **ImageSequence (optional)**
   - No smart_pointer<ImageSequence>; remove SP_Info from ImageSequence
     (CLEAN-01) if desired in Phase 3.

## Migration order (waves)

**Wave 1 — ScalingDrawer and Draw(ScalingDrawer*)**
- TowerDataWrap: create and store two unique_ptr<ScalingDrawer> (pDr, pBigDr);
  expose raw pointers for NumberDrawer ctors and GetDrawer().
- dragon_game_runner.cc: use make_unique for ScalingDrawer; pass .get() to
  NumberDrawer and to DragonGameController.
- DragonGameController: store ScalingDrawer*; GetDrawer() returns
  ScalingDrawer*.
- Change all VisualEntity::Draw and overrides from
  `void Draw(smart_pointer<ScalingDrawer> pDr)` to `void Draw(ScalingDrawer*
  pDr)` in entities.h, entities.cc, level.h, level.cc, fireball.h, fireball.cc,
  critters.h, critters.cc, dragon.h, dragon.cc, menu_controller.h,
  menu_controller.cc, tutorial.h, tutorial.cc, buy_now_controller,
  level_controller.cc (AdNumberDrawer, BonusDrawer), basic_controllers.cc.
- Draw call sites: already pass pGl->GetDrawer() (will be raw ptr).
- Remove SP_Info from Drawer and ScalingDrawer when no smart_pointer<
  ScalingDrawer> remains.

**Wave 2 — NumberDrawer**
- TowerDataWrap: hold unique_ptr<NumberDrawer> pNum, pBigNum; create with
  pDr.get() and pBigDr.get().
- dragon_game_runner.cc: use make_unique for NumberDrawer; pass raw to
  DragonGameController.
- DragonGameController: store NumberDrawer* pNum, pBigNum; getters return
  NumberDrawer*.
- NumberDrawer: ctor and member pDr become ScalingDrawer* (draw_utils.h,
  draw_utils.cc).
- TextDrawEntity, TutorialTextEntity, MenuDisplay: change smart_pointer<
  NumberDrawer> to NumberDrawer* (entities.h, tutorial.h, menu_controller.h;
  corresponding .cc and call sites pass GetNumberDrawer() or raw ptr).
- Remove SP_Info from NumberDrawer when no smart_pointer<NumberDrawer>
  remains.

**Wave 3 — Leaf entities (bottom-up by dependency)**
- **3a — No dependency on other smart_pointer entity types:** StaticImage,
  StaticRectangle, Animation, AnimationOnce, HighScoreShower, IntroTextShower,
  BonusScore, AdNumberDrawer, BonusDrawer, SimpleSoundEntity (if
  SoundInterfaceProxy* is acceptable), SoundControls. Creation sites switch
  to make_unique and AddOwnedVisualEntity/AddOwnedEventEntity/AddOwnedBoth (or
  equivalent); controllers hold unique_ptr or put in owned_entities and raw in
  lsDraw/lsUpdate. Ensure CleanUp for owned lists removes dead entities and
  keeps raw lists in sync.
- **3b — Depend on NumberDrawer* (already raw):** TextDrawEntity,
  TutorialTextEntity, MenuDisplay. Same ownership pattern; they already hold
  NumberDrawer* after Wave 2.
- **3c — Shared reference (e.g. pMenuCaret):** MenuDisplay and MenuController
  both reference the same Animation. Owner: one place (e.g. MenuController
  holds unique_ptr<Animation> for caret, MenuDisplay receives Animation* in
  ctor).
- **3d — Entity base and SP_Info:** After all leaf entity types are migrated
  off smart_pointer, remove SP_Info from Entity (and any subclass that has no
  remaining smart_pointer use) per CLEAN-01.

**Wave 4 (optional) — ImageSequence**
- Remove SP_Info from ImageSequence (image_sequence.h) if no ref-count use.

## Files to touch (summary)

**Headers**
- game_utils/draw_utils.h — Drawer, ScalingDrawer, NumberDrawer (SP_Info;
  NumberDrawer::pDr → ScalingDrawer*).
- game/entities.h — Entity, VisualEntity::Draw(ScalingDrawer*), TextDrawEntity
  (pNum → NumberDrawer*), SimpleSoundEntity (pSnd → SoundInterfaceProxy* if
  migrated), PhysicalEntity::HitDetection(PhysicalEntity*), all Draw
  signatures.
- game/level.h — Road::Draw, FancyRoad::Draw (ScalingDrawer*).
- game/fireball.h — FireballBonusAnimation::Draw, etc. (ScalingDrawer*).
- game/critters.h — Draw(ScalingDrawer*) overrides.
- game/dragon.h — Dragon::Draw(ScalingDrawer*).
- game/controller/basic_controllers.h — EntityListController: lsDraw/lsUpdate
  types if changed to raw ptr; AddV/AddE signatures if changed to unique_ptr.
- game/controller/menu_controller.h — MenuDisplay pNum, pMenuCaret (raw);
  MenuEntry (SP_Info only if removed later).
- game/controller/dragon_game_controller.h — pDr, pNum, pBigNum (raw);
  GetDrawer/GetNumberDrawer/GetBigNumberDrawer return raw ptr.
- game/controller/level_controller.h — (AdNumberDrawer, BonusDrawer local;
  pSc, pTutorialText ownership).
- game/controller/buy_now_controller.h — Draw(ScalingDrawer*); mSlimes type.
- game/tutorial.h — TutorialTextEntity pNum (raw); TutorialLevelOne/Two
  pTexter (ownership: LevelController owns, tutorial holds raw or keep
  smart_pointer until TutorialTextEntity migrated).
- game_utils/image_sequence.h — Optional: remove SP_Info from ImageSequence.

**Implementation**
- game_utils/draw_utils.cc — NumberDrawer ctor and usages (ScalingDrawer*).
- game/entities.cc — All Draw(ScalingDrawer*); HitDetection(PhysicalEntity*).
- game/level.cc — Road::Draw, FancyRoad::Draw.
- game/fireball.cc — Draw(ScalingDrawer*); AnimationOnce/Fireball creation and
  list types (Phase 3 only where leaf entities are migrated).
- game/critters.cc — Draw(ScalingDrawer*); BonusScore, AnimationOnce
  creation; list types.
- game/dragon.cc — Dragon::Draw(ScalingDrawer*).
- game/dragon_game_runner.cc — TowerDataWrap: make_unique for ScalingDrawer,
  NumberDrawer; pass raw to DragonGameController.
- game/dragon_game_runner.h — TowerDataWrap: unique_ptr for pDr, pNum, pBigNum
  (and second ScalingDrawer if stored).
- game/controller/dragon_game_controller.cc — Ctor take ScalingDrawer*,
  NumberDrawer*; store raw; getters return raw; all entity creation sites
  (make_unique, AddOwned* or equivalent).
- game/controller/basic_controllers.cc — AddBackground: make_unique
  StaticRectangle, AddOwnedVisualEntity; DragonScoreController: HighScoreShower
  / IntroTextShower owned; Cutscene: FancyCritter (Phase 4); CleanUp/owned
  list handling if needed.
- game/controller/menu_controller.cc — Draw(ScalingDrawer*); MenuDisplay ctor
  (NumberDrawer*, Animation*).
- game/controller/level_controller.cc — AdNumberDrawer, BonusDrawer Draw(
  ScalingDrawer*); Init: AdNumberDrawer, BonusDrawer, TutorialTextEntity
  ownership (unique_ptr at LevelController, raw in lists).
- game/controller/buy_now_controller.cc — Draw(ScalingDrawer*); mSlimes
  (vector of unique_ptr or raw depending on ownership).
- game/tutorial.cc — TutorialTextEntity::Draw(ScalingDrawer*); pNum (raw).

**Other**
- simulation_test.cc, simulation.cc — No direct ScalingDrawer/NumberDrawer
  ownership; may reference GetDrawer() type (return type change only).

## Risks and notes

- **Entity list duality:** Controllers use both smart_pointer lists (lsDraw,
  lsUpdate) and owned_entities + raw lists. Migrating entity types to
  unique_ptr requires consistent cleanup: when an owned entity’s bExist becomes
  false, remove it from owned_entities (and sync owned_visual_entities /
  owned_event_entities). Current CleanUp(list<VisualEntity*>) erases the raw
  pointer from the list but does not release the unique_ptr; cleanup logic must
  be updated so that the single owner (owned_entities) is the source of truth
  and raw lists are updated when owned_entities is pruned.
- **Shared references (pMenuCaret):** Animation is in both MenuDisplay and
  controller lists. Assign a single owner (e.g. MenuController); MenuDisplay
  gets Animation* in ctor.
- **Types deferred to Phase 4:** Critter, FancyCritter, Dragon, Castle, Road,
  FancyRoad, LevelController’s vRd, vDr, generators, fireball types (Fireball,
  ChainExplosion, KnightOnFire, etc.), TimedFireballBonus, FireballBonusAnimation,
  ConsumableEntity subclasses, GameController (vCnt), MenuController. Do not
  migrate these in Phase 3; record if any have unclear lifetime for DOC-01.
- **SoundInterfaceProxy:** If still smart_pointer in DragonGameController,
  SimpleSoundEntity can hold SoundInterfaceProxy* once ownership is decided
  (e.g. DragonGameController owns one proxy, passes raw to SimpleSoundEntity).
- **Build and test:** After each wave (or each logical step), run build and
  ctest; run simulation_test.exe (VER-01, VER-02).
- **Skipped types:** If any leaf type has unclear lifetime (e.g. shared across
  callbacks, stored in container with unclear owner), do not migrate; record
  for DOC-01 per success criteria.

## RESEARCH COMPLETE

Findings above are sufficient to produce an executable plan with waves and
tasks. No blockers. Phase 2 outcome (raw pGr, pSm; unique_ptr FontWriter) is
assumed; TowerDataWrap remains the single owner of ScalingDrawer and
NumberDrawer after migration.
