# Phase 4: Critters, generators, fireball, level — Research

**Researched:** 2025-02-24  
**Domain:** Smart-pointer migration for level-owned and dragon-owned entities  
**Confidence:** HIGH (codebase-only; CONTEXT from user)

## Summary

Phase 4 migrates types that own leaf entities: critters (Castle, Dragon, Princess, Knight, Mage, Trader, Slime, Sliminess, etc.), generators (Knight, Princess, Trader, Mage, Skelly), fireball-related types (Fireball, FireballBonusAnimation, TimedFireballBonus, ChainExplosion, KnightOnFire), and level-owned structures (Road, Castle, dragons). Requirements BURN-01/02/03 (single owning site, owner holds unique_ptr, others T*; make_unique where possible), CLEAN-01 (remove SP_Info when no smart_pointer<T> remains), and VER-01/02 (build and tests pass after each step) apply.

Research is grounded in the codebase and the locked decisions in the phase CONTEXT. Key findings: (1) LevelController and Dragon are the main owners; (2) EntityListController must only own entities not referred to elsewhere—all level-owned and dragon-owned entities are exposed via GetNonOwnedUpdateEntities / GetNonOwnedDrawEntities; (3) Fireball ownership moves entirely to Level (Dragon::lsBalls removed); (4) Carry-over bonus handoff must run RecoverBonuses before Clear (fix Dragon ctor clearing before first Update); (5) Generators and dragons become unique_ptr in LevelController and are exposed via GetNonOwned*; Castle::pDrag becomes Dragon*.

**Primary recommendation:** Migrate in bottom-up order: level-owned leaf entities first (Road, then FireballBonusAnimation, Slime, Sliminess), then fireball (Level-owned list, remove Dragon::lsBalls), then TimedFireballBonus (Dragon owns lsBonuses; carry-over handoff fixed), then Dragon (LevelController::vDr owns, Castle::pDrag raw), then generators (LevelController owns unique_ptrs, pGr/pMgGen raw), then critters created by generators (already added via AddBoth/AddE; after generators use GetNonOwned*, these stay as non-owned in lsUpdate/lsDraw or move to owned + GetNonOwned* as decided). Centralize CleanUp(lsBonus) in LevelController.

---

## 1. Where each Phase 4 type lives and current ownership

### 1.1 LevelController (level_controller.h / .cc)

| Type / member | Current type | Owner | Notes |
|---------------|--------------|--------|--------|
| vCs | vector\<unique_ptr\<Castle\>\> | LevelController | Already unique_ptr. |
| vRd | vector\<smart_pointer\<Road\>\> | LevelController | Level-owned; migrate to unique_ptr. |
| vDr | vector\<smart_pointer\<Dragon\>\> | LevelController | Level owns; migrate to unique_ptr; Castle::pDrag → Dragon*. |
| lsBonus | list\<smart_pointer\<FireballBonusAnimation\>\> | LevelController | Only owner; expose via GetNonOwned*; centralize CleanUp(lsBonus) here. |
| lsSlimes | list\<smart_pointer\<Slime\>\> | LevelController | Level-owned; migrate to list\<unique_ptr\<Slime\>\> or similar. |
| lsSliminess | list\<smart_pointer\<Sliminess\>\> | LevelController | Same. |
| pGr | KnightGenerator* | — | Non-owning; stays raw. After migration, points into LevelController-owned generator. |
| pMgGen | MageGenerator* | — | Non-owning; same. |
| Generators (Knight, Princess, Trader, Mage) | smart_pointer in Init | LevelController | Created in Init() with make_smart, AddE(gen); migrate to unique_ptr stored in LevelController, expose via GetNonOwnedUpdateEntities; no AddE. |

**GetNonOwnedUpdateEntities / GetNonOwnedDrawEntities (level_controller.cc 406–423):** Currently return vCs[i].get(), pTutorialText.get(), pSc.get(). After migration they must also return: vDr[i].get(), generator raw ptrs, lsBonus (raw ptrs), fireballs (Level-owned list, raw ptrs), lsSlimes, lsSliminess (raw ptrs). EntityListController::Update() already iterates GetNonOwned* alongside lsUpdate/lsDraw, so no AddE/AddBoth for these once they are owned in LevelController.

