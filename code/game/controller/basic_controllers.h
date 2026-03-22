#ifndef TOWER_DEFENSE_BASIC_CONTROLLERS_H
#define TOWER_DEFENSE_BASIC_CONTROLLERS_H

#include "game_controller_interface.h"
#include "../../utils/index.h"
#include "../../utils/timer.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include "../../wrappers/gui_key_type.h"
#include <list>
#include <memory>
#include <vector>

struct Animation;
struct ConsumableEntity;
struct Entity;
struct FancyCritter;

// TODO: move to a separate file.
/** A controller that keeps lists of things to draw, update, and consume; each
 * tick it moves, updates, then paints from back to front. */
struct EntityListController : public GameController {
  std::string get_class_name() { return "EntityListController"; }

  /** Consumable entities (knight, princess, …) owned here. */
  std::list<std::unique_ptr<ConsumableEntity>> lsPpl;

  /** Non-consumable entities owned here (animations, effects, …). */
  std::list<std::unique_ptr<Entity>> owned_entities;

  /** Raw-pointer view for iteration — points into lsPpl or owned_entities. */
  std::list<Entity *> owned_entity_list;

  void AddOwnedEntity(std::unique_ptr<Entity> p);

  /** Own a consumable entity (knight, princess, …). */
  template <class T> void AddOwnedConsumable(std::unique_ptr<T> p) {
    T *raw = p.get();
    lsPpl.push_back(std::unique_ptr<ConsumableEntity>(p.release()));
    owned_entity_list.push_back(raw);
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
  void Update() override;

  /** Entities that tick and draw here but are owned elsewhere. */
  virtual std::vector<Entity *> GetNonOwnedEntities() { return {}; }

  /** Pointers to consumable entities for hit detection (e.g. fireball). */
  virtual std::vector<ConsumableEntity *> GetConsumablePointers();

  void OnKey(GuiKeyType c, bool bUp) override;

  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "basic"; }
};

struct StartScreenController : public EntityListController {
  std::string get_class_name() { return "StartScreenController"; }
  StartScreenController(DragonGameController *pGl_, Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  /** Step to the next screen and let the bard play the start_game tune. */
  void Next();

  void OnKey(GuiKeyType c, bool bUp) override;

  void OnMouseDown(Point /*pPos*/) override { Next(); }
  std::string GetControllerName() const override { return "start"; }
};

struct Cutscene : public EntityListController {
  std::string get_class_name() { return "Cutscene"; }
  std::unique_ptr<FancyCritter> pCrRun;
  std::unique_ptr<FancyCritter> pCrFollow;

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
  ~Cutscene();

  void Update() override;
  void OnKey(GuiKeyType c, bool bUp) override;
  void OnMouseDown(Point /*pPos*/) override {}
  std::string GetControllerName() const override { return "cutscene"; }

  std::vector<Entity *> GetNonOwnedEntities() override;
};

/** A screen that shows the dragon's tally and leaves on click or when time
 * runs out. */
struct DragonScoreController : public EntityListController {
  std::string get_class_name() { return "DragonScoreController"; }
  Timer t;
  bool bClickToExit;

  DragonScoreController(DragonGameController *pGl_, Rectangle rBound, Color c,
                        bool bScoreShow);

  void OnKey(GuiKeyType c, bool bUp) override;
  void Update() override;
  void DoubleClick() override;

  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "score"; }
};

/** A controller that steps to the next screen when only the veil remains, or
 * when the hero gives a sign. */
struct AutoAdvanceController : public EntityListController {
  std::string get_class_name() { return "AutoAdvanceController"; }
  AutoAdvanceController(DragonGameController *pGl_, Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  void Update() override;

  void OnKey(GuiKeyType c, bool bUp) override;

  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "logo"; }
};

#endif
