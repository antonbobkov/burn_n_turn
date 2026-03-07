# Phase 5: Ownership options per object

**Purpose:** Outline who can own each object in the controller/runner layer so
decisions can be made before planning. Phase scope: GameController
implementations, DragonGameRunner, game_runner_interface_impl; "controllers
own view entities; runner owns current controller."

**Current state (brief):** Runner holds `TowerDataWrap*` (raw; Runner creates
and deletes it). TowerDataWrap holds `unique_ptr<DragonGameController>` and
owns drawers, config, file path, level layouts. DGC holds
`vector<smart_pointer<GameController>>` and `smart_pointer<MenuController>`.
Entry points: main uses `GetGameRunner(pe)` → `smart_pointer<GameRunner>`;
simulation_test constructs `DragonGameRunner` directly via `make_smart`.

---

## 1. DragonGameRunner

**What it is:** The GameRunner implementation; owns the "tower" (game session)
and delegates input/update to the active controller.

| Option | Owner | Notes |
|--------|--------|--------|
| A | Entry point (main, simulation_test) | Caller holds `unique_ptr<GameRunner>`. GetGameRunner returns unique_ptr. Runner created once, lives until process/test end. |
| B | Factory / program engine | Some central object creates and holds the runner. Entry just calls Run() and has no pointer. |
| C | (Current) smart_pointer at entry | GetGameRunner returns smart_pointer<GameRunner>; main/simulation hold smart_pointer. Migrate to unique_ptr at same call sites. |

**Recommendation:** A or C — single owner at entry; migrate to unique_ptr
(Phase 6 entry-point goal). B is a larger refactor.

**Selected: Option A** — Entry point holds `unique_ptr<GameRunner>`; GetGameRunner returns `unique_ptr<GameRunner>`.

---

## 2. TowerDataWrap

**What it is:** "The tower's chest" — holds exit event, graphics/sound
pointers, drawers (pDr, pBigDr, pNum, pBigNum, pFancyNum), FileManager/config
paths, level layouts, and `unique_ptr<DragonGameController>`.

| Option | Owner | Notes |
|--------|--------|--------|
| A | DragonGameRunner | Runner owns TowerDataWrap (unique_ptr). Current behavior is Runner new/delete; make it explicit. |
| B | Inline into DragonGameRunner | Merge TowerDataWrap members into DragonGameRunner; no separate type. Simplifies object graph. |
| C | Separate wrapper owned by entry | Entry owns both runner and a "tower data" object and passes refs. Unusual for current design. |

**Recommendation:** A (Runner owns unique_ptr<TowerDataWrap>) or B (merge into
Runner). Current raw pData fits A with minimal change.

**Selected: Option B** — TowerDataWrap is merged into DragonGameRunner; its members become direct members of the runner class.

---

## 3. DragonGameController (DGC)

**What it is:** "The great hall" — current score, high score, settings,
bounds, graphics/sound refs, level layouts, controller list, menu controller,
preloader, bonuses to carry over.

| Option | Owner | Notes |
|--------|--------|--------|
| A | TowerDataWrap (hence Runner) | Already so: TowerDataWrap::pCnt is unique_ptr<DragonGameController>. Keep; DGC is the single game session. |
| B | Runner directly | If TowerDataWrap is merged into Runner, Runner holds unique_ptr<DragonGameController>. |
| C | Entry point | Entry would own DGC and pass it to Runner. Unusual; Runner would need to accept non-owning ref. |

**Recommendation:** A or B — single owner above DGC (Runner or its data wrap).
No shared ownership.

**Selected: Option B** — Because TowerDataWrap is merged into Runner (decision 2B), Runner directly holds `unique_ptr<DragonGameController>`.

---

## 4. Controller list (vCnt) and active controller

**What it is:** DGC::vCnt is `vector<smart_pointer<GameController>>`; nActive
indexes the current screen (menu, level, start, cutscene, score, buy, etc.).

| Option | Owner | Notes |
|--------|--------|--------|
| A | DGC owns all in vector<unique_ptr<GameController>> | All controllers created for the session live in vCnt; active is an index. No controller destroyed on screen change. |
| B | DGC holds one unique_ptr<GameController> | Only the active controller exists; on transition, destroy current and make_unique the next. No vector. |
| C | DGC owns vector; some entries can be null / lazy-created | Hybrid: e.g. menu and level created on first use, rest created when needed, stored in vector. |

**Recommendation:** A matches current "many controllers alive" design and
minimal behavior change. B simplifies ownership but changes lifecycle (recreate
on each transition). C is possible if we want lazy creation without keeping
every screen alive.

**Selected: Option A** — DGC owns `vector<unique_ptr<GameController>>`; active controller identified by index.

---

## 5. MenuController (pMenu)

**What it is:** The menu screen; DGC holds `smart_pointer<MenuController>`
and pushes it onto vCnt when entering menu.

