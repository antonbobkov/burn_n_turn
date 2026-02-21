#include "game_utils/game_runner_interface.h"
#include "game/controller/buy_now_controller.h"
#include "game/controller/dragon_game_controller.h"
#include "game/dragon_constants.h"
#include "game/entities.h"
#include "utils/random_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"

BuyNowController::BuyNowController(DragonGameController *pGl_, Rectangle rBound,
                                   Color c)
    : EntityListController(pGl_, rBound, c), t(120), nSlimeCount(50),
      tVel(nFramesInSecond / 2) {
  for (int i = 0; i < nSlimeCount; i++) {
    mSlimes.push_back(std::make_unique<Animation>(
        0, pGl->GetImgSeq("slime"), nFramesInSecond / 10,
        Point(rBound.sz.x / 2, rBound.sz.y / 2 + 25), true));
    mSlimeVel.push_back(fPoint());
    mSlimePos.push_back(mSlimes.back()->pos);
  }

  bNoRefresh = true;
}

void BuyNowController::RandomizeVelocity(fPoint &fVel, fPoint pPos) {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(pPos);

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

void BuyNowController::DrawSlimes() {
  for (unsigned i = 0; i < mSlimes.size(); i++)
    mSlimes[i]->Draw(pGl->GetDrawer());
}

void BuyNowController::Update() {
  EntityListController::Update();

  if (tVel.Tick()) {
    for (unsigned i = 0; i < mSlimes.size(); i++)
      if (float(rand()) / RAND_MAX < .25)
        RandomizeVelocity(mSlimeVel[i], mSlimePos[i]);
  }

  for (unsigned i = 0; i < mSlimes.size(); i++) {
    mSlimes[i]->Update();
    mSlimePos[i] += mSlimeVel[i];
    mSlimes[i]->pos = mSlimePos[i].ToPnt();
  }

  if (t >= 0)
    t--;

  pGl->RefreshAll();
}

void BuyNowController::OnKey(GuiKeyType c, bool bUp) {
  if (t < 0)
    EntityListController::OnKey(c, bUp);
}

void BuyNowController::OnMouseDown(Point pPos) {
  if (t < 0)
    EntityListController::OnMouseDown(pPos);
}

void SlimeUpdater::Draw(ScalingDrawer *pDr) {
  pBuy->DrawSlimes();
}
