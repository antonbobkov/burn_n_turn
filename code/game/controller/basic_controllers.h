#ifndef TOWER_DEFENSE_BASIC_CONTROLLERS_H
#define TOWER_DEFENSE_BASIC_CONTROLLERS_H

/* Include after GameController and TwrGlobalController are defined (e.g. from
 * game.h). */
#include "entities.h"
#include "gui_key_type.h"
#include "smart_pointer.h"

/** Controller that draws a single full-screen image and advances on key. */
struct SimpleController : public GameController {
  Index nImage;

  SimpleController(smart_pointer<TwrGlobalController> pGraph,
                   std::string strFileName);
  ~SimpleController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

/** Controller that draws background + text that flashes every second. */
struct FlashingController : public GameController {
  Index nImage, nText;
  unsigned nTimer;
  bool bShow;

  FlashingController(smart_pointer<TwrGlobalController> pGraph,
                     std::string strFileName, std::string strTextName);
  ~FlashingController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

/** GameController with draw/update/consumable lists; Update runs Move, Update,
 * then draws by priority. */
struct EntityListController : public GameController {
  std::list<ASSP<VisualEntity>> lsDraw;
  std::list<ASSP<EventEntity>> lsUpdate;
  std::list<ASSP<ConsumableEntity>> lsPpl;

  /** Add visual entity to lsDraw. */
  void AddV(smart_pointer<VisualEntity> pVs);
  /** Adds an event entity to the update list. */
  void AddE(smart_pointer<EventEntity> pEv);

  template <class T> void AddBoth(T &t) {
    lsDraw.push_back(ASSP<VisualEntity>(this, t));
    lsUpdate.push_back(ASSP<EventEntity>(this, t));
  }

  /** Add scaled fullscreen StaticRectangle of color c to lsDraw. */
  void AddBackground(Color c);

  EntityListController(const EntityListController &b);
  bool bNoRefresh;
  EntityListController(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound,
                  Color c);

  /**
   * Each frame: remove dead things from the lists, move everything that can
   * move, then update everyone. Draw everything in order from back to front.
   * Refresh the screen unless refresh is disabled.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

struct StartScreenController : public EntityListController {
  StartScreenController(smart_pointer<TwrGlobalController> pGl_,
                        Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  /** Advance to next screen and play start_game sound. */
  void Next();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos) { Next(); }
  /*virtual*/ std::string GetControllerName() const { return "start"; }
};

struct BuyNowController;

/** Controller for buy-now screen: slime animations and timer. */
struct BuyNowController : public EntityListController {
  int t;
  std::vector<smart_pointer<Animation>> mSlimes;
  std::vector<fPoint> mSlimeVel;
  std::vector<fPoint> mSlimePos;
  int nSlimeCount;

  Timer tVel;

  BuyNowController(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound,
                   Color c);

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

struct Cutscene : public EntityListController {
  SSP<FancyCritter> pCrRun;
  SSP<FancyCritter> pCrFollow;

  Timer tm;
  bool Beepy;

  bool bRelease;

  /**
   * Sets up the chase: black screen, one character running and one waiting.
   * Runner starts left or right depending on flip; when the runner reaches the
   * middle, the chaser is released. Beep/boop timer for sound.
   */
  Cutscene(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound_,
           std::string sRun, std::string sChase, bool bFlip = false);

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouseDown(Point pPos) {}
  /*virtual*/ std::string GetControllerName() const { return "cutscene"; }
};

/** Controller that shows dragon score and exits on click or timer. */
struct DragonScoreController : public EntityListController {
  Timer t;
  bool bClickToExit;

  DragonScoreController(smart_pointer<TwrGlobalController> pGl_,
                        Rectangle rBound, Color c, bool bScoreShow);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void Update();
  /*virtual*/ void DoubleClick();

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "score"; }
};

/** EntityListController that advances (Next) when only background is left or
 * on input. */
struct AutoAdvanceController : public EntityListController {
  AutoAdvanceController(const EntityListController &b)
      : EntityListController(b) {}

  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "logo"; }
};

#endif
