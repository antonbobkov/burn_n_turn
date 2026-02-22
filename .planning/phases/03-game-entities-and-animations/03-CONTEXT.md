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

### AddOwned... usage

Use **AddOwned...()** only for objects that are **not** used anywhere else.
Pattern: `AddOwned...(std::make_unique(...))`. If an object is used in more
than one place (another component, list, or reference), do not use
AddOwned—use explicit unique_ptr (or value/copies) and raw pointers or
copies as appropriate.

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

#### 2a. GetNonOwned* — who may override

**Decision:** **Only MenuController and LevelController** override
GetNonOwnedDrawEntities / GetNonOwnedUpdateEntities (for caret and
tutorial text respectively). **Simple controllers must not** override
GetNonOwned*; they receive entities by value (copies) and own them.
When one object is passed to multiple places, prefer making a copy;
controllers own the object.

---

#### 3. Menu caret (pMenuCaret — Animation)

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | DragonGameController::StartUp | — |
| Draw/update participation | MenuController | Via GetNonOwned* overrides |
| Display reference | MenuDisplay | `pMenuCaret` (positions the caret when drawing) |

**Options considered:**
- **A.** MenuController owns (e.g. unique_ptr); MenuDisplay holds Animation*.
- **B.** DragonGameController owns; pass raw to MenuDisplay and to
  MenuController. AddBoth takes smart_pointer, so not used for raw ptrs.

**Decision:** **MenuController** owns the caret via **explicit unique_ptr**
(not AddOwned..., since the caret is used in two places: MenuDisplay and
draw/update). MenuDisplay holds **Animation*** only; raw pointer passed to
MenuDisplay ctor. The caret is **not** added via AddBoth (AddBoth does not
work for raw pointers). Instead MenuController **overrides
GetNonOwnedDrawEntities and GetNonOwnedUpdateEntities** and returns the
caret there so the base draw/update loop includes it.

---

#### 4. Shared visuals: logo and burn (pL, pBurnL, pBurnR)

Same visual needed on **three** controllers: StartScreenController (pCnt1),
MenuController (pMenu), BuyNowController (pBuy).

**Options considered:**
- **A.** DragonGameController owns; pass raw to each controller’s
  AddV/AddBoth.
- **B.** First controller owns; others hold raw (dangling risk).
- **C.** Construct once on the stack (as non-pointers, by value), then
  pass **copies** to the controllers.

**Decision:** **Option C / 2A (copy semantics).** Construct these objects
once, then pass **copies** to pCnt1, pMenu, and pBuy. Each controller gets
its own copy and **owns** it (e.g. via AddOwned*). StaticImage and
Animation must be copyable; implement or verify copy constructors.
No shared ownership.

---

#### 5. SoundControls: pBckgMusic and pNoMusic

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | DragonGameController::StartUp (or per-controller) | — |
| In lists | pCnt1 (pBckgMusic); pMenu, pCut1–3, pCnt2, pCnt3 (pNoMusic); every LevelController (pBckgMusic) | AddE(...); LevelController::pSc |

**Decision:** **By value (copies).** Make **copies** of SoundControls and
pass a copy into each controller that needs one. **Each controller owns**
its copy (e.g. AddOwnedEventEntity or AddOwnedBoth). No shared ownership;
DragonGameController does not own SoundControls for other screens.
SoundControls must be copyable.

---

#### 6. TutorialTextEntity (pTT) in LevelController

| Role | Owner | Raw pointer |
|------|--------|-------------|
| Create | LevelController::Init | — |
| Store / refs | LevelController (pTutorialText); tutOne/tutTwo (pTexter) | — |

**Decision:** **LevelController** owns by storing **unique_ptr explicitly**
(no AddOwned calls). pTutorialText is that unique_ptr; tutOne/tutTwo hold
**TutorialTextEntity*** only. TutorialTextEntity must be drawn via
**GetNonOwnedDrawEntities** override (LevelController returns it in that
override so the base draw loop includes it).

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
- Preference: when one object is passed to multiple places, **prefer making
  a copy**; the controller that receives it **owns** the object. Avoid
  shared ownership for Phase 3 leaf types. Single-owner (unique_ptr) at
  the controller that holds the copy; no GetNonOwned* for simple
  controllers.
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