### 1.2 Dragon (dragon.h / .cc)

| Type / member | Current type | Owner | Notes |
|---------------|--------------|--------|--------|
| lsBonuses | list\<smart_pointer\<TimedFireballBonus\>\> | Dragon | Dragon owns; LevelController returns ptrs via GetNonOwned* (no separate lsUpdate for bonuses). |
| lsBalls | list\<smart_pointer\<Fireball\>\> | Dragon | **Delete entirely.** Level owns all fireballs; Dragon no longer keeps a list. |

**FindDragon (level_controller.cc 144–148):** Returns smart_pointer\<Dragon\> for the dragon matching Dragon* p. After vDr becomes vector\<unique_ptr\<Dragon\>\>, return raw Dragon* or optional reference; call sites (Castle::pDrag, etc.) become Dragon*.

### 1.3 Castle (critters.h / .cc)

| Type / member | Current type | Owner | Notes |
|---------------|--------------|--------|--------|
| pDrag | smart_pointer\<Dragon\> | Non-owning | Becomes Dragon*; LevelController::vDr owns. |

Castle is already in vCs as unique_ptr; no change to Castle ownership.

### 1.4 Fireball and related (fireball.h / .cc, dragon.cc)

| Type | Created / held where | After migration |
|------|----------------------|------------------|
| Fireball | Dragon::Fire() → make_smart(new Fireball(...)); pAd->AddBoth(pFb); lsBalls.push_back(pFb) (middle shot only) | Level owns a single list (e.g. list\<unique_ptr\<Fireball\>\> or vector); Dragon::Fire() creates with make_unique and pushes to Level’s list; AddBoth replaced by “add to Level list” and expose via GetNonOwned*. |
| CircularFireball | dragon.cc ~120: make_smart(new CircularFireball(...)); pAd->AddBoth(pFb) | Same: Level owns; create and add to Level’s fireball (or general entity) list. |
| ChainExplosion | fireball.cc: make_smart(new ChainExplosion(...)); pBc->AddBoth(pCE) | LevelController is pBc for level; can be owned in same Level-owned list or AddOwnedBoth as now; CONTEXT says Level owns all fireballs, so chain explosions likely in same Level-owned list. |
| KnightOnFire | fireball.cc, dragon.cc: make_smart(new KnightOnFire(...)); pBc->AddBoth(pKn) | Same: Level-owned list, expose via GetNonOwned*. |
| FireballBonusAnimation | critters.cc: make_smart(new FireballBonusAnimation(...)); pAc->lsBonus.push_back(pFb); pAc->AddBoth(pFb) | LevelController::lsBonus only owner; change to list\<unique_ptr\<FireballBonusAnimation\>\>; push_back(std::move(...)); expose via GetNonOwned*; stop AddBoth(pFb) (bonuses only in lsBonus and GetNonOwned*). |
| TimedFireballBonus | Dragon::lsBonuses; Dragon::AddBonus(); RecoverBonuses from carry-over | Dragon keeps owning lsBonuses (unique_ptr list); expose via GetNonOwned* for Update; carry-over: handoff by move on first Update (RecoverBonuses then Clear); Dragon ctor must not clear before RecoverBonuses. |

### 1.5 Generators (critter_generators.h / .cc, level_controller.cc)

| Type | Created where | Current | After migration |
|------|----------------|--------|------------------|
| KnightGenerator | level_controller.cc Init: make_smart(new KnightGenerator(...)); pGr = pGen.get(); AddE(pGen) | smart_pointer, added to lsUpdate | LevelController holds e.g. unique_ptr\<KnightGenerator\>; pGr = gen.get(); do not AddE; include in GetNonOwnedUpdateEntities. |
| PrincessGenerator | Same pattern | Same | Same. |
| TraderGenerator | Same | Same | Same. |
| MageGenerator | Same; pMgGen = pMGen.get() | Same | Same. |
| SkellyGenerator | critters.cc SummonSkeletons: make_smart(new SkellyGenerator(...)); pAc->AddE(pSkel) | Not level-owned; short-lived, added to lsUpdate | EntityListController only owns entities not referred elsewhere; SkellyGenerator can stay AddE (controller “owns” for lifetime) or be given to LevelController; CONTEXT says “Generators: LevelController owns” for the four level generators; Skelly is created by SummonSkeletons and added with AddE—treat as controller-owned (AddE) unless we move to a Level-owned “temp generators” list. |

