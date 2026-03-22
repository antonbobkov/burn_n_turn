#ifndef TOWER_DEFENSE_ENTITY_LIST_CONTROLLER_H
#define TOWER_DEFENSE_ENTITY_LIST_CONTROLLER_H

#include "game_controller_interface.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include "../../wrappers/gui_key_type.h"
#include <list>
#include <memory>
#include <vector>

struct ConsumableEntity;
struct Entity;

/** A controller that keeps lists of things to draw, update, and consume; each
 * tick it moves, updates, then paints from back to front. */
struct EntityListController : public GameController {
  std::string get_class_name() { return "EntityListController"; }

  /** Non-consumable entities owned here (animations, effects, …). */
  std::list<std::unique_ptr<Entity>> owned_entities;

  /** Raw-pointer view for iteration — points into lsPpl or owned_entities. */
  std::list<Entity *> owned_entity_list;

  void AddOwnedEntity(std::unique_ptr<Entity> p);

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
  virtual std::vector<ConsumableEntity *> GetConsumablePointers() { return {}; }

  /** Clean up dead consumable entities. Override in subclasses that own them. */
  virtual void CleanUpConsumables() {}

  /** Clean dead entries from owned_entity_list and return the number of
   * entities that would be drawn this tick. */
  int CountDrawable();

  void OnKey(GuiKeyType c, bool bUp) override;

  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "basic"; }
};

#endif
