# Phase 4: Critters, generators, fireball, level — Context

**Gathered:** 2025-02-23  
**Status:** Ready for planning

<domain>

## Phase Boundary

Migrate types that **own** leaf entities: critters, generators (Princess, Trader, Knight, Mage, Skelly, Slime, etc.), fireball-related types, and level-owned structures. Bottom-up: these come after Phase 3. Scope is fixed; discussion clarifies how to implement within this boundary.

</domain>

<decisions>

## Rule: EntityListController ownership

**EntityListController should only own entities that are not referred to anywhere else.**  
If an entity is owned or referenced by another object (e.g. Dragon, LevelController, Castle), it must not also be stored in the base `lsDraw` / `lsUpdate` with ownership. Use the **GetNonOwnedUpdateEntities** / **GetNonOwnedDrawEntities** pattern to expose such entities for the update/draw loop; the single owner holds them (e.g. in a `unique_ptr` or a dedicated list).

---

## Objects Stored in Multiple Places — Ownership

Below is the list of objects that currently appear in more than one owning or referencing container. For each, the **proposed canonical owner**, **code snippets** for context, and your decisions are recorded.

---

### 1. **Dragon**

| Where stored | Type | Role |
|--------------|------|------|
| `LevelController::vDr` | `std::vector<smart_pointer<Dragon>>` | Level's list of dragons |
| `Castle::pDrag` | `smart_pointer<Dragon>` | Castle's reference when dragon is parked there |

**Code (current):** LevelController::Init assigns the same dragon to the castle:

```cpp
// level_controller.cc Init
vDr.push_back(make_smart(new Dragon(vCs[0].get(), pSelf, ...)));
if (vDr.back()->pCs != nullptr)
  vDr.back()->pCs->pDrag = vDr.back();
```

When dragon lands (Dragon::Toggle): `pCs->pDrag = pAd->FindDragon(this);` — castle gets a reference from `vDr`.

**Proposed owner:** **LevelController::vDr** is the single owner. Castle holds a **non-owning pointer** (`Dragon*`). No second `smart_pointer`/`unique_ptr` to the same dragon.

---

### 2. **Fireball** (player-shot and chain)

| Where stored | Type | Role |
|--------------|------|------|
| `Dragon::lsBalls` | `std::list<smart_pointer<Fireball>>` | One fireball per multi-shot (the “middle” one) |
| `EntityListController::lsDraw` / `lsUpdate` | `smart_pointer<Fireball>` | All active fireballs (via `AddBoth`) |

**Code (current):**

```cpp
// dragon.cc Dragon::Fire
void Dragon::Fire(fPoint fDir) {
  CleanUp(lsBalls);   // remove dead fireballs from dragon's list
  // ...
  for (unsigned i = 0; i < nNumber; ++i) {
    smart_pointer<Fireball> pFb = make_smart(new Fireball(...));
    pAd->AddBoth(pFb);           // level owns for draw/update
    if (i == nNumber / 2)
      lsBalls.push_back(pFb);    // dragon also keeps ref to one fireball
  }
}
```

**Role of `lsBalls`:** It holds at most one fireball per multi-shot (the one at index `nNumber/2`). At the next `Fire()`, `CleanUp(lsBalls)` removes that fireball if it has expired (`bExist == false`). The level already runs `CleanUp(lsUpdate)` / `CleanUp(lsDraw)` in `EntityListController::Update()`, so dead fireballs are removed from the level’s lists regardless. **We can delete `lsBalls`:** the level is the single owner and performs cleanup; Dragon does not need an owning list. If any gameplay ever needed to “track” one fireball of a shot, use a non-owning `Fireball*` or an id, not a second owning container.

**Decision:** Level owns all fireballs (in level-owned storage; see EntityListController rule). **Delete `Dragon::lsBalls`.** Dragon does not own fireballs; level cleans them up in its own update loop.

---

### 3. **FireballBonusAnimation**

| Where stored | Type | Role |
|--------------|------|------|
| `LevelController::lsBonus` | `std::list<smart_pointer<FireballBonusAnimation>>` | Level’s list of bonus animations |
| `EntityListController::lsDraw` / `lsUpdate` | same | Same objects added via `AddBoth` + `lsBonus.push_back` |

**Code (current):** Critters create and register in both places:

```cpp
// critters.cc (e.g. Trader death)
smart_pointer<FireballBonusAnimation> pFb = make_smart(
    new FireballBonusAnimation(GetPosition(), RandomBonus(false), pAc));
pAc->AddBoth(pFb);           // base lsDraw, lsUpdate
pAc->lsBonus.push_back(pFb); // LevelController’s list
```

Cleanup today: `CleanUp(pAd->lsBonus)` is called from **Dragon::Update** (when flying, before hit-testing bonuses) and from **Knight::Update** (before skeleton hit-testing). So the level’s list is cleaned by code that iterates it.

