#ifndef GAME_RUNNER_INTERFACE_H
#define GAME_RUNNER_INTERFACE_H

#include "event.h"
#include "utils/smart_pointer.h"
#include "wrappers/gui_key_type.h"

#include "MessageWriter.h"
#include "wrappers/GuiGen.h"
#include "wrappers/SuiGen.h"

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

ProgramInfo GetProgramInfo();

struct ProgramEngine {
  smart_pointer<Event> pExitProgram;

  smart_pointer<GraphicalInterface<Index>> pGr;
  smart_pointer<SoundInterface<Index>> pSndMng;
  smart_pointer<MessageWriter> pMsg;

  FileManager *p_fm;

  Size szScreenRez;
  Size szActualRez;

  ProgramEngine(smart_pointer<Event> pExitProgram_,
                smart_pointer<GraphicalInterface<Index>> pGr_,
                smart_pointer<SoundInterface<Index>> pSndMng_,
                smart_pointer<MessageWriter> pMsg_, FileManager *p_fm_)
      : pExitProgram(pExitProgram_), pGr(pGr_), pSndMng(pSndMng_), pMsg(pMsg_),
        p_fm(p_fm_) {
    szScreenRez = GetProgramInfo().szScreenRez;
    szActualRez = szScreenRez;
  }

  ProgramEngine(smart_pointer<Event> pExitProgram_,
                smart_pointer<GraphicalInterface<Index>> pGr_,
                smart_pointer<SoundInterface<Index>> pSndMng_,
                smart_pointer<MessageWriter> pMsg_, Size szScreenRez_,
                FileManager *p_fm_)
      : pExitProgram(pExitProgram_), pGr(pGr_), pSndMng(pSndMng_), pMsg(pMsg_),
        p_fm(p_fm_), szScreenRez(szScreenRez_) {
    szActualRez = szScreenRez;
  }

  FileManager *GetFileManager() const { return p_fm; }
};

class GameRunner : virtual public SP_Info {
public:
  std::string get_class_name() override { return "GameRunner"; }
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

smart_pointer<GameRunner> GetGameRunner(ProgramEngine pe);

#endif // GAME_RUNNER_INTERFACE_H
