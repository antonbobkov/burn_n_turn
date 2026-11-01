#ifndef TOWER_DEFENSE_CASTLE_H
#define TOWER_DEFENSE_CASTLE_H

#include "critters/critters.h"
#include <string>
#include <vector>

class LevelController;
class Dragon;

class Castle : public Critter {
public:
  std::string get_class_name() override { return "Castle"; }

  Castle(Point p, Rectangle rBound_, LevelController *pAv_);

  std::string GetType() override { return "castle"; }
  void OnKnight(std::string cWhat);
  void Draw(ScalingDrawer *pDr) override;

  int GetRoyalCount() const { return (int)vRoyals.size(); }
  /** The royals held in this tower, oldest capture first. */
  const std::vector<RoyalKind> &GetRoyals() const { return vRoyals; }
  /** Welcome newly captured royals; they take the next free windows in turn. */
  void AddRoyals(const std::vector<RoyalKind> &royals);
  bool HasDragon() const { return pDrag != nullptr; }
  void SetDragon(Dragon *d) { pDrag = d; }
  bool IsBroken() const { return bBroken; }

private:
  std::vector<RoyalKind> vRoyals;
  LevelController *pAv;
  Dragon *pDrag;
  bool bBroken;
};

#endif
