#ifndef TOWER_DEFENSE_GAME_CONTROLLER_INTERFACE_H
#define TOWER_DEFENSE_GAME_CONTROLLER_INTERFACE_H

#include "../../wrappers/geometry.h"
#include "../../wrappers/gui_key_type.h"

class DragonGameController;

/** The base of all screen keepers: holds the realm and bounds; input and
 * tick are virtual (OnKey, OnMouse, Fire, etc.). */
class GameController {
public:
  GameController(const GameController &gc)
      : pGl(gc.pGl), rBound(gc.rBound) {}

  GameController(DragonGameController *pGl_,
                 Rectangle rBound_ = Rectangle())
      : pGl(pGl_), rBound(rBound_) {}

  virtual void Update() {}
  virtual void OnKey(GuiKeyType /*c*/, bool /*bUp*/) {}
  virtual void OnMouse(Point /*pPos*/) {}
  virtual void OnMouseDown(Point /*pPos*/) {}
  virtual void OnMouseUp() {}
  virtual void DoubleClick() {}
  virtual void Fire() {}

  /** The name of this screen for scribes and trials (e.g. menu, level). */
  virtual std::string GetControllerName() const { return "basic"; }

  DragonGameController *GetGl() const { return pGl; }
  Rectangle GetBound() const { return rBound; }

protected:
  DragonGameController *pGl;
  Rectangle rBound;
};

#endif
