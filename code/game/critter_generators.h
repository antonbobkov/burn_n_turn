#ifndef TOWER_DEFENSE_CRITTER_GENERATORS_H
#define TOWER_DEFENSE_CRITTER_GENERATORS_H

#include "entities.h"
#include "level.h"
#include "smart_pointer.h"

struct LevelController;

/** Spawns skeleton knights on a timer at a position. */
struct SkellyGenerator : public EventEntity {
  std::string get_class_name() override { return "SkellyGenerator"; }
  Timer t;
  Point p;

  SSP<LevelController> pAdv;

  SkellyGenerator(Point p_, smart_pointer<LevelController> pAdv_);

  /*virutal*/ void Update();
};

/** Spawns knights along a path on a timer. */
struct KnightGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "KnightGenerator"; }
  bool bFirst;

  float dRate;
  Rectangle rBound;
  SSP<LevelController> pBc;
  ImageSequence seq;

  Timer tm;

  BrokenLine bl;

  /** Returns the current spawn rate (depends on completion and ghost mode). */
  float GetRate();

  KnightGenerator(float dRate_, Rectangle rBound_,
                  smart_pointer<LevelController> pBc_, const BrokenLine &bl_);

  /** Spawn one Knight (or Golem/ghost) on bl; add to pBc. */
  void Generate(bool bGolem = false);

  /*virtual*/ void Update();
};

/** Spawns princesses at a rate within bounds. */
struct PrincessGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "PrincessGenerator"; }
  float dRate;
  Rectangle rBound;
  SSP<LevelController> pBc;
  Timer tm;
  bool bFirst;

  PrincessGenerator(float dRate_, Rectangle rBound_,
                    smart_pointer<LevelController> pBc_);

  /**
   * When the timer fires: pick a random road and spawn a princess there,
   * moving toward the castles. The first princess gets a "capture" hint.
   * Add her to the game, play the arrival sound, and notify the tutorial.
   */
  /*virtual*/ void Update();
};

struct MageGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "MageGenerator"; }
  float dRate;
  Rectangle rBound;
  SSP<LevelController> pBc;
  Timer tm;

  MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                smart_pointer<LevelController> pBc_);

  /*virtual*/ void Update();

  /** Spawn one Mage on a random road; add to pBc. */
  void MageGenerate();
};

/** Spawns traders at a rate within bounds. */
struct TraderGenerator : virtual public EventEntity {
  std::string get_class_name() override { return "TraderGenerator"; }
  float dRate;
  Rectangle rBound;
  SSP<LevelController> pBc;
  Timer tm;
  bool bFirst;
  bool bFirstBns;

  float GetRate();

  TraderGenerator(float dRate_, Rectangle rBound_,
                  smart_pointer<LevelController> pBc_);

  /*virtual*/ void Update();
};

#endif
