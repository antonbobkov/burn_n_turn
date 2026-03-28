#ifndef TOWER_DEFENSE_CASTLE_H
#define TOWER_DEFENSE_CASTLE_H

#include "critters/critters.h"
#include <string>

class LevelController;
class Dragon;

class Castle : public Critter {
public:
  std::string get_class_name() override { return "Castle"; }

  Castle(Point p, Rectangle rBound_, LevelController *pAv_);

  std::string GetType() override { return "castle"; }
  void OnKnight(std::string cWhat);
  void Draw(ScalingDrawer *pDr) override;

  int GetPrincessCount() const { return nPrincesses; }
  void AddPrincesses(int n) { nPrincesses += n; }
  bool HasDragon() const { return pDrag != nullptr; }
  void SetDragon(Dragon *d) { pDrag = d; }
  bool IsBroken() const { return bBroken; }

private:
  int nPrincesses;
  LevelController *pAv;
  Dragon *pDrag;
  bool bBroken;
};

#endif
