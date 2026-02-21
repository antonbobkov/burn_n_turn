# Phase 3 Plan — Goal-Backward Verification Report

**Verified:** 2025-02-19  
**Plan:** 03-PLAN.md  
**Phase goal (ROADMAP):** Migrate leaf game types: Entity hierarchy, Drawer/
NumberDrawer and concrete drawers, AnimationOnce, StaticImage, Animation,
simple entities. Ownership lives in level, controllers, or critters.

**Success criteria:** (1) Leaf game entities and animations with clear single
owner migrated; SP_Info removed where no smart_pointer<T> remains. (2) Build
and all tests pass. (3) Any type skipped (lifetime unclear) recorded for final
summary.

---

## Summary

**Verdict: Plan is substantially complete and correct.** If an executor
completes all waves and tasks in order, the phase goal and success criteria
(1) and (2) are satisfied. A few gaps and clarifications are listed below;
none block correctness if the executor applies common sense, but addressing
them reduces ambiguity and ensures (3) and cleanup behavior.

---

## Coverage check

### Goal types vs plan

| ROADMAP / must_have type | Covered by task(s) |
|--------------------------|--------------------|
| Entity hierarchy (leaf)   | 3.12 (HitDetection), 3.13 (SP_Info off Entity/subclasses) |
| Drawer / ScalingDrawer   | Wave 1 (1.1–1.5) |
| NumberDrawer            | Wave 2 (2.1–2.4) |
| AnimationOnce           | 3.4 (all creation sites) |
| StaticImage             | 3.2 (pTrial, pBuyNow) |
| Animation               | 3.3 (mSlimes, menu/golem/skeleton/etc.), 3.11 (pMenuCaret) |
| Simple entities         | 3.1–3.11 (StaticRectangle, HighScoreShower, IntroTextShower, BonusScore, AdNumberDrawer, BonusDrawer, TextDrawEntity, TutorialTextEntity, MenuDisplay, SimpleSoundEntity, SoundControls) |

All leaf entity types listed in the plan’s must_haves are assigned to
specific tasks. Phase 4 types (Critter, Dragon, Castle, Road, generators,
TimedFireballBonus, ConsumableEntity containers, etc.) are correctly out of
scope and only referenced as “do not migrate; record for DOC-01.”

### Must_haves vs tasks

- **ScalingDrawer single owner, Draw(ScalingDrawer*):** Enforced by Wave 1
  (TowerDataWrap unique_ptr, DragonGameController raw ptr, Tasks 1.3–1.4
  signatures and implementations, 1.5 SP_Info removal).
- **NumberDrawer single owner, call sites use NumberDrawer*:** Enforced by
  Wave 2 (TowerDataWrap unique_ptr, DragonGameController raw ptr, 2.3
  TextDrawEntity/TutorialTextEntity/MenuDisplay, 2.4 SP_Info removal).
- **Leaf entities single owner; raw in lists:** Enforced by Wave 3 (3.1–3.11)
  via AddOwnedVisualEntity/AddOwnedEventEntity/AddOwnedBoth and raw in
  owned_* (and where applicable lsDraw/lsUpdate).
- **SP_Info removed (CLEAN-01):** 1.5 (Drawer, ScalingDrawer), 2.4
  (NumberDrawer), 3.13 (Entity and migrated subclasses), 4.1 (ImageSequence).
- **Build and tests (VER-01, VER-02):** Required after each task / wave.
- **Skipped types recorded (DOC-01):** Mentioned in plan end (“record any
  skipped types for DOC-01”) but no dedicated task to produce the record.

---

## Gaps and suggestions

### 1. DOC-01 record (success criterion 3)

**Gap:** Success criterion (3) requires “Any type skipped (lifetime unclear)
recorded for final summary.” The plan tells the executor not to migrate
Phase 4 types and to “record any skipped types for DOC-01,” but there is no
explicit task to write that record (e.g. a short list or doc section).

