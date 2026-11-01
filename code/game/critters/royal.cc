#include "royal.h"
#include "../controller/dragon_game_controller.h"
#include "../controller/level_controller.h"
#include "../dragon_constants.h"
#include "../../utils/random_utils.h"
#include <memory>
#include <string>

RoyalKind RandomRoyalKind() {
  return GetRandNum(2) == 0 ? RoyalKind::kPrincess : RoyalKind::kPrince;
}

std::unique_ptr<Royal> MakeRoyal(RoyalKind kind, fPoint pos, fPoint vel,
                                 Rectangle bound, float priority,
                                 LevelController *level) {
  std::string name = kind == RoyalKind::kPrince ? "prince" : "princess";
  const ImageSequence &seq =
      level->GetGl()->GetImgSeq(vel.x < 0 ? name + "_f" : name);
  return std::make_unique<Royal>(
      Critter(7, pos, vel, bound, priority, seq, true), level, kind);
}

void Royal::OnHit(char /*cWhat*/) {
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 250));

  this->Destroy();

  // Each royal burns in their own flames, and none may be mistaken for another.
  std::string name =
      kind_ == RoyalKind::kPrince ? "prince_die" : "princess_die";
  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(), pAc->GetGl()->GetImgSeq(fVel.x < 0 ? name + "_f" : name),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
}

void Royal::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->GetGl()->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->GetGl()->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}
