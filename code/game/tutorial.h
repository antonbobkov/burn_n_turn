#ifndef TOWER_DEFENSE_TUTORIAL_H
#define TOWER_DEFENSE_TUTORIAL_H

#include "game_utils/draw_utils.h"
#include "entities.h"

struct DragonGameController;

/** The wise one's scrolling words. SetText queues new lines; Update scrolls
 * between the old and the new. */
struct TutorialTextEntity : virtual public EventEntity, public VisualEntity {
  std::string get_class_name() override { return "TutorialTextEntity"; }
  float dPriority;
  Point pos;

  std::vector<std::string> sText;
  std::vector<std::string> sNewText;

  NumberDrawer *pNum;

  Timer t;
  int nOffset;
  int nDelta;

  int nTextVerticalSpacing;

  struct DragonGameController *pGl;

  TutorialTextEntity(float dPriority_, Point pos_, NumberDrawer *pNum_,
                     DragonGameController *pGl_)
      : dPriority(dPriority_), pos(pos_), pNum(pNum_), t(1), nOffset(0),
        nTextVerticalSpacing(7), nDelta(1), pGl(pGl_) {}

  /** Set the scroll's words to v; scroll from current lines to v or set if
   * empty. */
  void SetText(std::vector<std::string> v);

  /** Draw the lines with a vertical offset; skip if the wise one's guidance is
   * off. */
  /*virtual*/ void Draw(ScalingDrawer *pDr);

  /*virtual*/ float GetPriority() { return dPriority; }

  /** Advance the scroll each tick; when it ends, the new words replace the
   * old. */
  /*virtual*/ void Update();
};

/** First tale: tracks when the knight falls, when the dragon flies, when the
 * princess appears or is captured; GetText/Update feed the scrolling words. */
struct TutorialLevelOne {
  bool bKilledKnight;
  bool bFlying;
  bool bPrincessGenerated;
  bool bPrincessCaptured;

  TutorialTextEntity *pTexter;

  TutorialLevelOne()
      : bKilledKnight(false), bFlying(false), bPrincessGenerated(false),
        bPrincessCaptured(false), pTexter() {}

  /** Choose which lines the wise one speaks for the first chapter. */
  std::vector<std::string> GetText();

  /** Send the current lines to the scroll. */
  void Update();

  void ShotFired() {}

  /** Mark that the knight fell and refresh the wise one's words. */
  void KnightKilled();

  /** Mark that the dragon took flight and refresh the words. */
  void FlyOn();

  /** Mark that the dragon landed and refresh the words. */
  void FlyOff();

  /** Mark that the princess appeared and refresh the words. */
  void PrincessGenerate();

  /** Mark that the princess was captured and refresh the words. */
  void PrincessCaptured();
};

/** Second tale: trader appearing and falling, bonus picked up; GetText/Update
 * feed the scrolling words. */
struct TutorialLevelTwo {
  bool bTraderGenerated;
  bool bTraderKilled;
  bool bBonusPickedUp;

  TutorialTextEntity *pTexter;

  TutorialLevelTwo()
      : bTraderGenerated(false), bTraderKilled(false), bBonusPickedUp(false),
        pTexter() {}

  /** Build the wise one's lines from trader and bonus state. */
  std::vector<std::string> GetText();

  /** Send the current lines to the scroll. */
  void Update();

  /** Mark that the trader fell and refresh the words. */
  void TraderKilled();

  /** Mark that the trader appeared and refresh the words. */
  void TraderGenerate();

  /** Mark that a bonus was picked up and refresh the words. */
  void BonusPickUp();
};

#endif
