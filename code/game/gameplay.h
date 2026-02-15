#ifndef TOWER_DEFENSE_GAMEPLAY_H
#define TOWER_DEFENSE_GAMEPLAY_H

#include "game/controller/menu_controller.h"
#include "game_utils/draw_utils.h"
#include "game_utils/sound_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"
#include "wrappers/gui_key_type.h"

/** Included after critter_generators.h and dragon.h from game.h. */
struct DragonGameController;
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
  DragonGameController *pGl;
  Rectangle rBound;

  HighScoreShower(DragonGameController *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

struct IntroTextShower : public VisualEntity {
  std::string get_class_name() override { return "IntroTextShower"; }
  DragonGameController *pGl;
  Rectangle rBound;

  IntroTextShower(DragonGameController *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

#endif
