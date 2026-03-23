#ifndef TOWER_DEFENSE_TUTORIAL_H
#define TOWER_DEFENSE_TUTORIAL_H

#include "../game_utils/draw_utils.h"
#include "entities.h"
#include <string>
#include <vector>

struct DragonGameController;

/** The wise one's scrolling words. SetText queues new lines; Update scrolls
 * between the old and the new. */
struct TutorialTextEntity : public Entity {
  std::string get_class_name() override { return "TutorialTextEntity"; }
  bool ShouldDraw() override { return true; }
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
        nDelta(1), nTextVerticalSpacing(7), pGl(pGl_) {}

  /** Set the scroll's words to v; scroll from current lines to v or set if
   * empty. */
  void SetText(std::vector<std::string> v);

  /** Draw the lines with a vertical offset; skip if the wise one's guidance is
   * off. */
  void Draw(ScalingDrawer *pDr) override;

  float GetPriority() override { return dPriority; }

  /** Advance the scroll each tick; when it ends, the new words replace the
   * old. */
  void Update() override;
};

/** Game event that can trigger a tutorial state change. */
enum class TutorialEvent {
  KnightKilled,
  FlyOn,
  FlyOff,
  PrincessCaptured,
  PrincessGenerate,
  ShotFired,
  BonusPickUp,
  TraderKilled,
  TraderGenerate,
};

/** Abstract tutorial handler: receives game events and decides what to show.
 * Subclasses implement GetText() to describe the current state; Update()
 * pushes it to the text entity. */
class Tutorial {
 public:
  explicit Tutorial(TutorialTextEntity *tutorial_text)
      : tutorial_text_(tutorial_text) {}
  virtual ~Tutorial() = default;
  virtual void Notify(TutorialEvent event) = 0;

 protected:
  /** Push GetText() result to the text entity. */
  void UpdateTutorialText();

 private:
  virtual std::vector<std::string> GetText() { return {}; }
  TutorialTextEntity *tutorial_text_;
};

/** Tutorial that does nothing; used for levels without tutorial guidance. */
class NoopTutorial : public Tutorial {
 public:
  NoopTutorial() : Tutorial(nullptr) {}
  void Notify(TutorialEvent /*event*/) override {}
};

/** First chapter tutorial: tracks knight, flight, and princess milestones. */
class TutorialLevelOne : public Tutorial {
 public:
  TutorialLevelOne(TutorialTextEntity *tutorial_text, bool is_joystick,
                   bool is_keyboard);
  void Notify(TutorialEvent event) override;

 private:
  bool killed_knight_;
  bool flying_;
  bool princess_generated_;
  bool princess_captured_;
  std::string steer_message_;
  std::string shooting_message_;
  std::string take_off_message_;
  bool show_flying_shoot_hint_;

  std::vector<std::string> GetText() override;
};

/** Second chapter tutorial: tracks trader appearance and bonus milestones. */
class TutorialLevelTwo : public Tutorial {
 public:
  explicit TutorialLevelTwo(TutorialTextEntity *tutorial_text);
  void Notify(TutorialEvent event) override;

 private:
  bool trader_generated_;
  bool trader_killed_;
  bool bonus_picked_up_;

  std::vector<std::string> GetText() override;
};

#endif
