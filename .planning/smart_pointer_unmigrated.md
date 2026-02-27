# Types not migrated (smart pointer → unique_ptr/raw)

## Phase 4 complete

Phase 4 migrated: Dragon (vDr unique_ptr, Castle::pDrag raw), TimedFireballBonus
(unique_ptr at Dragon and DGC), Fireball/ChainExplosion/KnightOnFire/CircularFireball
(level-owned via AddOwnedBoth), FireballBonusAnimation, Slime/Sliminess/MegaSlime/MegaSliminess
(level-owned), generators (LevelController-owned; SkellyGenerator via AddSpawnedGenerator).
SP_Info removed from FireballBonus (no smart_pointer<FireballBonus> in codebase).

## Types still using smart_pointer (post–Phase 4)

| Type | Where / rationale |
|------|-------------------|
| ConsumableEntity | EntityListController::lsPpl; consumables (Princess, Knight, Trader, Mage, etc.) created by generators and added via AddE/AddBoth. Single owner is lsPpl; migration to unique_ptr deferred to Phase 5+. |
| VisualEntity / EventEntity | EntityListController::lsDraw, lsUpdate; base lists still smart_pointer; migration deferred. |
| GameController (and subclasses) | DragonGameController::vCnt, pMenu; controller hierarchy. |
| FancyCritter | Cutscene pCrRun, pCrFollow; basic_controllers. |
| Knight, Princess, Mage, Trader, etc. | Created in critter_generators/critters with make_smart and AddBoth/AddE; stored in lsPpl/lsUpdate. |
| Ghostiness, AnimationOnce, FloatingSlime, Sliminess (one site) | Various critter/spawn paths; AddE or member. |
| LevelController, BuyNowController, MenuController, DragonScoreController, EntityListController | DGC Init; vCnt and pMenu. |
| GameRunner | game_runner_interface_impl. |

Doc-only (Phase 4 DOC-01). Phase 4 success criteria met: critter/generator/fireball/level ownership
migrated where planned; SP_Info removed from FireballBonus; build and tests pass.