### 1.6 LevelLayout (level.h)

LevelLayout is a value type (Rectangle, vectors of Point/Road, etc.). No smart pointers; no migration of the type. Optional single vector owner if desired (already passed by value/ref).

### 1.7 Other critters (Princess, Knight, Mage, Trader, Slime, MegaSlime, etc.)

Created in generators or critters.cc and added via AddBoth(pCr) / AddE(...). They are in lsUpdate/lsDraw/lsPpl. After generators and Level-owned lists use GetNonOwned*, these remain in lsPpl/lsUpdate/lsDraw as non-owned (raw ptrs from generator/level logic) or are owned in LevelController and exposed via GetNonOwned*; CONTEXT says “EntityListController only owns entities not referred elsewhere; use GetNonOwned* for everything else,” so critters spawned by level/generators are “referred elsewhere” (level/generators) and should not be double-owned in EntityListController. So: generators create critters and pass them into the level’s owned or “non-owned” pool; LevelController exposes them via GetNonOwned* and does not AddBoth them into lsDraw/lsUpdate. Current code does AddBoth(pCr) from generator code (e.g. critter_generators.cc), so the list that “owns” is currently the controller’s lsUpdate/lsDraw. Migration can either: (a) have LevelController own a list of critters and generators push into it (unique_ptr), then expose via GetNonOwned*, or (b) keep AddE/AddBoth but with raw pointers and a single owner list elsewhere. CONTEXT says “expose via GetNonOwnedUpdateEntities” for generators and “EntityListController only owns entities not referred elsewhere,” so the pattern is: LevelController owns generators and level-owned entities; GetNonOwned* exposes them; EntityListController’s lsUpdate/lsDraw are for things only the controller owns. So spawned critters (Knight, Princess, etc.) are either owned in LevelController (e.g. lsPpl-like list as unique_ptr) and exposed via GetNonOwned*, or they remain in lsPpl but with ownership clarified. Unmigrated doc (smart_pointer_unmigrated.md) lists “ConsumableEntity (lsPpl)” for Phase 4; lsPpl is currently list\<smart_pointer\<ConsumableEntity\>\>. So migration of ConsumableEntity/critters implies LevelController (or one owner) holds unique_ptrs and exposes raw ptrs; GetNonOwned* can include those, and lsPpl could become a list of raw pointers for iteration if ownership is in LevelController.

---

## 2. Order of migration (respecting CONTEXT and bottom-up)

1. **Road / FancyRoad** — Level-owned (vRd). Migrate to vector\<unique_ptr\<Road\>\>; update level_controller Init (vRd.push_back(make_unique<FancyRoad>(...))); AddV(vRd[i]) → remove, add vRd to GetNonOwnedDrawEntities.  
2. **FireballBonusAnimation** — LevelController::lsBonus only owner. Migrate lsBonus to list\<unique_ptr\<FireballBonusAnimation\>\>; centralize CleanUp(lsBonus) in LevelController::Update(); expose via GetNonOwnedUpdateEntities/GetNonOwnedDrawEntities; stop AddBoth(pFb) at creation (critters.cc), only push to lsBonus.  
3. **Slime / Sliminess** — LevelController::lsSlimes, lsSliminess. Migrate to list\<unique_ptr\<Slime\>\>, list\<unique_ptr\<Sliminess\>\>; CleanUp already in LevelController::Update(); expose via GetNonOwned*.  
4. **Fireball (and ChainExplosion, KnightOnFire)** — Level owns all. Add e.g. list\<unique_ptr\<Fireball\>\> (or a common base) in LevelController; Dragon::Fire() and fireball.cc create with make_unique and append to Level’s list; remove Dragon::lsBalls and CleanUp(lsBalls); expose fireballs (and chain/knight-on-fire) via GetNonOwned*.  
5. **TimedFireballBonus** — Dragon::lsBonuses owns. Migrate to list\<unique_ptr\<TimedFireballBonus\>\>; RecoverBonuses/Clear order: first Update call RecoverBonuses then ClearBonusesToCarryOver; remove ClearBonusesToCarryOver from Dragon ctor so list is not cleared before RecoverBonuses. Expose via GetNonOwned* (LevelController returns Dragon’s bonus ptrs).  
6. **Dragon** — LevelController::vDr owns. vDr → vector\<unique_ptr\<Dragon\>\>; Castle::pDrag → Dragon*; FindDragon returns Dragon*; all pDrag assignments and checks use raw ptr; remove lsBalls.  
7. **Generators (Knight, Princess, Trader, Mage)** — LevelController owns. Store as unique_ptrs (e.g. one per type or vector\<unique_ptr\<EventEntity\>\>); pGr, pMgGen stay raw; remove AddE(gen); add generator .get() to GetNonOwnedUpdateEntities (and draw if needed).  
8. **SkellyGenerator** — Short-lived; created in SummonSkeletons, AddE(pSkel). Keep controller-owned (AddE with unique_ptr) or move to LevelController-owned “temp” list; either way, single owner.  
9. **Critters (Princess, Knight, Mage, Trader, Slime, MegaSlime, etc.) and ConsumableEntity in lsPpl** — Bottom-up: these are created by generators or level logic and currently in lsPpl/lsUpdate/lsDraw. After generators and Level use GetNonOwned*, decide single owner (LevelController or controller list) and migrate lsPpl to unique_ptr at owner and raw ptrs in GetNonOwned* / iteration.

