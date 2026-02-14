#ifndef TOWER_DEFENSE_BASIC_CONTROLLERS_H
#define TOWER_DEFENSE_BASIC_CONTROLLERS_H

/* Include after GameController and DragonGameControllerList are defined (e.g.
 * from game.h). */
#include "utils/smart_pointer.h"
#include "utils/timer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"
#include "wrappers/gui_key_type.h"
#include <list>
#include <memory>
#include <vector>


struct Animation;
struct ConsumableEntity;
struct Entity;
struct EventEntity;
struct FancyCritter;
struct VisualEntity;

/** Controller that draws a single full-screen image and advances on key. */
struct SimpleController : public GameController {
  std::string get_class_name() override { return "SimpleController"; }
  Index nImage;

  SimpleController(DragonGameControllerList *pGraph, std::string strFileName);
  ~SimpleController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

/** Controller that draws background + text that flashes every second. */
struct FlashingController : public GameController {
  std::string get_class_name() override { return "FlashingController"; }
  Index nImage, nText;
  unsigned nTimer;
  bool bShow;

  FlashingController(DragonGameControllerList *pGraph, std::string strFileName,
                     std::string strTextName);
  ~FlashingController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

/** GameController with draw/update/consumable lists; Update runs Move, Update,
 * then draws by priority. */
struct EntityListController : public GameController {
  std::string get_class_name() override { return "EntityListController"; }
  std::list<smart_pointer<VisualEntity>> lsDraw;
  std::list<smart_pointer<EventEntity>> lsUpdate;
  std::list<smart_pointer<ConsumableEntity>> lsPpl;

  /** Owned entities (controller owns lifetime). Empty for now. */
  std::list<std::unique_ptr<Entity>> owned_entities;
  std::list<VisualEntity *> owned_visual_entities;
  std::list<EventEntity *> owned_event_entities;

  /** Add visual entity to lsDraw. */
  void AddV(smart_pointer<VisualEntity> pVs);
  /** Adds an event entity to the update list. */
  void AddE(smart_pointer<EventEntity> pEv);

  void AddOwnedVisualEntity(std::unique_ptr<VisualEntity> p);
  void AddOwnedEventEntity(std::unique_ptr<EventEntity> p);
  template <class T> void AddOwnedBoth(std::unique_ptr<T> p) {
    T *raw = p.get();
    owned_entities.push_back(std::unique_ptr<Entity>(p.release()));
    owned_visual_entities.push_back(raw);
    owned_event_entities.push_back(raw);
  }

  template <class T> void AddBoth(T &t) {
    lsDraw.push_back(t);
    lsUpdate.push_back(t);
  }

  /** Add scaled fullscreen StaticRectangle of color c to lsDraw. */
  void AddBackground(Color c);

  EntityListController(const EntityListController &) = delete;
  bool bNoRefresh;
  EntityListController(DragonGameControllerList *pGl_, Rectangle rBound,
                       Color c);

  /**
   * Each frame: remove dead things from the lists, move everything that can
   * move, then update everyone. Draw everything in order from back to front.
   * Refresh the screen unless refresh is disabled.
   */
  /*virtual*/ void Update();

  /** Non-owned entities to update and draw (e.g. owned elsewhere). */
  virtual std::vector<EventEntity *> GetNonOwnedUpdateEntities() { return {}; }
  virtual std::vector<VisualEntity *> GetNonOwnedDrawEntities() { return {}; }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

struct StartScreenController : public EntityListController {
  std::string get_class_name() override { return "StartScreenController"; }
  StartScreenController(DragonGameControllerList *pGl_, Rectangle rBound,
                        Color c)
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
  std::string get_class_name() override { return "BuyNowController"; }
  int t;
  std::vector<smart_pointer<Animation>> mSlimes;
  std::vector<fPoint> mSlimeVel;
  std::vector<fPoint> mSlimePos;
  int nSlimeCount;

  Timer tVel;

  BuyNowController(DragonGameControllerList *pGl_, Rectangle rBound, Color c);

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
  std::string get_class_name() override { return "Cutscene"; }
  smart_pointer<FancyCritter> pCrRun;
  smart_pointer<FancyCritter> pCrFollow;

  Timer tm;
  bool Beepy;

  bool bRelease;

  /**
   * Sets up the chase: black screen, one character running and one waiting.
   * Runner starts left or right depending on flip; when the runner reaches the
   * middle, the chaser is released. Beep/boop timer for sound.
   */
  Cutscene(DragonGameControllerList *pGl_, Rectangle rBound_, std::string sRun,
           std::string sChase, bool bFlip = false);

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouseDown(Point pPos) {}
  /*virtual*/ std::string GetControllerName() const { return "cutscene"; }
};

/** Controller that shows dragon score and exits on click or timer. */
struct DragonScoreController : public EntityListController {
  std::string get_class_name() override { return "DragonScoreController"; }
  Timer t;
  bool bClickToExit;

  DragonScoreController(DragonGameControllerList *pGl_, Rectangle rBound,
                        Color c, bool bScoreShow);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void Update();
  /*virtual*/ void DoubleClick();

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "score"; }
};

/** EntityListController that advances (Next) when only background is left or
 * on input. */
struct AutoAdvanceController : public EntityListController {
  std::string get_class_name() override { return "AutoAdvanceController"; }
  AutoAdvanceController(DragonGameControllerList *pGl_, Rectangle rBound,
                        Color c)
      : EntityListController(pGl_, rBound, c) {}

  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "logo"; }
};

#endif
