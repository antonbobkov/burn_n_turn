#ifndef TOWER_DEFENSE_ENTITY_LIST_CONTROLLER_H
#define TOWER_DEFENSE_ENTITY_LIST_CONTROLLER_H

#include "game_controller_interface.h"
#include "../entity_ledger.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include "../../wrappers/gui_key_type.h"
#include <list>
#include <memory>
#include <vector>

class Entity;

/** A controller that keeps lists of things to draw, update, and consume; each
 * tick it moves, updates, then paints from back to front. */
class EntityListController : public GameController, public EntityLedger {
public:
  std::string get_class_name() { return "EntityListController"; }

  void AddOwnedEntity(std::unique_ptr<Entity> p);

  /** Add a fullscreen colored veil to the draw list. */
  void AddBackground(Color c);

  EntityListController(const EntityListController &) = delete;
  EntityListController(DragonGameController *pGl_, Rectangle rBound, Color c);

  /** Prevent the automatic screen refresh at the end of each tick. */
  void SuppressRefresh() { bNoRefresh = true; }

  /**
   * Each tick: clear the fallen from the list, move all that can move, then
   * update everyone. Draw from back to front. Refresh the vista unless
   * refresh is stilled.
   */
  void Update() override;

  /** Inscribe a soul into the ledger so it moves and draws each tick.
   * Does nothing if the soul is already null. */
  void Register(Entity *e) override;

  /** Erase a soul from the ledger — marks the slot null so the frame loop
   * can skip it safely and purge it after the tick ends. */
  void Unregister(Entity *e) override;

  /** Count of owned entities that exist and would be drawn this tick. */
  int CountDrawable();

  void OnKey(GuiKeyType c, bool bUp) override;
  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "basic"; }

private:
  /** Souls inscribed in this ledger — includes both owned and non-owned
   * entities. Declared before owned_entities so it outlives it during
   * destruction (entity destructors call Unregister). Slots are null-outed
   * by Unregister and purged at the end of each tick. */
  std::vector<Entity *> registered_entities_;

  /** Non-consumable entities owned here (animations, effects, …). */
  std::list<std::unique_ptr<Entity>> owned_entities;
  bool bNoRefresh;
};

#endif
