#ifndef GAME_RUNNER_INTERFACE_H
#define GAME_RUNNER_INTERFACE_H

#include "event.h"
#include "../wrappers/gui_key_type.h"

#include <memory>
#include "MessageWriter.h"
#include "../wrappers/GuiGen.h"
#include "../wrappers/SuiGen.h"

class FileManager;
class ProgramInfo {
public:
  ProgramInfo()
      : bMouseCapture(false), bFullScreen(false), bFlexibleResolution(false),
        bBlackBox(false), nFramerate(0) {}
  ProgramInfo(Size szScreenRez_, std::string strTitle_, int nFramerate_,
              bool bMouseCapture_ = false, bool bFullScreen_ = false,
              bool bFlexibleResolution_ = false, bool bBlackBox_ = false)
      : szScreenRez(szScreenRez_), strTitle(strTitle_), nFramerate(nFramerate_),
        bMouseCapture(bMouseCapture_), bFullScreen(bFullScreen_),
        bFlexibleResolution(bFlexibleResolution_), bBlackBox(bBlackBox_) {}

  Size GetScreenRez() const { return szScreenRez; }
  std::string GetTitle() const { return strTitle; }
  int GetFramerate() const { return nFramerate; }
  bool IsMouseCapture() const { return bMouseCapture; }
  bool IsFullScreen() const { return bFullScreen; }
  bool IsFlexibleResolution() const { return bFlexibleResolution; }
  bool IsBlackBox() const { return bBlackBox; }

private:
  Size szScreenRez;
  std::string strTitle;
  int nFramerate;
  bool bMouseCapture;
  bool bFullScreen;
  bool bFlexibleResolution;
  bool bBlackBox;
};

ProgramInfo GetProgramInfo();

class ProgramEngine {
public:
  ProgramEngine(std::unique_ptr<Event> pExitProgram_,
                GraphicalInterface<Index> *pGr_,
                SoundInterface<Index> *pSndMng_,
                std::unique_ptr<MessageWriter> pMsg_, FileManager *p_fm_)
      : pExitProgram(std::move(pExitProgram_)), pGr(pGr_), pSndMng(pSndMng_),
        pMsg(std::move(pMsg_)), p_fm(p_fm_) {
    szScreenRez = GetProgramInfo().GetScreenRez();
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
  Event *GetExitProgram() const { return pExitProgram.get(); }
  GraphicalInterface<Index> *GetGraphics() const { return pGr; }
  SoundInterface<Index> *GetSound() const { return pSndMng; }
  MessageWriter *GetMessageWriter() const { return pMsg.get(); }
  Size GetScreenRez() const { return szScreenRez; }
  Size GetActualRez() const { return szActualRez; }

private:
  std::unique_ptr<Event> pExitProgram;
  GraphicalInterface<Index> *pGr;
  SoundInterface<Index> *pSndMng;
  std::unique_ptr<MessageWriter> pMsg;
  FileManager *p_fm;
  Size szScreenRez;
  Size szActualRez;
};

class GameRunner {
public:
  virtual ~GameRunner() = default;
  virtual void Update() = 0;

  virtual void KeyDown(GuiKeyType /*nCode*/) {}
  virtual void KeyUp(GuiKeyType /*nCode*/) {}
  virtual void MouseMove(Point /*p*/) {}
  virtual void MouseClick(Point /*p*/) {}
  virtual void MouseDown(Point /*p*/) {}
  virtual void MouseUp() {}
  virtual void DoubleClick() {}
  virtual void Fire() {}
};

std::unique_ptr<GameRunner> GetGameRunner(ProgramEngine const& pe);

#endif // GAME_RUNNER_INTERFACE_H
