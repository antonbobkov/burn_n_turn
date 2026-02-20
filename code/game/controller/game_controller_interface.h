#ifndef TOWER_DEFENSE_GAME_CONTROLLER_INTERFACE_H
#define TOWER_DEFENSE_GAME_CONTROLLER_INTERFACE_H

#include "utils/smart_pointer.h"
#include "wrappers/geometry.h"
#include "wrappers/gui_key_type.h"

struct DragonGameController;

/** The base of all screen keepers: holds the realm and bounds; input and
 * tick are virtual (OnKey, OnMouse, Fire, etc.). */
struct GameController : virtual public SP_Info {
  std::string get_class_name() override { return "GameController"; }
  DragonGameController *pGl;

  Rectangle rBound;

  GameController(const GameController &gc)
      : pGl(gc.pGl), rBound(gc.rBound) {}

  GameController(DragonGameController *pGl_,
                 Rectangle rBound_ = Rectangle())
      : pGl(pGl_), rBound(rBound_) {}

  virtual void Update() {}
  virtual void OnKey(GuiKeyType c, bool bUp) {}
  virtual void OnMouse(Point pPos) {}
  virtual void OnMouseDown(Point pPos) {}
  virtual void OnMouseUp() {}
  virtual void DoubleClick() {}
  virtual void Fire() {}

  /** The name of this screen for scribes and trials (e.g. menu, level). */
  virtual std::string GetControllerName() const { return "basic"; }
};

#endif
