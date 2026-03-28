#ifndef TOWER_DEFENSE_CRITTERS_MAGE_H
#define TOWER_DEFENSE_CRITTERS_MAGE_H

#include "critters.h"
#include "../dragon_constants.h"

class LevelController;

class Mage : public Critter {
public:
  std::string get_class_name() override { return "Mage"; }

  Mage(const Critter &cr, LevelController *pAc_, bool bAngry_);

  Index GetImage() override { return seq.GetImageAt(0); }

  void SummonSlimes();

  std::string GetType() override { return "mage"; }

  void Update() override;

  void OnHit(char cWhat) override;

private:
  LevelController *pAc;
  bool bAngry;
  bool bCasting;
  Timer tUntilSpell;
  Timer tSpell;
  Timer tSpellAnimate;
  fPoint fMvVel;
};

/** How many frames until a mage begins its next summoning ritual. */
inline int GetTimeUntillSpell() {
  return 8 * nFramesInSecond + rand() % (3 * nFramesInSecond);
}

#endif
