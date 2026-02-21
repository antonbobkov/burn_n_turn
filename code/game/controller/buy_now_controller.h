#ifndef TOWER_DEFENSE_BUY_NOW_CONTROLLER_H
#define TOWER_DEFENSE_BUY_NOW_CONTROLLER_H

#include "game/controller/basic_controllers.h"
#include "game/entities.h"
#include "game_utils/draw_utils.h"
#include "utils/smart_pointer.h"

/** The keeper of the buy-now screen: slime dances and a countdown. */
struct BuyNowController : public EntityListController {
  std::string get_class_name() override { return "BuyNowController"; }
  int t;
  std::vector<smart_pointer<Animation>> mSlimes;
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
  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "buy"; }
};

/** A sight that paints the slime dances on the buy-now screen. */
struct SlimeUpdater : public VisualEntity {
  BuyNowController *pBuy;

  SlimeUpdater(BuyNowController *pBuy_) : pBuy(pBuy_) {}

  /*virtual*/ void Draw(ScalingDrawer *pDr);
  /*virtual*/ float GetPriority() { return 0; }
};

#endif
