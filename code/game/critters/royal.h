#ifndef TOWER_DEFENSE_CRITTERS_ROYAL_H
#define TOWER_DEFENSE_CRITTERS_ROYAL_H

#include "critters.h"

#include <memory>

class LevelController;

/** The royal: a princess or prince the dragon can rescue and carry to the
 * castle. */
class Royal : public Critter {
public:
  std::string get_class_name() override { return "Royal"; }

  Royal(const Critter &cr, LevelController *pAc_, RoyalKind kind)
      : Critter(cr), pAc(pAc_), kind_(kind) {}

  Index GetImage() override { return seq.GetImageAt(0); }

  RoyalKind GetRoyalKind() const override { return kind_; }

  std::string GetType() override { return "royal"; }

  /** When hit: show the bonus tally and vanish from the world. */
  void OnHit(char cWhat) override;

  void Draw(ScalingDrawer *pDr) override;

private:
  LevelController *pAc;
  /** The bloodline this royal was born to, which never changes. */
  RoyalKind kind_;
};

/** The fates flip a coin: a princess or a prince, each as likely as the other. */
RoyalKind RandomRoyalKind();

/** Summon a royal of the given kind, facing the way they walk. */
std::unique_ptr<Royal> MakeRoyal(RoyalKind kind, fPoint pos, fPoint vel,
                                 Rectangle bound, float priority,
                                 LevelController *level);

#endif