```cpp
// dragon.cc Dragon::Update (when bFly)
CleanUp(pAd->lsBonus);
for (auto itr = pAd->lsBonus.begin(); itr != pAd->lsBonus.end(); ++itr) {
  if (this->HitDetection(itr->get())) {
    AddBonus(GetBonus((*itr)->n, nBonusPickUpTime));
    (*itr)->bExist = false;
    // ...
  }
}
```

**Decision: Option B** — **LevelController::lsBonus** is the single owning list. These entities are **not** stored in EntityListController’s `lsDraw`/`lsUpdate`. LevelController exposes them via **GetNonOwnedDrawEntities** / **GetNonOwnedUpdateEntities** so the base update/draw loop runs them. **Cleanup:** LevelController must ensure dead animations are removed. Either:
- LevelController::Update() calls `CleanUp(lsBonus)` at the start of the frame (before or after calling base Update), or
- Keep cleanup in the same places that currently iterate `lsBonus` (Dragon::Update, Knight::Update, etc.) but ensure they only set `bExist = false` and that LevelController runs `CleanUp(lsBonus)` once per frame so the owning list stays consistent. Planner should centralize cleanup in LevelController so `lsBonus` is the single source of truth and gets cleaned in one place.

---

### 4. **TimedFireballBonus** (on dragon)

| Where stored | Type | Role |
|--------------|------|------|
| `Dragon::lsBonuses` | `std::list<smart_pointer<TimedFireballBonus>>` | Active bonuses on the dragon |
| `EntityListController::lsUpdate` | same | Added via `AddE(pBonus)` so they tick each frame |

**Code (current):**

When a bonus is added to the dragon (e.g. pickup or carry-over):

```cpp
// dragon.cc
void Dragon::AddBonus(smart_pointer<TimedFireballBonus> pBonus, bool bSilent) {
  if (pBonus.is_null()) return;
  lsBonuses.push_back(pBonus);   // Dragon owns
  pAd->AddE(pBonus);             // Level’s lsUpdate also holds a ref → duplicate
}
```

So the same bonus is in both Dragon’s list and the base `lsUpdate`. The base Update loop then calls `Update()` on each so timers tick:

```cpp
// basic_controllers.cc EntityListController::Update
for (auto &p : lsUpdate) {
  if (p->bExist) p->Update();
}
// ...
for (EventEntity *pEx : GetNonOwnedUpdateEntities()) {
  if (pEx->bExist) pEx->Update();
}
```

Other uses of `lsBonuses`:
- **GetAllBonuses()** (for fireball stats): `CleanUp(lsBonuses)` then iterates to aggregate.
- **FlushBonuses()** (on level win): iterates `lsBonuses` and calls `pAd->pGl->AddBonusToCarryOver(*itr)`.

**Decision:** **Dragon::lsBonuses** is the single owner. Level must **not** store these in `lsUpdate`. LevelController implements **GetNonOwnedUpdateEntities()** (and **GetNonOwnedDrawEntities()** if bonuses are drawn) to return pointers to the dragon’s bonuses so the base loop updates/draws them without owning them.

---

### 5. **LevelLayout**

| Where stored | Type | Role |
|--------------|------|------|
| `DragonGameRunner::vLvl` | `std::vector<LevelLayout>` | Loaded from file |
| `DragonGameController::vLvl` | `std::vector<LevelLayout>` | Copy (constructor takes `const std::vector<LevelLayout>&` and copies) |

**Code (LevelLayout type):**

```cpp
// level.h
struct LevelLayout {
  Rectangle sBound;
  unsigned nLvl;
  BrokenLine blKnightGen;
  std::vector<Point> vCastleLoc;
  std::vector<Road> vRoadGen;
  unsigned nTimer;
  std::vector<float> vFreq;
  void Convert(int n = 24);
};
```

**LevelLayout does not use smart pointers.** It is a value type (plain struct with value members). **There is no need to migrate the LevelLayout type itself for smart pointers.** The only design choice is whether to keep two copies of `std::vector<LevelLayout>` (Runner and DGC) or have one owner and the other hold a non-owning reference (pointer/span). If we keep a single copy for clarity, Runner can own and DGC can take `const std::vector<LevelLayout>*` or a span; otherwise leave as-is and do not treat LevelLayout as a Phase 4 migration target for ownership.

---

### 6. **TimedFireballBonus (carry-over between levels)**

| Where stored | Type | Role |
|--------------|------|------|
| `DragonGameController::lsBonusesToCarryOver` | `std::list<smart_pointer<TimedFireballBonus>>` | Bonuses to give to the next level |
| (after handoff) `Dragon::lsBonuses` | same | RecoverBonuses() adds them to the dragon |

**Handoff logic (when do we handoff?):**

1. **Level won**  
   When the player wins the level, code calls **FlushBonuses()** on the current dragon:
   - Iterates `Dragon::lsBonuses` and for each calls `pAd->pGl->AddBonusToCarryOver(*itr)`.
   - So DGC’s `lsBonusesToCarryOver` gets references to those bonuses (shared refs with current dragon until level is torn down).

