#include "princess.h"
#include "../controller/dragon_game_controller.h"
#include "../controller/level_controller.h"
#include "../dragon_constants.h"
#include <memory>

void Princess::OnHit(char /*cWhat*/) {
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 250));

  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->GetGl()->GetImgSeq("princess_die_f")
                 : pAc->GetGl()->GetImgSeq("princess_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
}

void Princess::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->GetGl()->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->GetGl()->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}
