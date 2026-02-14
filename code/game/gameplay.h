#ifndef TOWER_DEFENSE_GAMEPLAY_H
#define TOWER_DEFENSE_GAMEPLAY_H

#include "wrappers/color.h"
#include "game/controller/menu_controller.h"
#include "wrappers/gui_key_type.h"
#include "utils/smart_pointer.h"

/** Included after critter_generators.h and dragon.h from game.h. */
struct LevelController;

/** Floating "+N" score text at a point; animates then removes. */
struct BonusScore : public EventEntity, public VisualEntity {
  std::string get_class_name() override { return "BonusScore"; }
  LevelController *pAc;
  std::string sText;
  unsigned nScore;
  unsigned nScoreSoFar;
  Point p;
  Timer t;
  unsigned nC;
  Color c;

  BonusScore(LevelController *pAc_, Point p_, unsigned nScore_);

  /*virtual*/ void Update();

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return 5; }

  /*virtual*/ Point GetPosition() { return p; }
};

struct SoundControls : public EventEntity {
  std::string get_class_name() override { return "SoundControls"; }
  BackgroundMusicPlayer &plr;
  int nTheme;

  SoundControls(BackgroundMusicPlayer &plr_, int nTheme_)
      : plr(plr_), nTheme(nTheme_) {}

  /*virtual*/ void Update();
};

/** Draws high score in a rectangle. */
struct HighScoreShower : public VisualEntity {
  std::string get_class_name() override { return "HighScoreShower"; }
  DragonGameControllerList *pGl;
  Rectangle rBound;

  HighScoreShower(DragonGameControllerList *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

struct IntroTextShower : public VisualEntity {
  std::string get_class_name() override { return "IntroTextShower"; }
  DragonGameControllerList *pGl;
  Rectangle rBound;

  IntroTextShower(DragonGameControllerList *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

#endif
