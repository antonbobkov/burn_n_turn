#include "simulation.h"

#include "MessageWriter.h"
#include "file_utils.h"
#include "game_runner_interface.h"

#include "GuiMock.h"
#include "SuiMock.h"

#include <memory>
#include <sstream>

using namespace Gui;

namespace {

const unsigned kSimulationFrames = 200;
const unsigned kScreenW = 960;
const unsigned kScreenH = 600;

void PopulateConfig(Gui::InMemoryFileManager &fm) {
  std::unique_ptr<Gui::OutStreamHandler> out = fm.WriteFile("config.txt");
  out->GetStream() << "SYSTEM 0\nPATH .\n";
}

void PopulateLevels(Gui::InMemoryFileManager &fm) {
  std::string level_content("LEVEL 1\n\n"
                            "FREQ 1 \n"
                            "SPWN &\n"
                            "CSTL \n"
                            "ROAD 0 0 \n"
                            "TIME 60\n\n");
  for (const std::string &key :
       {" .levels.txt", ".levels.txt", ".levels_trial.txt"}) {
    std::unique_ptr<Gui::OutStreamHandler> out = fm.WriteFile(key);
    out->GetStream() << level_content;
  }
}

void PopulateFont(Gui::InMemoryFileManager &fm, const std::string &key,
                  const std::string &content) {
  std::unique_ptr<Gui::OutStreamHandler> out = fm.WriteFile(key);
  out->GetStream() << content;
}

} // namespace

void RunSimulation() {
  srand(12345);

  SP<MockGraphicalInterface> p_mock_gr(new MockGraphicalInterface());
  SP<GraphicalInterface<Index>> p_gr(
      new SimpleGraphicalInterface<std::string>(p_mock_gr));

  SP<MockSoundInterface> p_mock_snd(new MockSoundInterface());
  SP<SoundInterface<Index>> p_snd(
      new SimpleSoundInterface<std::string>(p_mock_snd));

  std::unique_ptr<Gui::InMemoryFileManager> fm(new Gui::InMemoryFileManager());
  PopulateConfig(*fm);
  PopulateLevels(*fm);
  PopulateFont(*fm, " .dragonfont\\dragonfont.txt", " ");
  PopulateFont(*fm, " .dragonfont\\dragonfont2.txt", " ");
  PopulateFont(*fm, ".dragonfont\\dragonfont.txt", " ");
  PopulateFont(*fm, ".dragonfont\\dragonfont2.txt", " ");

  bool b_exit = false;
  bool b_true = true;
  SP<Event> p_exit_ev(NewSwitchEvent(b_exit, b_true));
  SP<MessageWriter> p_msg(new EmptyWriter());
  Size sz(kScreenW, kScreenH);

  ProgramEngine pe(p_exit_ev, p_gr, p_snd, p_msg, sz, fm.get());

  SP<GlobalController> p_gl = GetGlobalController(pe);

  for (unsigned i = 0; i < kSimulationFrames && !b_exit; ++i)
    p_gl->Update();
}
