#include "trader.h"
#include "../controller/dragon_game_controller.h"
#include "../controller/level_controller.h"
#include "../dragon_constants.h"
#include "../fireball.h"
#include "../tutorial.h"
#include "../../utils/random_utils.h"
#include <memory>
#include <vector>

std::string GetBonusImage(int n) {
  if (n == 0)
    return "void_bonus";
  if (n == 1)
    return "pershot_bonus";
  if (n == 2)
    return "laser_bonus";
  if (n == 3)
    return "big_bonus";
  if (n == 4)
    return "totnum_bonus";
  if (n == 5)
    return "explode_bonus";
  if (n == 6)
    return "split_bonus";
  if (n == 7)
    return "burning_bonus";
  if (n == 8)
    return "ring_bonus";
  if (n == 9)
    return "nuke_bonus";
  if (n == 10)
    return "speed_bonus";
  return "void_bonus";
}

int RandomBonus(bool bInTower) {
  std::vector<float> v;

  v.push_back(0.F);  // time
  v.push_back(1.2F); // pershot
  v.push_back(.5F);  // laser
  v.push_back(1.F);  // big
  v.push_back(1.F);  // total num
  v.push_back(.8F);  // explode
  v.push_back(1.F);  // split fireball
  v.push_back(.15F); // set on fire

  if (bInTower)
    v.push_back(1.F); // ring fireball
  else
    v.push_back(0.F);

  v.push_back(.1F); // nuke
  v.push_back(.8F); // speed
  v.push_back(0.F); // shooting frequency

  return GetRandFromDistribution(v);
}

void Trader::OnHit(char /*cWhat*/) {
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 60));

  this->Destroy();

  pAc->TutorialNotify(TutorialEvent::TraderKilled);

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->GetGl()->GetImgSeq("trader_die")
                 : pAc->GetGl()->GetImgSeq("trader_die_f"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  auto pFb = std::make_unique<FireballBonusAnimation>(
      GetPosition(), RandomBonus(false), pAc);
  if (bFirstBns) {
    pFb->SetUnderText("loot");
    bFirstBns = false;
  }
  pAc->AddBonusAnimation(std::move(pFb));
}

void Trader::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->GetGl()->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->GetGl()->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}
