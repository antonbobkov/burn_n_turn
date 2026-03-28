#ifndef TOWER_DEFENSE_CRITTER_GENERATORS_H
#define TOWER_DEFENSE_CRITTER_GENERATORS_H

#include "../game_utils/image_sequence.h"
#include "entities.h"
#include "level.h"

class LevelController;

/** Summons skeleton knights on a timer at a spot. */
class SkellyGenerator : public Entity {
public:
  std::string get_class_name() override { return "SkellyGenerator"; }

  SkellyGenerator(Point p_, LevelController *pAdv_);

  void Update() override;

private:
  Timer t;
  Point p;
  LevelController *pAdv;
};

/** Summons knights along the path on a timer. */
class FighterGenerator : public Entity {
public:
  std::string get_class_name() override { return "FighterGenerator"; }

  /** The current rate at which knights appear (by completion and ghost mode). */
  float GetRate();

  FighterGenerator(float dRate_, Rectangle rBound_, LevelController *pBc_,
                  const BrokenLine &bl_);

  /** Summon one knight (or golem or ghost) on the path and add to the realm. */
  void Generate(bool bGolem = false);

  void Update() override;

private:
  bool bFirst;
  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  ImageSequence seq;
  Timer tm;
  BrokenLine bl;
};

/** Summons princesses at a rate within the chapter bounds. */
class PrincessGenerator : public Entity {
public:
  std::string get_class_name() override { return "PrincessGenerator"; }

  PrincessGenerator(float dRate_, Rectangle rBound_, LevelController *pBc_);

  /**
   * When the timer strikes: choose a road and summon a princess there, walking
   * toward the castles. The first princess bears the "capture" hint. Add her to
   * the realm, play the arrival tune, and tell the wise one.
   */
  void Update() override;

private:
  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  Timer tm;
  bool bFirst;
};

class MageGenerator : public Entity {
public:
  std::string get_class_name() override { return "MageGenerator"; }

  MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                 LevelController *pBc_);

  void Update() override;

  /** Summon one mage on a random road and add to the realm. */
  void MageGenerate();

private:
  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  Timer tm;
};

/** Summons traders at a rate within the chapter bounds. */
class TraderGenerator : public Entity {
public:
  std::string get_class_name() override { return "TraderGenerator"; }

  float GetRate();

  TraderGenerator(float dRate_, Rectangle rBound_, LevelController *pBc_);

  void Update() override;

private:
  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  Timer tm;
  bool bFirst;
  bool bFirstBns;
};

#endif