---

## 3. Concrete call sites that must change

### 3.1 Dragon / Castle (vDr owns; pDrag non-owning)

- **level_controller.h:** `std::vector<smart_pointer<Dragon>> vDr` → `std::vector<std::unique_ptr<Dragon>> vDr`.  
- **level_controller.cc:**  
  - Init: `vDr.push_back(make_smart(new Dragon(...)))` → `vDr.push_back(std::make_unique<Dragon>(...))`; `vDr.back()->pCs->pDrag = vDr.back()` → `vDr.back()->pCs->pDrag = vDr.back().get()`.  
  - FindDragon: return type `smart_pointer<Dragon>` → `Dragon*`; return `vDr[i].get()` (or nullptr).  
  - All `vDr[i]` used as ref → `vDr[i].get()` where a pointer is needed, or keep `vDr[i]` for unique_ptr access.  
- **critters.h:** `smart_pointer<Dragon> pDrag` → `Dragon* pDrag`.  
- **critters.cc:**  
  - Castle land/takeoff: `pDrag = smart_pointer<Dragon>()` → `pDrag = nullptr`; `if (!pDrag.is_null())` → `if (pDrag != nullptr)`; `pDrag->...` unchanged.  
- **dragon.cc:**  
  - `pCs->pDrag = pAd->FindDragon(this)` → `pCs->pDrag = pAd->FindDragon(this)` (FindDragon returns Dragon*).  
  - `pCs->pDrag = smart_pointer<Dragon>()` → `pCs->pDrag = nullptr`.  
  - `if (!pAd->vCs[i]->pDrag.is_null())` → `if (pAd->vCs[i]->pDrag != nullptr)`.  
  - `vDr.back()->pCs->pDrag = vDr.back()` (in level_controller) → already above.

### 3.2 Fireball: Level owns all; delete Dragon::lsBalls

- **dragon.h:** Remove `std::list<smart_pointer<Fireball>> lsBalls`.  
- **dragon.cc:**  
  - Remove `CleanUp(lsBalls)` and all `lsBalls.push_back(pFb)`.  
  - Dragon::Fire(): create fireball with `std::make_unique<Fireball>(...)`, pass to LevelController (e.g. `pAd->AddFireball(std::move(pFb))`); LevelController stores in its list and exposes via GetNonOwned*.  
- **level_controller:** Add fireball list (e.g. `std::list<std::unique_ptr<Fireball>>` or shared base); add `AddFireball(std::unique_ptr<Fireball>)`; in GetNonOwnedUpdateEntities/GetNonOwnedDrawEntities push raw ptrs from that list; CleanUp that list in Update (remove !bExist).  
- **fireball.cc:** Where Fireball/KnightOnFire/ChainExplosion are created and pBc->AddBoth(pFb) used: create with make_unique, call LevelController method to take ownership (pBc is LevelController* when in level), or keep AddBoth but with a variant that takes unique_ptr and moves into Level’s list.

