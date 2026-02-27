#ifndef TOWER_DEFENSE_BASIC_CONTROLLERS_H
#define TOWER_DEFENSE_BASIC_CONTROLLERS_H

#include "game/controller/game_controller_interface.h"
#include "utils/index.h"
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

/** A controller that keeps lists of things to draw, update, and consume; each
 * tick it moves, updates, then paints from back to front. */
struct EntityListController : public GameController {
  std::string get_class_name() override { return "EntityListController"; }
  std::list<smart_pointer<VisualEntity>> lsDraw;
  std::list<smart_pointer<EventEntity>> lsUpdate;
  std::list<smart_pointer<ConsumableEntity>> lsPpl;

  /** Creatures and sights this controller brought into the world (it owns
   * them). Empty for now. */
  std::list<std::unique_ptr<Entity>> owned_entities;
  std::list<VisualEntity *> owned_visual_entities;
  std::list<EventEntity *> owned_event_entities;

  /** Add a sight to the list of things drawn. */
  void AddV(smart_pointer<VisualEntity> pVs);
  /** Add a creature that ticks each frame to the update list. */
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

  /** Add a fullscreen colored veil to the draw list. */
  void AddBackground(Color c);

  EntityListController(const EntityListController &) = delete;
  bool bNoRefresh;
  EntityListController(DragonGameController *pGl_, Rectangle rBound, Color c);

  /**
   * Each tick: clear the fallen from the lists, move all that can move, then
   * update everyone. Draw from back to front. Refresh the vista unless
   * refresh is stilled.
   */
  /*virtual*/ void Update();

  /** Creatures and sights that tick and draw here but are owned elsewhere. */
  virtual std::vector<EventEntity *> GetNonOwnedUpdateEntities() { return {}; }
  virtual std::vector<VisualEntity *> GetNonOwnedDrawEntities() { return {}; }

  /** Pointers to consumable entities for hit detection (e.g. fireball). */
  virtual std::vector<ConsumableEntity *> GetConsumablePointers();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

struct StartScreenController : public EntityListController {
  std::string get_class_name() override { return "StartScreenController"; }
  StartScreenController(DragonGameController *pGl_, Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  /** Step to the next screen and let the bard play the start_game tune. */
  void Next();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos) { Next(); }
  /*virtual*/ std::string GetControllerName() const { return "start"; }
};

struct Cutscene : public EntityListController {
  std::string get_class_name() override { return "Cutscene"; }
  smart_pointer<FancyCritter> pCrRun;
  smart_pointer<FancyCritter> pCrFollow;

  Timer tm;
  bool Beepy;

  bool bRelease;

  /**
   * Set the stage: dark veil, one soul running and one waiting. Runner starts
   * left or right by flip; when the runner reaches the middle, the chaser is
   * unleashed. A timer drives the beeps and boops.
   */
  Cutscene(DragonGameController *pGl_, Rectangle rBound_, std::string sRun,
           std::string sChase, bool bFlip = false);

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouseDown(Point pPos) {}
  /*virtual*/ std::string GetControllerName() const { return "cutscene"; }
};

/** A screen that shows the dragon's tally and leaves on click or when time
 * runs out. */
struct DragonScoreController : public EntityListController {
  std::string get_class_name() override { return "DragonScoreController"; }
  Timer t;
  bool bClickToExit;

  DragonScoreController(DragonGameController *pGl_, Rectangle rBound, Color c,
                        bool bScoreShow);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void Update();
  /*virtual*/ void DoubleClick();

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "score"; }
};

/** A controller that steps to the next screen when only the veil remains, or
 * when the hero gives a sign. */
struct AutoAdvanceController : public EntityListController {
  std::string get_class_name() override { return "AutoAdvanceController"; }
  AutoAdvanceController(DragonGameController *pGl_, Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "logo"; }
};

#endif