**Suggestion:** Add a small task (e.g. after 3.14 or at end of Wave 3):
“Task 3.15 — Record skipped types for DOC-01: In a short note (e.g. in this
phase folder or in PROJECT.md), list types intentionally not migrated in
Phase 3 (e.g. Critter, Dragon, Castle, Road, generators, TimedFireballBonus,
ConsumableEntity, FancyCritter, …) with one-line rationale (e.g. ‘Phase 4’
or ‘lifetime owned elsewhere’).” This makes success criterion (3) verifiable.

### 2. CleanUp and owned-list sync (Task 3.14)

**Gap:** `EntityListController` has `owned_entities`, `owned_visual_entities`,
and `owned_event_entities`. Ownership is in `owned_entities` (unique_ptr);
the other two hold raw pointers. If CleanUp is run only on
`owned_visual_entities` / `owned_event_entities`, erasing by bExist would
remove raw pointers without releasing the unique_ptrs in `owned_entities`
(leak). If CleanUp is run only on `owned_entities`, the raw lists would hold
dangling pointers.

**Suggestion:** In Task 3.14, state explicitly that CleanUp for owned entities
must erase in sync: when removing an entity with `!bExist`, remove it from
`owned_entities` and from the corresponding entry in
`owned_visual_entities` and/or `owned_event_entities` in a single pass (or
document the chosen sync strategy) so that no raw pointer in owned_* outlives
its unique_ptr.

### 3. “Store raw in lsDraw” wording

**Minor:** Some tasks (e.g. 3.7) say “store raw in lsDraw.” For entities
owned by the controller and added via AddOwnedVisualEntity, they are stored
in `owned_visual_entities` (raw), not in `lsDraw`. `lsDraw` remains used for
non-owned entities (e.g. from level) until Phase 4.

**Suggestion:** In those tasks, clarify: “add via AddOwnedVisualEntity; store
raw in owned_visual_entities (draw list for owned entities)” to avoid
ambiguity about whether to push into both lsDraw and owned_visual_entities.

### 4. AddV / AddE and list types

**Note:** The plan does not require changing `lsDraw` / `lsUpdate` from
`list<smart_pointer<...>>` to `list<...*>` in Phase 3. Migrated leaf entities
are owned via unique_ptr and registered in owned_* lists; non-owned entities
still added via AddV/AddE keep using smart_pointer in lsDraw/lsUpdate until
Phase 4. This is consistent and correct; no change needed.

---

## Ordering and dependencies

- **Wave 2 after Wave 1:** NumberDrawer takes ScalingDrawer* (Task 1.1); Wave
  2 then moves NumberDrawer to unique_ptr. Correct.
- **Wave 3 after Wave 2:** TextDrawEntity, TutorialTextEntity, MenuDisplay
  use NumberDrawer* (Task 2.3); Wave 3 then migrates ownership of those
  entities (3.10, 3.11). Correct.
- **3.11 after 3.3:** pMenuCaret is created in 3.3 and ownership/sharing
  clarified in 3.11. Correct.
- **3.13 after leaf migration:** SP_Info removed from Entity and migrated
  subclasses only after no smart_pointer<Entity> remains for those types.
  Correct.
- **Wave 4 optional:** ImageSequence has no smart_pointer<ImageSequence> in
  codebase; optional SP_Info removal is safe and self-contained.

No missing or broken dependencies identified.

---

## Conclusion

The plan is **complete and correct** for the phase goal and success criteria
(1) and (2). Recommended additions:

1. Add an explicit **DOC-01 task** (e.g. 3.15) so that criterion (3) is
   verifiable.
2. In **Task 3.14**, explicitly require **synced CleanUp** of owned_entities
   with owned_visual_entities/owned_event_entities.
3. Optionally clarify **“store raw in lsDraw”** as “store raw in
   owned_visual_entities” where the entity is controller-owned.

With these, the plan is unambiguous and fully backward-verifiable against
the ROADMAP phase goal and all three success criteria.
