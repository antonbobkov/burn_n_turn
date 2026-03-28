#ifndef TOWER_DEFENSE_CRITTERS_TRADER_H
#define TOWER_DEFENSE_CRITTERS_TRADER_H

#include "critters.h"
#include <string>

class LevelController;

/** Pick a random fireball bonus index, weighted by desirability.
 * bInTower controls whether ring fireball is on the table. */
int RandomBonus(bool bInTower = true);

/** Return the image name string for bonus index n. */
std::string GetBonusImage(int n);

/** The trader: drops a treasure when felled; first-bonus is tracked elsewhere. */
class Trader : public Critter {
public:
  std::string get_class_name() override { return "Trader"; }

  Trader(const Critter &cr, LevelController *pAc_, bool &bFirstBns_)
      : Critter(cr), pAc(pAc_), bFirstBns(bFirstBns_) {}

  Index GetImage() override { return seq.GetImageAt(0); }

  std::string GetType() override { return "trader"; }

  void OnHit(char cWhat) override;

  void Draw(ScalingDrawer *pDr) override;

private:
  LevelController *pAc;
  bool &bFirstBns;
};

#endif
