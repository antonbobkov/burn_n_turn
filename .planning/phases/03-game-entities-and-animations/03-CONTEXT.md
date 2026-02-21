# Phase 3: Game entities and animations — Context

**Gathered:** 2025-02-21  
**Status:** Ready for planning

<domain>
## Phase Boundary

Migrate leaf game types: Entity hierarchy, Drawer/NumberDrawer and concrete
drawers, AnimationOnce, StaticImage, Animation, simple entities. Ownership
lives in level, controllers, or critters. (ROADMAP Phase 3 — fixed.)

This discussion clarifies **who owns** objects that are used in multiple
places, and **who holds raw pointers**, so the plan can assign unique_ptr
and T* consistently.

</domain>

<decisions>
## Implementation Decisions

### Objects used in multiple places — ownership and raw pointers

The following objects are created in one place and used (stored or passed)
in several others. For each, **who owns (unique_ptr)** and **who holds raw
pointer** is set as below. The plan and executor should follow these
choices.

---

#### 1. ScalingDrawer (pDr, pBigDr)

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create/store | **TowerDataWrap** (dragon_game_runner) | — |
| Use for drawing | DragonGameController | `pDr` (from TowerDataWrap), returned by GetDrawer() |
| Use for drawing | NumberDrawer | `pDr` (ctor param; non-owning) |
| Call sites | All Draw(ScalingDrawer* pDr) | Parameter only |

**Decision:** TowerDataWrap owns both ScalingDrawers with unique_ptr.
DragonGameController and NumberDrawer hold ScalingDrawer* only. No other
owner.

---

#### 2. NumberDrawer (pNum, pBigNum)

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create/store | **TowerDataWrap** | — |
| Expose to game | DragonGameController | `pNum`, `pBigNum`; getters return NumberDrawer* |
| Use in entities | TextDrawEntity, TutorialTextEntity, MenuDisplay, AdNumberDrawer, BonusDrawer, HighScoreShower, BonusScore, Princess/Trader/Knight Draw, etc. | From pGl->GetNumberDrawer() or GetBigNumberDrawer() |

**Decision:** TowerDataWrap owns both NumberDrawers with unique_ptr.
DragonGameController stores and returns raw pointers. All entities and
level/critters use NumberDrawer* from getters only.

---

#### 3. Menu caret (pMenuCaret — Animation)

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | DragonGameController::StartUp | — |
| In draw/update lists | MenuController | In lsDraw/lsUpdate via AddBoth |
| Display reference | MenuDisplay | `pMenuCaret` (positions the caret when drawing) |

**Options considered:**
- **A.** MenuController owns (e.g. unique_ptr in owned list); MenuDisplay
  holds Animation*.
- **B.** DragonGameController owns; pass raw to MenuDisplay and to
  MenuController’s AddBoth.

**Decision:** **MenuController** owns the caret (e.g. unique_ptr in owned
list or single member). MenuDisplay holds **Animation*** only. Creation
can stay in DragonGameController::StartUp but ownership is transferred to
MenuController (e.g. AddOwnedBoth or equivalent); raw pointer passed to
MenuDisplay ctor and to AddBoth for draw/update.

---

#### 4. Shared visuals: logo and burn (pL, pBurnL, pBurnR)

Same entity added to **three** controllers: StartScreenController (pCnt1),
MenuController (pMenu), BuyNowController (pBuy).

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | DragonGameController::StartUp | — |
| In lists | pCnt1, pMenu, pBuy | lsDraw / lsUpdate |

**Options considered:**
- **A.** DragonGameController owns (e.g. dedicated owned list); pass raw to
  each controller’s AddV/AddBoth.
- **B.** First controller (e.g. pCnt1) owns; others hold raw. Risk: if
  controllers are destroyed in non-FIFO order, raw pointers in pMenu/pBuy
  can dangle.

**Decision:** **DragonGameController** owns pL, pBurnL, pBurnR (e.g.
unique_ptr in an owned list or members). pCnt1, pMenu, and pBuy hold **raw
pointers** in their lsDraw/lsUpdate only. CleanUp in controllers must not
delete these; only remove from lists.

---

#### 5. SoundControls: pBckgMusic and pNoMusic

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | DragonGameController::StartUp | — |
| In lists | pCnt1 (pBckgMusic); pMenu, pCut1–3, pCnt2, pCnt3 (pNoMusic); every LevelController (pBckgMusic) | AddE(...); LevelController::pSc = pBckgMusic.get() |

**Decision:** **DragonGameController** owns both SoundControls (unique_ptr
or owned list). All controllers that use them hold **raw pointers** only
(in lsUpdate and, for LevelController, pSc). Lifetime of SoundControls
extends beyond any single screen, so owner is the game controller.

---

#### 6. TutorialTextEntity (pTT) in LevelController

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | LevelController::Init | — |
| In lists / refs | LevelController (AddBoth, pTutorialText); tutOne/tutTwo (pTexter) | — |

**Decision:** **LevelController** owns (e.g. AddOwnedBoth; unique_ptr or
owned list). pTutorialText and tutOne/tutTwo hold **TutorialTextEntity***
only.

---

#### 7. BonusScore, AnimationOnce created in critters/generators/fireball

Created in Princess::OnHit, Trader::OnHit, Knight::OnHit, Mage, fireball,
etc., and given to LevelController via `pAc->AddOwnedBoth(std::make_unique<...>)`.

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | Critter/fireball/generator code | — |
| Store | LevelController | owned_* + raw in lsDraw/lsUpdate |

**Decision:** **LevelController** owns (AddOwnedBoth transfers ownership).
Critters/generators/fireball only create and transfer; they do not keep
pointers. Raw pointers in LevelController’s lists only.

---

#### 8. Other leaf entities (single controller)

Entities created and used by a single controller (e.g. StaticRectangle in
basic_controllers, AdNumberDrawer/BonusDrawer in level_controller Init,
StaticImage/Animation in dragon_game_controller for one screen only):
**that controller** owns with unique_ptr / AddOwned*; any internal or
list reference is raw.

---

### Phase 4 types (not owned in Phase 3)

- **Dragon** shared between LevelController::vDr and Castle::pDrag:
  ownership and raw assignment are Phase 4. LevelController will own
  (unique_ptr in vDr); Castle will hold Dragon* only.
- **Critter, Castle, Road, generators, TimedFireballBonus, ConsumableEntity,
  FancyCritter**: Phase 4; no ownership change in Phase 3.

---

### CleanUp and list sync

Controllers that own entities (unique_ptr / owned_*) must CleanUp so that
when an entity is removed (e.g. !bExist), it is removed from owned_*
and from lsDraw/lsUpdate in one pass. No raw pointer in a list may
outlive its unique_ptr. Plan should document or implement the chosen sync
strategy (e.g. same pass, or clear raw list when owned is cleared).

</decisions>

<specifics>
## Specific Ideas

- User requested explicit identification of objects used in multiple places
  and options for owner vs raw pointer. The above tables and decisions
  serve as the single reference for planner and executor.
- Preference: single clear owner (unique_ptr) at the place that creates or
  has the longest lifetime; everyone else raw. Avoid shared ownership for
  Phase 3 leaf types.
</specifics>

<deferred>
## Deferred Ideas

- Dragon/Castle pDrag ownership and LevelController::vDr: Phase 4.
- Any other shared-ownership or refcount patterns: handle in later phases
  or keep as documented exceptions.
</deferred>

---

*Phase: 03-game-entities-and-animations*  
*Context gathered: 2025-02-21*
