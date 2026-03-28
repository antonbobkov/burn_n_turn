#ifndef TOWER_DEFENSE_CRITTERS_SLIME_H
#define TOWER_DEFENSE_CRITTERS_SLIME_H

#include "critters.h"
#include <memory>
#include <vector>

class LevelController;

/** The great slime: it may split or merge in the dance of the MegaSlime. */
class MegaSlime : public Critter {
public:
  std::string get_class_name() override { return "MegaSlime"; }

  MegaSlime(fPoint fPos, Rectangle rBound, LevelController *pAc_);

  void Draw(ScalingDrawer *pDr) override { Critter::Draw(pDr); }

  void RandomizeVelocity();

  /**
   * Clear bonuses it has swallowed. When it touches a pickup, the treasure
   * is gone and a sound plays. Each tick: advance the dance; on jump and land,
   * play sounds and either change direction or stand still.
   */
  void Update() override;

  void OnHit(char cWhat) override;

  Index GetImage() override { return seq.GetImageAt(0); }

  std::string GetType() override { return "megaslime"; }

private:
  LevelController *pAc;
  int nHealth;
};

/** A slime: it drifts toward its prey, a timer guiding its steps. */
class Slime : public Critter {
public:
  std::string get_class_name() override { return "Slime"; }

  Slime(fPoint fPos, Rectangle rBound, LevelController *pAc_, int nGeneration_);

  int GetGeneration() { return nGeneration; }

  void RandomizeVelocity();

  void Update() override;

  void OnHit(char cWhat) override;

  Index GetImage() override { return seq.GetImageAt(0); }

  std::string GetType() override { return "slime"; }

  ~Slime();

private:
  LevelController *pAc;
  Timer t;
  int nGeneration;
};

/** Waits on a timer at a spot, then vanishes and summons a slime. */
class SpawningSlime : public Entity {
public:
  std::string get_class_name() override { return "SpawningSlime"; }

  SpawningSlime(Point p_, LevelController *pAdv_, bool bFast_, int nGeneration_);

  void Update() override;

  /** End the summoning; the spawn and its shimmer are gone. */
  void Destroy();

  Point GetPosition() override { return p; }

  /** If the shimmer animation exists, add it to out (used when building the
   * full entity draw list). */
  void AppendSpawnAnimation(std::vector<Entity *> &out) {
    if (pSlm_)
      out.push_back(pSlm_.get());
  }

  ~SpawningSlime();

private:
  Timer t;
  Point p;
  bool bFast;
  int nGeneration;
  LevelController *pAdv;
  std::unique_ptr<AnimationOnce> pSlm_;
};

/** Waits for a shimmer animation to finish, then vanishes and summons a mega slime. */
class SpawningMegaSlime : public Entity {
public:
  std::string get_class_name() override { return "SpawningMegaSlime"; }

  SpawningMegaSlime(Point p_, LevelController *pAdv_);

  void Update() override;

  Point GetPosition() override { return p; }
  void Destroy();

  /** If the shimmer animation exists, add it to out (used when building the
   * full entity draw list). */
  void AppendSpawnAnimation(std::vector<Entity *> &out) {
    if (pSlm_)
      out.push_back(pSlm_.get());
  }

private:
  Point p;
  LevelController *pAdv;
  std::unique_ptr<AnimationOnce> pSlm_;
};

/** A drifting slime that moves with position and velocity (e.g. in the menu). */
class FloatingSlime : public SimpleVisualEntity {
public:
  std::string get_class_name() override { return "FloatingSlime"; }

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, int nTime);

  Point GetPosition() override { return fPos.ToPnt(); }
  void Update() override;

private:
  fPoint fPos;
  fPoint fVel;
  Timer tTermination;
};

#endif
