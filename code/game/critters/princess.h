#ifndef TOWER_DEFENSE_CRITTERS_PRINCESS_H
#define TOWER_DEFENSE_CRITTERS_PRINCESS_H

#include "critters.h"

class LevelController;

/** The princess: a soul the dragon can rescue and carry to the castle. */
class Princess : public Critter {
public:
  std::string get_class_name() override { return "Princess"; }

  Princess(const Critter &cr, LevelController *pAc_) : Critter(cr), pAc(pAc_) {}

  Index GetImage() override { return seq.GetImageAt(0); }

  std::string GetType() override { return "princess"; }

  /** When hit: show the bonus tally and vanish from the world. */
  void OnHit(char cWhat) override;

  void Draw(ScalingDrawer *pDr) override;

private:
  LevelController *pAc;
};

#endif
