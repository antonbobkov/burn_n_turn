#include "simulation.h"

#include "MessageWriter.h"
#include "file_utils.h"
#include "game_runner_interface.h"

#include "GuiMock.h"
#include "SuiMock.h"

#include <iostream>
#include <memory>

using namespace Gui;

namespace {

const unsigned kSimulationFrames = 200;
const unsigned kScreenW = 960;
const unsigned kScreenH = 600;

} // namespace

void RunSimulation() {
  std::cout << "[sim] Starting simulation (seed 12345)\n";
  srand(12345);

  std::cout << "[sim] Creating mocks and file manager\n";
  SP<MockGraphicalInterface> p_mock_gr(new MockGraphicalInterface());
  SP<GraphicalInterface<Index>> p_gr(
      new SimpleGraphicalInterface<std::string>(p_mock_gr));

  SP<MockSoundInterface> p_mock_snd(new MockSoundInterface());
  SP<SoundInterface<Index>> p_snd(
      new SimpleSoundInterface<std::string>(p_mock_snd));

  std::unique_ptr<Gui::StdFileManager> underlying(new Gui::StdFileManager());
  std::unique_ptr<Gui::CachingReadOnlyFileManager> fm(
      new Gui::CachingReadOnlyFileManager(underlying.get(), ".txt"));

  bool b_exit = false;
  bool b_true = true;
  SP<Event> p_exit_ev(NewSwitchEvent(b_exit, b_true));
  SP<MessageWriter> p_msg(new EmptyWriter());
  Size sz(kScreenW, kScreenH);

  std::cout << "[sim] Creating ProgramEngine and GlobalController\n";
  ProgramEngine pe(p_exit_ev, p_gr, p_snd, p_msg, sz, fm.get());
  SP<GlobalController> p_gl = GetGlobalController(pe);

  std::cout << "[sim] Running " << kSimulationFrames << " frames\n";
  for (unsigned i = 0; i < kSimulationFrames && !b_exit; ++i) {
    if (i == 0 || (i + 1) % 50 == 0 || i == kSimulationFrames - 1)
      std::cout << "[sim] Frame " << (i + 1) << "/" << kSimulationFrames
                << "\n";
    p_gl->Update();
  }
  std::cout << "[sim] Done\n";
}
