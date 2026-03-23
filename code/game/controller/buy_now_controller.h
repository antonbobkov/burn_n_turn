#ifndef TOWER_DEFENSE_BUY_NOW_CONTROLLER_H
#define TOWER_DEFENSE_BUY_NOW_CONTROLLER_H

#include "basic_controllers.h"
#include "../entities.h"
#include "../../game_utils/draw_utils.h"
#include <memory>

/** The keeper of the buy-now screen: slime dances and a countdown. */
class BuyNowController : public EntityListController {
public:
  std::string get_class_name() { return "BuyNowController"; }
  int t;
  std::vector<std::unique_ptr<Animation>> mSlimes;
  std::vector<fPoint> mSlimeVel;
  std::vector<fPoint> mSlimePos;
  int nSlimeCount;

  Timer tVel;

  BuyNowController(DragonGameController *pGl_, Rectangle rBound, Color c);

  /** Give a slime a new random heading, sometimes toward the center; scale by
   * speed. */
  void RandomizeVelocity(fPoint &fVel, fPoint pPos);

  /** Paint all the slime dances. */
  void DrawSlimes();

  /**
   * The usual tick: now and then some slimes choose a new path. All slimes
   * move and dance. The countdown runs. The vista is refreshed.
   */
  void Update() override;

  void OnKey(GuiKeyType c, bool bUp) override;

  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "buy"; }
};

/** A sight that paints the slime dances on the buy-now screen. */
class SlimeUpdater : public Entity {
public:
  bool ShouldDraw() override { return true; }
  BuyNowController *pBuy;

  SlimeUpdater(BuyNowController *pBuy_) : pBuy(pBuy_) {}

  /*virtual*/ void Draw(ScalingDrawer *pDr);
  /*virtual*/ float GetPriority() { return 0; }
};

#endif