### 3.3 FireballBonusAnimation: LevelController::lsBonus only owner; GetNonOwned*

- **level_controller.h:** `std::list<smart_pointer<FireballBonusAnimation>> lsBonus` → `std::list<std::unique_ptr<FireballBonusAnimation>> lsBonus`.  
- **level_controller.cc:**  
  - GetNonOwnedUpdateEntities / GetNonOwnedDrawEntities: append raw ptrs from lsBonus.  
  - Update: single `CleanUp(lsBonus)` here (remove other CleanUp(lsBonus) from dragon.cc, critters.cc).  
- **critters.cc:** Where `pAc->lsBonus.push_back(pFb)` and `pAc->AddBoth(pFb)`: create with make_unique, push `pAc->AddBonusAnimation(std::move(pFb))` (LevelController takes ownership); do not AddBoth.  
- **dragon.cc:** Remove `CleanUp(pAd->lsBonus)` and loop over `pAd->lsBonus`; use GetNonOwned* or a direct accessor for drawing/update if needed, or rely on LevelController Update to iterate bonuses.  
- **critters.cc:** CleanUp(pAc->lsBonus) and iteration over pAc->lsBonus: remove; LevelController centralizes CleanUp and iteration via GetNonOwned*.

### 3.4 TimedFireballBonus: Dragon owns lsBonuses; GetNonOwned*; carry-over handoff

- **dragon.h:** `std::list<smart_pointer<TimedFireballBonus>> lsBonuses` → `std::list<std::unique_ptr<TimedFireballBonus>> lsBonuses`.  
- **dragon.cc:** AddBonus: take unique_ptr or move from carry-over; RecoverBonuses: from GetBonusesToCarryOver(), move into lsBonuses (handoff by move on first Update).  
- **Dragon ctor:** Remove `pAd->pGl->ClearBonusesToCarryOver();` so the list is not cleared before first Update.  
- **level_controller.cc Update (bFirstUpdate):** Keep `vDr[0]->RecoverBonuses();` then `pGl->ClearBonusesToCarryOver();` so RecoverBonuses runs before Clear.  
- **LevelController:** Expose Dragon’s TimedFireballBonus ptrs in GetNonOwnedUpdateEntities (e.g. iterate vDr, then for each dragon iterate lsBonuses and push .get()).

### 3.5 Generators: LevelController owns; expose via GetNonOwned*

- **level_controller.cc Init:**  
  - `smart_pointer<KnightGenerator> pGen = make_smart(new KnightGenerator(...))` → `auto pGen = std::make_unique<KnightGenerator>(...)`; store in member (e.g. `std::unique_ptr<KnightGenerator> pKnightGen`); `pGr = pGen.get()`; do not call AddE(pGen). Same for Princess, Trader, Mage.  
- **level_controller.h:** Add members for generator ownership (e.g. unique_ptr\<KnightGenerator\>, etc.); keep pGr, pMgGen as raw pointers.  
- **GetNonOwnedUpdateEntities:** Append pGr, pMgGen, and the other two generators’ .get().

### 3.6 Carry-over bonuses: RecoverBonuses then Clear; fix Dragon ctor

- **dragon.cc Dragon ctor:** Remove the line `pAd->pGl->ClearBonusesToCarryOver();`.  
- **level_controller.cc Update (bFirstUpdate block):** Keep order: `vDr[0]->RecoverBonuses();` then `pGl->ClearBonusesToCarryOver();` so bonuses are recovered before clear.  
- **DragonGameController:** GetBonusesToCarryOver and AddBonusToCarryOver stay; type can migrate to unique_ptr for the list element type when TimedFireballBonus migrates.

---

## 4. Risks and dependencies

### 4.1 CleanUp patterns

- **entities.h:** `CleanUp(std::list<T>&)` requires `(*itr)->bExist`. Works for `list<unique_ptr<Entity>>` and `list<Raw*>` (owned_visual_entities, owned_event_entities). For `list<unique_ptr<Fireball>>` (or similar), same template works.  
- **Order in EntityListController::Update():** Clean raw-pointer lists and owned_* before owned_entities so no raw ptr outlives its unique_ptr. LevelController’s Update must CleanUp(lsBonus), lsSlimes, lsSliminess, and any Level-owned fireball list before or in sync with base Update; centralizing CleanUp(lsBonus) in LevelController avoids double-CleanUp and keeps lifetime in one place.

