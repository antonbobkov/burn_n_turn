#ifndef GAME_RUNNER_INTERFACE_H
#define GAME_RUNNER_INTERFACE_H

#include "General.h"
#include "event.h"

#include "GuiGen.h"
#include "MessageWriter.h"
#include "SuiGen.h"

namespace Gui {
class FileManager;
struct ProgramInfo {
  Size szScreenRez;
  std::string strTitle;
  unsigned nFramerate;
  bool bMouseCapture;
  bool bFullScreen;
  bool bFlexibleResolution;
  bool bBlackBox;

  ProgramInfo()
      : bMouseCapture(false), bFullScreen(false), bFlexibleResolution(false) {}
  ProgramInfo(Size szScreenRez_, std::string strTitle_, unsigned nFramerate_,
              bool bMouseCapture_ = false, bool bFullScreen_ = false,
              bool bFlexibleResolution_ = false, bool bBlackBox_ = false)
      : szScreenRez(szScreenRez_), strTitle(strTitle_), nFramerate(nFramerate_),
        bBlackBox(bBlackBox_), bMouseCapture(bMouseCapture_),
        bFullScreen(bFullScreen_), bFlexibleResolution(bFlexibleResolution_) {}
};

} // namespace Gui

Gui::ProgramInfo GetProgramInfo();

namespace Gui {

struct ProgramEngine {
  SP<Event> pExitProgram;

  SP<GraphicalInterface<Index>> pGr;
  SP<SoundInterface<Index>> pSndMng;
  SP<MessageWriter> pMsg;

  FileManager *p_fm;

  Size szScreenRez;
  Size szActualRez;

  ProgramEngine(SP<Event> pExitProgram_, SP<GraphicalInterface<Index>> pGr_,
                SP<SoundInterface<Index>> pSndMng_, SP<MessageWriter> pMsg_,
                FileManager *p_fm_)
      : pExitProgram(pExitProgram_), pGr(pGr_), pSndMng(pSndMng_), pMsg(pMsg_),
        p_fm(p_fm_) {
    szScreenRez = GetProgramInfo().szScreenRez;
    szActualRez = szScreenRez;
  }

  ProgramEngine(SP<Event> pExitProgram_, SP<GraphicalInterface<Index>> pGr_,
                SP<SoundInterface<Index>> pSndMng_, SP<MessageWriter> pMsg_,
                Size szScreenRez_, FileManager *p_fm_)
      : pExitProgram(pExitProgram_), pGr(pGr_), pSndMng(pSndMng_), pMsg(pMsg_),
        p_fm(p_fm_), szScreenRez(szScreenRez_) {
    szActualRez = szScreenRez;
  }

  FileManager *GetFileManager() const { return p_fm; }
};

class GlobalController : virtual public SP_Info {
public:
  virtual void Update() = 0;

  virtual void KeyDown(GuiKeyType nCode) {}
  virtual void KeyUp(GuiKeyType nCode) {}
  virtual void MouseMove(Point p) {}
  virtual void MouseClick(Point p) {}
  virtual void MouseDown(Point p) {}
  virtual void MouseUp() {}
  virtual void DoubleClick() {}
  virtual void Fire() {}
};

} // namespace Gui

SP<Gui::GlobalController> GetGlobalController(Gui::ProgramEngine pe);

#endif // GAME_RUNNER_INTERFACE_H