| Option | Owner | Notes |
|--------|--------|--------|
| A | DGC only (unique_ptr<MenuController>) | pMenu is the single owner; when added to vCnt, vCnt holds the same controller (e.g. by raw pointer or by moving into a slot). Need a clear rule: pMenu vs vCnt. |
| B | DGC::vCnt only | MenuController is created and pushed into vCnt; no separate pMenu. DGC gets menu via "find controller by name" or similar. |
| C | DGC holds unique_ptr; vCnt stores raw pointers | pMenu unique_ptr in DGC; vCnt is vector<GameController*> (non-owning). Other controllers owned by vCnt (unique_ptr) or by other fields. |

**Recommendation:** Decide whether menu is "special" (pMenu) or "one of the
stack" (only in vCnt). A/C: keep single unique_ptr<MenuController> in DGC;
vCnt either does not own menu or holds raw ptr to it. B: no pMenu; create
MenuController when entering menu and push into vCnt (then need to track which
element is menu for ExitMenuResume).

**Selected: Option B modified** — `pMenu` is kept as a convenience member on
DGC but is a **raw (non-owning) pointer** (`MenuController*`). Ownership lives
exclusively in `vCnt` (`vector<unique_ptr<GameController>>`; decision 4A). When
MenuController is created it is pushed into `vCnt` as a `unique_ptr`; `pMenu`
is then set to point at that same object. This gives DGC typed access to the
menu without a second owner. `pMenu` must be cleared (set to `nullptr`) if the
menu entry is ever removed from `vCnt`.

---

## 6. View entities owned by controllers (EntityListController)

**What it is:** EntityListController has lsDraw, lsUpdate, lsPpl
(smart_pointer) and owned_entities / owned_visual_entities /
owned_event_entities (unique_ptr / raw). Subclasses (Menu, Level, etc.) add
their own (e.g. MenuDisplay, level entities).

| Option | Owner | Notes |
|--------|--------|--------|
| A | Each controller owns its view entities | Controller holds unique_ptr (or list<unique_ptr>) for what it creates; lsDraw/lsUpdate/lsPpl become non-owning (raw or refs). Matches "controllers own view entities." |
| B | DGC owns all view entities | Controllers get raw pointers; DGC holds central lists. Big refactor; not aligned with roadmap. |
| C | Hybrid: controller owns "its" entities; shared refs for shared UI | E.g. NumberDrawer is shared (DGC/TowerDataWrap); MenuDisplay is MenuController. Already partly so; clarify and finish migration. |

**Recommendation:** A/C — controller owns the entities that are specific to
its screen (MenuDisplay, level critters, etc.); shared resources (drawers,
fonts) owned above and passed as raw ptr. Migrate remaining smart_pointer in
EntityListController to unique_ptr where controller is sole owner.

**Selected: No change in this pass** — view entity ownership inside controllers
is out of scope for Phase 5. Keep as-is.

---

## 7. Shared resources (drawers, fonts, config)

**What it is:** ScalingDrawer (pDr, pBigDr), NumberDrawer (pNum, pBigNum),
FontWriter (pFancyNum), ConfigurationFile, FilePath, LevelLayouts. Currently
owned by TowerDataWrap; DGC and controllers use raw pointers.

| Option | Owner | Notes |
|--------|--------|--------|
| A | TowerDataWrap (or Runner if merged) | Single owner; DGC and all controllers receive raw pointers. No change of owner; just confirm and document. |
| B | DGC | DGC would own drawers/config; TowerDataWrap would pass ownership to DGC at construction. DGC already "uses" them; ownership in DGC is possible. |
| C | Split (e.g. config at entry, drawers in Runner) | More granular ownership; possible but more complex. |

**Recommendation:** A — keep ownership in TowerDataWrap (or Runner). DGC and
controllers stay non-owning. Easiest and matches current layout.

**Selected: Option A** — Runner (after merging TowerDataWrap, decision 2B) owns drawers, config, and fonts. DGC and all controllers receive raw pointers.

---

## 8. GameRunner interface and GetGameRunner

**What it is:** GameRunner is the interface; DragonGameRunner is the
implementation. GetGameRunner(ProgramEngine) returns smart_pointer<GameRunner>;
callers (main, tests) hold that.

| Option | Owner | Notes |
|--------|--------|--------|
| A | Entry point holds unique_ptr<GameRunner> | GetGameRunner returns unique_ptr<GameRunner>; main/simulation_test take ownership. Phase 6 aligns with "entry holds unique_ptr at top level." |
| B | GetGameRunner returns unique_ptr; caller may wrap in shared for legacy | If something else needed shared ownership (unlikely), caller could move into shared_ptr. Default: unique_ptr at entry. |
| C | Keep smart_pointer return for Phase 5; migrate in Phase 6 | Phase 5 only migrates controller/runner internals; GetGameRunner and entry ownership in Phase 6. |

**Recommendation:** A for Phase 6; Phase 5 can leave GetGameRunner signature as-is (smart_pointer) and migrate internals only, or switch to unique_ptr in Phase 5 if entry-point call sites are touched anyway.

**Selected: Option A** — GetGameRunner returns `unique_ptr<GameRunner>`; entry points (main, simulation_test) take ownership.

---

## 9. EntityListController::lsDraw, lsUpdate, lsPpl