### 4.2 AddBoth / AddE vs AddOwned* / GetNonOwned*

- After migration, EntityListController **only** owns entities not referred to elsewhere (e.g. some UI, one-off animations). Level-owned and dragon-owned entities are **not** added with AddE/AddBoth; they are stored in LevelController (and Dragon) and exposed via GetNonOwnedUpdateEntities / GetNonOwnedDrawEntities.  
- Init() must stop calling AddE(gen) and AddBoth(vDr[i]) for those; instead, add their raw ptrs in GetNonOwned*.

### 4.3 FindDragon and pDrag

- FindDragon currently returns smart_pointer\<Dragon\>; call sites assign to Castle::pDrag or compare .is_null(). After migration, FindDragon returns Dragon*; all call sites use raw ptr (nullptr checks, assignment to pDrag).

### 4.4 Fireball creation in fireball.cc (ChainExplosion, KnightOnFire)

- pBc is EntityListController*; in level context it is LevelController*. So when Level owns fireballs, ChainExplosion and KnightOnFire creation should hand unique_ptr to LevelController (e.g. AddFireball, AddChainExplosion, AddKnightOnFire) instead of AddBoth(smart_pointer). Same for Fireball created in Fireball::Update (fireball.cc ~289).

### 4.5 Sliminess / Sliminess::pSlm

- Sliminess has `smart_pointer<AnimationOnce> pSlm` (critters.h). That is a leaf entity (Phase 3); if already migrated, pSlm may be unique_ptr or raw. Phase 4 migration of Sliminess ownership (LevelController::lsSliminess) is independent; pSlm stays as per Phase 3.

### 4.6 CleanUp(lsBonus) centralization

- Currently CleanUp(pAd->lsBonus) in dragon.cc (Dragon::Update) and CleanUp(pAc->lsBonus) in critters.cc (Knight::Update, Mage::Update). CONTEXT: “centralize CleanUp(lsBonus) in LevelController.” So move all CleanUp(lsBonus) to LevelController::Update() and remove from dragon.cc and critters.cc.

---

## 5. Don’t hand-roll

| Problem | Don’t build | Use instead |
|---------|-------------|-------------|
| Reference counting for level-owned entities | Custom refcount or shared ownership | single unique_ptr at LevelController/Dragon, raw ptrs elsewhere |
| Custom “non-owned list” type | New container type | GetNonOwnedUpdateEntities / GetNonOwnedDrawEntities returning vector\<T*\> |
| Double ownership of same entity | AddBoth(gen) and a separate owner | One owner (LevelController); GetNonOwned* for iteration |

---

## 6. Common pitfalls

- **Clearing carry-over before RecoverBonuses:** Dragon ctor currently clears carry-over; first Update then calls RecoverBonuses. That leaves RecoverBonuses with an empty list. Fix: remove Clear from Dragon ctor; keep RecoverBonuses then Clear in LevelController’s first Update.  
- **Forgetting to expose in GetNonOwned*:** After moving dragons, generators, lsBonus, fireballs to unique_ptr in LevelController, every type that must Update/Draw must be included in GetNonOwnedUpdateEntities / GetNonOwnedDrawEntities or they will not be ticked/drawn.  
- **Keeping Dragon::lsBalls:** CONTEXT says delete it entirely; all fireballs owned by Level.  
- **Leaving multiple CleanUp(lsBonus) call sites:** Centralize in LevelController::Update() only.

---

## 7. Sources

- **Primary:** codebase grep/read: level_controller.h/cc, dragon.h/cc, critters.h/cc, fireball.h/cc, basic_controllers.h/cc, entities.h, critter_generators.h/cc, level.h, dragon_game_controller.h/cc, smart_pointer_unmigrated.md.  
- **Phase context:** User-provided CONTEXT (04-CONTEXT.md content in the task): EntityListController ownership rule, Dragon/Castle ownership, Fireball Level ownership, FireballBonusAnimation, TimedFireballBonus, LevelLayout, carry-over handoff, generators.

---

## RESEARCH COMPLETE
