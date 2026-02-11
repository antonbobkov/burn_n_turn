#ifndef TOWER_DEFENSE_TUTORIAL_H
#define TOWER_DEFENSE_TUTORIAL_H

#include "entities.h"
#include "smart_pointer.h"

/** Scrolling tutorial text; SetText queues sNewText and Update scrolls between
 * sText and sNewText. */
struct TutorialTextEntity : virtual public EventEntity, public VisualEntity {
  float dPriority;
  Point pos;

  std::vector<std::string> sText;
  std::vector<std::string> sNewText;

  SSP<NumberDrawer> pNum;

  Timer t;
  int nOffset;
  int nDelta;

  int nTextVerticalSpacing;

  const bool *pIsTutorialOn;

  TutorialTextEntity(float dPriority_, Point pos_,
                     smart_pointer<NumberDrawer> pNum_,
                     const bool *pIsTutorialOn_)
      : dPriority(dPriority_), pos(pos_), pNum(this, pNum_), t(1), nOffset(0),
        nTextVerticalSpacing(7), nDelta(1), pIsTutorialOn(pIsTutorialOn_) {}

  /** Set content to v; scrolls from sText to v or sets sText if empty. */
  void SetText(std::vector<std::string> v);

  /** Draw sText lines with vertical offset nOffset; skip if tutorial off. */
  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return dPriority; }

  /** Advances the scroll each tick; when scroll ends, the new text replaces
   * the old. */
  /*virtual*/ void Update();
};

/** First tutorial: tracks knight kill, flying, princess spawn/capture;
 * GetText/Update drive TutorialTextEntity. */
struct TutorialLevelOne {
  bool bKilledKnight;
  bool bFlying;
  bool bPrincessGenerated;
  bool bPrincessCaptured;

  smart_pointer<TutorialTextEntity> pTexter;

  TutorialLevelOne()
      : bKilledKnight(false), bFlying(false), bPrincessGenerated(false),
        bPrincessCaptured(false), pTexter() {}

  /** Chooses which tutorial lines to show for level one. */
  std::vector<std::string> GetText();

  /** Pushes the current tutorial lines to the display. */
  void Update();

  void ShotFired() {}

  /** Set bKilledKnight and refresh tutorial text. */
  void KnightKilled();

  /** Set flying on and refresh text. */
  void FlyOn();

  /** Set flying off and refresh text. */
  void FlyOff();

  /** Mark princess spawned and refresh text. */
  void PrincessGenerate();

  /** Mark princess captured and refresh text. */
  void PrincessCaptured();
};

/** Second tutorial: trader spawn/kill and bonus pickup; GetText/Update drive
 * TutorialTextEntity. */
struct TutorialLevelTwo {
  bool bTraderGenerated;
  bool bTraderKilled;
  bool bBonusPickedUp;

  smart_pointer<TutorialTextEntity> pTexter;

  TutorialLevelTwo()
      : bTraderGenerated(false), bTraderKilled(false), bBonusPickedUp(false),
        pTexter() {}

  /** Build tutorial lines from trader/bonus state. */
  std::vector<std::string> GetText();

  /** Pushes the current tutorial lines to the display. */
  void Update();

  /** Set bTraderKilled and refresh text. */
  void TraderKilled();

  /** Mark trader spawned and refresh text. */
  void TraderGenerate();

  /** Mark bonus picked up and refresh text. */
  void BonusPickUp();
};

#endif
