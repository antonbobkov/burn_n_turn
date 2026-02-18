#ifndef GAME_RUNNER_INTERFACE_H
#define GAME_RUNNER_INTERFACE_H

#include "event.h"
#include "utils/smart_pointer.h"
#include "wrappers/gui_key_type.h"

#include <memory>
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
  std::unique_ptr<Event> pExitProgram;

  GraphicalInterface<Index> *pGr;
  SoundInterface<Index> *pSndMng;
  std::unique_ptr<MessageWriter> pMsg;

  FileManager *p_fm;

  Size szScreenRez;
  Size szActualRez;

  ProgramEngine(std::unique_ptr<Event> pExitProgram_,
                GraphicalInterface<Index> *pGr_,
                SoundInterface<Index> *pSndMng_,
                std::unique_ptr<MessageWriter> pMsg_, FileManager *p_fm_)
      : pExitProgram(std::move(pExitProgram_)), pGr(pGr_), pSndMng(pSndMng_),
        pMsg(std::move(pMsg_)), p_fm(p_fm_) {
    szScreenRez = GetProgramInfo().szScreenRez;
    szActualRez = szScreenRez;
  }

  ProgramEngine(std::unique_ptr<Event> pExitProgram_,
                GraphicalInterface<Index> *pGr_,
                SoundInterface<Index> *pSndMng_,
                std::unique_ptr<MessageWriter> pMsg_, Size szScreenRez_,
                FileManager *p_fm_)
      : pExitProgram(std::move(pExitProgram_)), pGr(pGr_), pSndMng(pSndMng_),
        pMsg(std::move(pMsg_)), p_fm(p_fm_), szScreenRez(szScreenRez_) {
    szActualRez = szScreenRez;
  }

  ProgramEngine(ProgramEngine const&) = delete;
  ProgramEngine& operator=(ProgramEngine const&) = delete;

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

smart_pointer<GameRunner> GetGameRunner(ProgramEngine const& pe);

#endif // GAME_RUNNER_INTERFACE_H
