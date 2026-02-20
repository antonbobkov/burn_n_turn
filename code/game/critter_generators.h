#ifndef TOWER_DEFENSE_CRITTER_GENERATORS_H
#define TOWER_DEFENSE_CRITTER_GENERATORS_H

#include "game_utils/image_sequence.h"
#include "entities.h"
#include "level.h"
#include "utils/smart_pointer.h"

struct LevelController;

/** Summons skeleton knights on a timer at a spot. */
struct SkellyGenerator : public EventEntity {
  std::string get_class_name() override { return "SkellyGenerator"; }
  Timer t;
  Point p;

  LevelController *pAdv;

  SkellyGenerator(Point p_, LevelController *pAdv_);

  /*virutal*/ void Update();
};

/** Summons knights along the path on a timer. */
struct KnightGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "KnightGenerator"; }
  bool bFirst;

  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  ImageSequence seq;

  Timer tm;

  BrokenLine bl;

  /** The current rate at which knights appear (by completion and ghost mode). */
  float GetRate();

  KnightGenerator(float dRate_, Rectangle rBound_, LevelController *pBc_,
                  const BrokenLine &bl_);

  /** Summon one knight (or golem or ghost) on the path and add to the realm. */
  void Generate(bool bGolem = false);

  /*virtual*/ void Update();
};

/** Summons princesses at a rate within the chapter bounds. */
struct PrincessGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "PrincessGenerator"; }
  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  Timer tm;
  bool bFirst;

  PrincessGenerator(float dRate_, Rectangle rBound_, LevelController *pBc_);

  /**
   * When the timer strikes: choose a road and summon a princess there, walking
   * toward the castles. The first princess bears the "capture" hint. Add her to
   * the realm, play the arrival tune, and tell the wise one.
   */
  /*virtual*/ void Update();
};

struct MageGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "MageGenerator"; }
  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  Timer tm;

  MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                 LevelController *pBc_);

  /*virtual*/ void Update();

  /** Summon one mage on a random road and add to the realm. */
  void MageGenerate();
};

/** Summons traders at a rate within the chapter bounds. */
struct TraderGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "TraderGenerator"; }
  float dRate;
  Rectangle rBound;
  LevelController *pBc;
  Timer tm;
  bool bFirst;
  bool bFirstBns;

  float GetRate();

  TraderGenerator(float dRate_, Rectangle rBound_, LevelController *pBc_);

  /*virtual*/ void Update();
};

#endif
