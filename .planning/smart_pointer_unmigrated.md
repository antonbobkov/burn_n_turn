# Types not migrated in Phase 3 (smart pointer → unique_ptr/raw)

Types below are still used with `smart_pointer<T>` and were left for Phase 4 or
other reasons.

| Type | Rationale |
|------|-----------|
| Critter | Phase 4; lifetime and ownership with level/critters. |
| Dragon | Phase 4; shared between LevelController::vDr and Castle::pDrag. |
| Castle | Phase 4; level-owned. |
| Road | Phase 4; level-owned (vRd). |
| FancyCritter | Phase 4; Cutscene holds pCrRun, pCrFollow. |
| Generators (Knight, Princess, Mage, Trader, Skelly) | Phase 4; level-owned. |
| TimedFireballBonus | Phase 4; lsBonusesToCarryOver, Dragon::lsBonuses. |
| ConsumableEntity | Phase 4; lsPpl and consumable hierarchy. |
| FireballBonusAnimation | Phase 4; lsBonus in LevelController. |
| Slime, Sliminess | Phase 4; lsSlimes, lsSliminess. |
| Fireball | Phase 4; Dragon::lsBalls. |
| GameController (and subclasses in vCnt) | Controllers in vCnt; Phase 4 or later. |
| VisualEntity / EventEntity in lsDraw, lsUpdate | Still smart_pointer lists; migration deferred. |

Doc-only (03-03 DOC-01).