**What it is:** Lists of VisualEntity, EventEntity, ConsumableEntity; currently
smart_pointer. Some entities are owned in owned_entities and exposed via
owned_visual_entities / owned_event_entities; others may be shared or
borrowed.

| Option | Owner | Notes |
|--------|--------|--------|
| A | Controller owns all in these lists | Replace with list<unique_ptr<...>> or store in owned_* and keep ls* as raw ptr lists. Single owner per entity. |
| B | Controller owns only "owned_*"; ls* are non-owning | owned_entities / AddOwned* already exist; migrate to unique_ptr there; lsDraw/lsUpdate/lsPpl become raw pointers (or refs) to same objects. No double ownership. |
| C | Mixed: some owned by controller, some by DGC or level | Clarify per-entity: e.g. level entities owned by LevelController; shared HUD by DGC. Document and implement consistently. |

**Recommendation:** B — controller owns via owned_* (unique_ptr); ls* are
non-owning lists for iteration. Matches Phase 3 direction (GetNonOwned*,
AddOwned*). Remove smart_pointer from lsDraw/lsUpdate/lsPpl.

**Selected: No change in this pass** — lsDraw/lsUpdate/lsPpl migration is out of scope for Phase 5. Keep as-is.

---

## 10. Cutscene::pCrRun, pCrFollow (FancyCritter)

**What it is:** Two critters used only in Cutscene; currently
smart_pointer<FancyCritter>.

| Option | Owner | Notes |
|--------|--------|--------|
| A | Cutscene (EntityListController) | unique_ptr<FancyCritter> x2; or add to owned_entities and use GetNonOwned* for update/draw. |
| B | In owned_entities / owned_visual_entities / owned_event_entities | Same as A but via the generic owned_* mechanism so Cutscene doesn't need dedicated members. |

**Recommendation:** A or B — Cutscene owns both; migrate to unique_ptr (or
owned_*). No shared ownership.

**Selected: Option A** — `pCrRun` and `pCrFollow` become `unique_ptr<FancyCritter>` members directly on Cutscene. They are registered into the update and draw lists via `GetNonOwned*` (raw pointers).

---

## 11. Preloader (DGC::pr)

**What it is:** DGC holds std::unique_ptr<Preloader>; used for loading assets.

| Option | Owner | Notes |
|--------|--------|--------|
| A | DGC | Keep unique_ptr<Preloader> in DGC. Already unique_ptr; no change. |
| B | TowerDataWrap / Runner | Move preloader up if it's considered "tower-wide" rather than "game session." |

**Recommendation:** A — DGC keeps owning Preloader unless we have a reason to
move it (e.g. reuse across sessions).

---

## 12. SoundInterfaceProxy (DGC::pSnd)

**What it is:** DGC holds std::unique_ptr<SoundInterfaceProxy>; wraps sound
for mute/music control.

| Option | Owner | Notes |
|--------|--------|--------|
| A | DGC | Keep unique_ptr<SoundInterfaceProxy> in DGC. Already unique_ptr. |
| B | TowerDataWrap / Runner | If we want one proxy per "tower" and pass to DGC as raw ptr. |

**Recommendation:** A unless we centralize sound state in Runner/TowerDataWrap.

---

## Summary table (final decisions)

| # | Object(s) | Decision | Owner / mechanism |
|---|-----------|----------|-------------------|
| 1 | DragonGameRunner | **A** | Entry (main / test) holds `unique_ptr<GameRunner>`; `GetGameRunner` returns `unique_ptr<GameRunner>`. |
| 2 | TowerDataWrap | **B** | Merged into DragonGameRunner; no separate type. |
| 3 | DragonGameController | **B** | Runner holds `unique_ptr<DragonGameController>` directly (follows from 2B). |
| 4 | vCnt (controller list) | **A** | DGC owns `vector<unique_ptr<GameController>>`; active = index. |
| 5 | MenuController (pMenu) | **B modified** | vCnt owns the object (`unique_ptr`); `pMenu` is a raw non-owning `MenuController*` on DGC for typed access. Must be kept in sync with vCnt. |
| 6 | View entities per controller | **no change** | Out of scope for Phase 5; keep as-is. |
| 7 | Shared resources (drawers, config) | **A** | Runner (merged from TowerDataWrap) owns drawers, config, fonts. DGC and controllers get raw pointers. |
| 8 | GetGameRunner / GameRunner interface | **A** | `GetGameRunner` returns `unique_ptr<GameRunner>`; entry holds ownership. |
| 9 | lsDraw / lsUpdate / lsPpl | **no change** | Out of scope for Phase 5; keep as-is. |
| 10 | Cutscene pCrRun / pCrFollow | **A** | `unique_ptr<FancyCritter>` members on Cutscene; registered into update/draw lists via `GetNonOwned*` (non-owning). |
| 11 | Preloader | **A** | DGC keeps `unique_ptr<Preloader>`; no change. |
| 12 | SoundInterfaceProxy | **A** | DGC keeps `unique_ptr<SoundInterfaceProxy>`; no change. |

---

*Phase: 05-controllers-and-runner*
*Document: ownership options for planning and CONTEXT*
