#ifndef TOWER_DEFENSE_BUY_NOW_CONTROLLER_H
#define TOWER_DEFENSE_BUY_NOW_CONTROLLER_H

#include "game/controller/basic_controllers.h"
#include "game/entities.h"
#include "game_utils/draw_utils.h"
#include "utils/smart_pointer.h"

/** Controller for buy-now screen: slime animations and timer. */
struct BuyNowController : public EntityListController {
  std::string get_class_name() override { return "BuyNowController"; }
  int t;
  std::vector<smart_pointer<Animation>> mSlimes;
  std::vector<fPoint> mSlimeVel;
  std::vector<fPoint> mSlimePos;
  int nSlimeCount;

  Timer tVel;

  BuyNowController(DragonGameController *pGl_, Rectangle rBound, Color c);

  /** Set fVel to random direction, sometimes toward center; scale by speed. */
  void RandomizeVelocity(fPoint &fVel, fPoint pPos);

  /** Draw all slime animations. */
  void DrawSlimes();

  /**
   * Runs the normal controller update. Every so often, some slimes get a new
   * random direction. All slimes move and animate. Countdown runs. Screen
   * refreshes.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "buy"; }
};

/** VisualEntity that draws slime animations for the buy-now screen. */
struct SlimeUpdater : public VisualEntity {
  BuyNowController *pBuy;

  SlimeUpdater(BuyNowController *pBuy_) : pBuy(pBuy_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
  /*virtual*/ float GetPriority() { return 0; }
};

#endif
