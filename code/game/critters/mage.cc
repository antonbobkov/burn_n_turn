#include "mage.h"
#include "fighter.h"
#include "slime.h"
#include "../castle.h"
#include "../controller/dragon_game_controller.h"
#include "../controller/level_controller.h"
#include "../dragon_constants.h"
#include "../../utils/random_utils.h"
#include <memory>

Mage::Mage(const Critter &cr, LevelController *pAc_, bool bAngry_)
    : Critter(cr), pAc(pAc_), bAngry(bAngry_), bCasting(false),
      tUntilSpell(GetTimeUntillSpell()), tSpell(3 * nFramesInSecond),
      tSpellAnimate(int(.7F * nFramesInSecond)) {
  fMvVel = Critter::fVel;
}

void Mage::OnHit(char /*cWhat*/) {
  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->GetGl()->GetImgSeq("mage_die_f")
                 : pAc->GetGl()->GetImgSeq("mage_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  // Slaying a mage triggers Angry Mode — from this point on, new mages spawn faster.
  pAc->GetGl()->SetAngry();

  // On level 7+ the mage curses the land as it falls, hatching slimes from its corpse.
  if (pAc->GetLevel() > 6)
    SummonSlimes();
}

void Mage::Update() {
  // A non-angry mage is harmless — it just walks its road. Only angry mages cast spells.
  if (bAngry) {
    if (!bCasting) {
      // The dark wizard refuses to conjure near the castle walls — he needs space for his ritual.
      bool bNearCastle = false;
      for (Castle *pC : pAc->GetCastlePointers()) {
        fPoint p = pC->GetPosition() - fPos;
        if (p.Length() < nSummonRadius) {
          bNearCastle = true;
          break;
        }
      }
      // Each frame, a random chance to begin casting (~once every 12 seconds on average).
      if (!bNearCastle)
        if (rand() % nSummonChance == 0) {
          bCasting = true;
          Critter::seq = pAc->GetGl()->GetImgSeq("mage_spell");
          Critter::fVel = fPoint(0, 0);
        }
    } else {
      if (tSpellAnimate.Tick()) {
        Critter::seq.Toggle();
      }

      if (tSpell.UntilTick() == int(1.F * nFramesInSecond)) {
        SummonSkeletons(pAc, GetPosition());
      }

      if (tSpell.Tick()) {
        bCasting = false;
        Critter::fVel = fMvVel;
        Critter::seq = fMvVel.x < 0 ? pAc->GetGl()->GetImgSeq("mage_f")
                                    : pAc->GetGl()->GetImgSeq("mage");
      }
    }
  }

  Critter::Update();
}

void Mage::SummonSlimes() {
  for (int i = 0; i < 2; ++i) {
    fPoint f = fPoint::Normalized(RandomAngle(), 10);

    pAc->AddSpawningSlime(std::make_unique<SpawningSlime>(
        GetPosition() + f.ToPnt(), pAc, true, 0));
  }
}
