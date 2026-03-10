#ifndef TOWER_DEFENSE_DRAGON_GAME_RUNNER_H
#define TOWER_DEFENSE_DRAGON_GAME_RUNNER_H

#include "../game_utils/game_runner_interface.h"
#include "../utils/configuration_file.h"
#include "../wrappers/geometry.h"
#include "../utils/smart_pointer.h"
#include <memory>
#include <vector>

struct DragonGameController;
class FilePath;
struct LevelLayout;
class FileManager;
struct FontWriter;
struct ScalingDrawer;
struct NumberDrawer;

/** The top of the tower: holds the chest, builds the chapters and runs the
 * tale. */
class DragonGameRunner : public GameRunner {
public:
  std::string get_class_name() override { return "DragonGameRunner"; }
  DragonGameRunner(ProgramEngine const& pe);
  ~DragonGameRunner();

  void Update() override;

  void KeyDown(GuiKeyType nCode) override;
  void KeyUp(GuiKeyType nCode) override;
  void MouseMove(Point pPos) override;
  void MouseDown(Point pPos) override;
  void MouseUp() override;
  void DoubleClick() override;
  void Fire() override;

  /** For scribes and trials: who holds the tower now (levels, menu, etc.). */
  DragonGameController *GetTowerController() const;
  /** Name of the screen before you (e.g. menu, start, level). */
  std::string GetActiveControllerName() const;
  /** True when the dragon has claimed the day (for the trial of simulation). */
  bool IsOnGameOverScreen() const;

private:
  Event *pExitProgram;

  GraphicalInterface<Index> *pGr;
  SoundInterface<Index> *pSm;

  std::unique_ptr<ScalingDrawer> pDr;
  std::unique_ptr<ScalingDrawer> pBigDr;
  std::unique_ptr<NumberDrawer> pNum;
  std::unique_ptr<NumberDrawer> pBigNum;
  std::unique_ptr<FontWriter> pFancyNum;

  FileManager *p_fm_;
  std::unique_ptr<ConfigurationFile> config_;
  std::unique_ptr<ConfigurationFile> game_data_;
  std::unique_ptr<FilePath> fp_;

  std::vector<LevelLayout> vLvl;
  std::unique_ptr<DragonGameController> pCnt;

  Size szActualRez;
};

#endif
