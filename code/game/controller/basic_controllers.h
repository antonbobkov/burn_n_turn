#ifndef TOWER_DEFENSE_BASIC_CONTROLLERS_H
#define TOWER_DEFENSE_BASIC_CONTROLLERS_H

#include "entity_list_controller.h"
#include "../../utils/timer.h"
#include <memory>

struct Animation;
struct FancyCritter;

struct StartScreenController : public EntityListController {
  std::string get_class_name() { return "StartScreenController"; }
  StartScreenController(DragonGameController *pGl_, Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  /** Step to the next screen and let the bard play the start_game tune. */
  void Next();

  void OnKey(GuiKeyType c, bool bUp) override;

  void OnMouseDown(Point /*pPos*/) override { Next(); }
  std::string GetControllerName() const override { return "start"; }
};

struct Cutscene : public EntityListController {
  std::string get_class_name() { return "Cutscene"; }

  /**
   * Set the stage: dark veil, one soul running and one waiting. Runner starts
   * left or right by flip; when the runner reaches the middle, the chaser is
   * unleashed. A timer drives the beeps and boops.
   */
  Cutscene(DragonGameController *pGl_, Rectangle rBound_, std::string sRun,
           std::string sChase, bool bFlip = false);
  ~Cutscene();

private:
  std::unique_ptr<FancyCritter> pCrRun;
  std::unique_ptr<FancyCritter> pCrFollow;

  Timer tm;
  bool Beepy;

  bool bRelease;

  void Update() override;
  void OnKey(GuiKeyType c, bool bUp) override;
  void OnMouseDown(Point /*pPos*/) override {}
  std::string GetControllerName() const override { return "cutscene"; }

  std::vector<Entity *> GetNonOwnedEntities() override;
};

/** A screen that shows the dragon's tally and leaves on click or when time
 * runs out. */
struct DragonScoreController : public EntityListController {
  std::string get_class_name() { return "DragonScoreController"; }

  DragonScoreController(DragonGameController *pGl_, Rectangle rBound, Color c,
                        bool bScoreShow);

private:
  Timer t;
  bool bClickToExit;

  void OnKey(GuiKeyType c, bool bUp) override;
  void Update() override;
  void DoubleClick() override;

  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "score"; }
};

/** A controller that steps to the next screen when only the veil remains, or
 * when the hero gives a sign. */
struct AutoAdvanceController : public EntityListController {
  std::string get_class_name() { return "AutoAdvanceController"; }
  AutoAdvanceController(DragonGameController *pGl_, Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  void Update() override;

  void OnKey(GuiKeyType c, bool bUp) override;

  void OnMouseDown(Point pPos) override;
  std::string GetControllerName() const override { return "logo"; }
};

#endif