2. **New level starts**  
   - **LevelController::Init** creates the new level and the new **Dragon**.  
   - In **Dragon::Dragon** ctor, the last line is `pAd->pGl->ClearBonusesToCarryOver()`, which clears DGC’s list. So with current order, the list is cleared as soon as the new dragon is created.  
   - In **LevelController::Update()**, on the **first** tick (`bFirstUpdate`), for levels where we recover (e.g. not 1, 4, 7, 10): we call `vDr[0]->RecoverBonuses()` then `pGl->ClearBonusesToCarryOver()`.

So the **intended** handoff is: DGC holds the list until the new level’s first Update; then RecoverBonuses() moves bonuses into the new dragon (AddBonus for each), then ClearBonusesToCarryOver(). The Dragon ctor’s ClearBonusesToCarryOver() can clear the list before RecoverBonuses runs; the planner should ensure handoff happens in one place (e.g. first Update: RecoverBonuses then Clear) and that the carry-over list is not cleared before RecoverBonuses.

**Code (current):**

```cpp
// dragon.cc
void Dragon::FlushBonuses() {
  for (auto itr = lsBonuses.begin(), etr = lsBonuses.end(); itr != etr; ++itr)
    pAd->pGl->AddBonusToCarryOver(*itr);
}
void Dragon::RecoverBonuses() {
  const auto &lst = pAd->pGl->GetBonusesToCarryOver();
  for (auto itr = lst.begin(), etr = lst.end(); itr != etr; ++itr)
    AddBonus(*itr, true);
}
// Dragon ctor ends with:
  pAd->pGl->ClearBonusesToCarryOver();
```

```cpp
// level_controller.cc Update (first tick)
if (bFirstUpdate) {
  bFirstUpdate = false;
  if (nLvl != 1 && nLvl != 4 && nLvl != 7 && nLvl != 10) {
    vDr[0]->RecoverBonuses();
  }
  pGl->ClearBonusesToCarryOver();
}
```

**Decision:** Single owner at any time. Handoff by **move**: when the new level’s first Update runs, RecoverBonuses takes ownership from DGC (e.g. move from lsBonusesToCarryOver into Dragon::lsBonuses), then ClearBonusesToCarryOver(). Planner should unify clear so the list is not cleared before RecoverBonuses.

---

### 7. **Generators (Knight, Princess, Trader, Mage, Skelly)**

| Where stored | Type | Role |
|--------------|------|------|
| `EntityListController::lsUpdate` | `smart_pointer<Generator>` | Created in LevelController::Init, added via AddE |
| `LevelController::pGr`, `pMgGen` | Raw pointers | Fast access for cheats / MageGenerate |

**Code (current):**

```cpp
// level_controller.cc Init
smart_pointer<KnightGenerator> pGen = make_smart(
    new KnightGenerator(lvl.vFreq.at(0), rBound, pSelf, lvl.blKnightGen));
// ... pPGen, pTGen, pMGen ...
pGr = pGen.get();
pMgGen = pMGen.get();
// ...
AddE(pGen);
AddE(pPGen);
AddE(pTGen);
AddE(pMGen);
```

So generators live in the base **lsUpdate**; LevelController only holds raw pointers.

**Decision:** **LevelController** owns generators (e.g. `std::vector<std::unique_ptr<EventEntity>>` or one `unique_ptr` per generator type). **Do not** put them in EntityListController’s `lsUpdate`. LevelController exposes them via **GetNonOwnedUpdateEntities()** (and **GetNonOwnedDrawEntities()** if they are drawn) so the base update/draw loop runs them. `pGr` and `pMgGen` remain non-owning raw pointers into that owned storage.

---

## Summary Table

| Object | Current “extra” place | Proposed single owner | Note |
|--------|------------------------|------------------------|------|
| Dragon | Castle::pDrag | LevelController::vDr | Castle: non-owning pointer |
| Fireball | Dragon::lsBalls | Level (entity lists / level-owned) | **Delete lsBalls** |
| FireballBonusAnimation | lsDraw/lsUpdate and lsBonus | LevelController::lsBonus only | Option B; GetNonOwned*; cleanup in LevelController |
| TimedFireballBonus (on dragon) | lsUpdate | Dragon::lsBonuses | Level: GetNonOwned* only |
| LevelLayout | DGC copy | N/A (no smart pointers in type) | No migration of type; optional single vector if desired |
| TimedFireballBonus (carry-over) | Handoff | DGC → Dragon on first Update | Move; RecoverBonuses then Clear; unify clear |
| Generators | Base lsUpdate | LevelController (unique_ptr) | GetNonOwnedUpdateEntities pattern |

</decisions>

<specifics>

## Specific Ideas

- Ownership should be unambiguous for migration to `unique_ptr` at owner and `T*` elsewhere.
- EntityListController only owns entities not referred to elsewhere; use GetNonOwned* for everything else.
- “Non-owning” = raw pointer or index; no second `smart_pointer`/`unique_ptr` to the same object.

</specifics>

<deferred>

## Deferred Ideas

None — discussion stayed within Phase 4 scope.

</deferred>

---

*Phase: 04-critters-generators-fireball-level*  
*Context gathered: 2025-02-23*
