#include <iostream>
#include <map>
#include <time.h>
#include <vector>

#include "wrappers/color.h"
#include "utils/file_utils.h"
#include "game_utils/game_runner_interface.h"
#include "wrappers/gui_key_type.h"
#include "utils/smart_pointer.h"

#include "GuiSdl.h"
#include "SuiSdl.h"

#define TRUE 1
#define FALSE 0


void SdlMapKeys(std::map<int, GuiKeyType> &vSdlMapper) {
  for (unsigned i = 0; i < 400; ++i)
    vSdlMapper[i] = GuiKeyType(i);

  vSdlMapper[SDLK_F1] = GUI_F1;
  vSdlMapper[SDLK_F2] = GUI_F2;
  vSdlMapper[SDLK_F3] = GUI_F3;
  vSdlMapper[SDLK_F4] = GUI_F4;
  vSdlMapper[SDLK_F5] = GUI_F5;
  vSdlMapper[SDLK_F6] = GUI_F6;
  vSdlMapper[SDLK_F7] = GUI_F7;
  vSdlMapper[SDLK_F8] = GUI_F8;
  vSdlMapper[SDLK_F9] = GUI_F9;
  vSdlMapper[SDLK_F10] = GUI_F10;
  vSdlMapper[SDLK_F11] = GUI_F11;
  vSdlMapper[SDLK_F12] = GUI_F12;

  vSdlMapper[SDLK_UP] = GUI_UP;
  vSdlMapper[SDLK_DOWN] = GUI_DOWN;
  vSdlMapper[SDLK_LEFT] = GUI_LEFT;
  vSdlMapper[SDLK_RIGHT] = GUI_RIGHT;

  vSdlMapper[SDLK_INSERT] = GUI_INSERT;
  vSdlMapper[SDLK_HOME] = GUI_HOME;
  vSdlMapper[SDLK_END] = GUI_END;
  vSdlMapper[SDLK_PAGEUP] = GUI_PGUP;
  vSdlMapper[SDLK_PAGEDOWN] = GUI_PGDOWN;

  vSdlMapper[SDLK_RSHIFT] = GUI_SHIFT;
  vSdlMapper[SDLK_LSHIFT] = GUI_SHIFT;
  vSdlMapper[SDLK_RCTRL] = GUI_CTRL;
  vSdlMapper[SDLK_LCTRL] = GUI_CTRL;

  vSdlMapper[SDLK_DELETE] = GUI_DELETE;
}

int main(int argc, char *argv[]) {
  srand((unsigned)time(NULL));

  SDL_Event event;

  bool bTrue = true;
  bool bExit = false;

  try {
    ProgramInfo inf = GetProgramInfo();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(0);

    Point pOffSet(0, 0);

    Rectangle sBound = Rectangle(Point(0, 0), inf.szScreenRez);

    Rectangle rOffSet = sBound;

    smart_pointer<SdlGraphicalInterface> pGraph = make_smart(
        new SdlGraphicalInterface(sBound.sz, inf.bFullScreen, rOffSet));
    smart_pointer<GraphicalInterface<Index>> pGr =
        make_smart(new SimpleGraphicalInterface<SdlImage *>(pGraph));

    smart_pointer<SdlSoundInterface> pSound =
        make_smart(new SdlSoundInterface());
    smart_pointer<SoundInterface<Index>> pSndMng =
        make_smart(new SimpleSoundInterface<Mix_Chunk *>(pSound));

    // SDL_WM_SetIcon(SDL_LoadBMP("icon\\game_icon.bmp"), NULL);
    // pGraph->SetIcon("icon\\game_icon.bmp");
    pGraph->SetTitle(inf.strTitle.c_str());

    pGr->DrawRectangle(sBound, Color(0, 0, 0), true);

    if (inf.bMouseCapture) {
      SDL_ShowCursor(SDL_DISABLE);
      SDL_SetWindowGrab(pGraph->pScreenWindow, SDL_TRUE);
    }

    std::unique_ptr<FileManager> fm(new StdFileManager());
    ProgramEngine pe(make_smart(NewSwitchEvent(bExit, bTrue)), pGr, pSndMng,
                     make_smart(new IoWriter()), inf.szScreenRez, fm.get());

    // if(inf.bFlexibleResolution && inf.bFullScreen && inf.bBlackBox)
    //	pe.szActualRez = Size(pInf->current_w, pInf->current_h);

    smart_pointer<GameRunner> pGl = GetGameRunner(pe);

    std::map<int, GuiKeyType> vSdlMapper;
    SdlMapKeys(vSdlMapper);

    Uint32 nTimer = SDL_GetTicks();

    Point pMousePos = Point(0, 0);
    while (!bExit) {
      if (SDL_GetTicks() - nTimer > inf.nFramerate) {
        nTimer = SDL_GetTicks();
        pGl->Update();
      }

      if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
          break;
        else if (event.type == SDL_KEYUP)
          pGl->KeyUp(vSdlMapper[event.key.keysym.sym]);
        else if (event.type == SDL_KEYDOWN) {
          pGl->KeyDown(vSdlMapper[event.key.keysym.sym]);

          if (event.key.keysym.sym == SDLK_EQUALS)
            std::cout << "Global: " << nGlobalSuperMegaCounter << "\n";
        } else if (event.type == SDL_MOUSEMOTION) {
          int x, y;
          SDL_GetRelativeMouseState(&x, &y);

          pMousePos += Point(x, y);
          if (inf.bMouseCapture)
            pGl->MouseMove(pMousePos);
          else
            pGl->MouseMove(Point(event.motion.x, event.motion.y));

        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
          pGl->MouseClick(Point(event.button.x, event.button.y));
          pGl->MouseDown(Point(event.button.x, event.button.y));
        } else if (event.type == SDL_MOUSEBUTTONUP) {
          pGl->MouseUp();
        }
      }
    }
  } catch (MyException &me) {
    std::cout << me.GetDescription(true) << "\n";

    std::ofstream ofs("error.txt");

    ofs << me.GetDescription(true) << "\n";

    ofs.close();
  }
  /*
  catch(...)
  {
  std::cout << "Unknown error!\n";
  }
  */

  if (nGlobalSuperMegaCounter != 0)
    std::cout << "Memory Leak: " << nGlobalSuperMegaCounter << "\n";

  return 0;
}
