#include <iostream>
#include <map>
#include <time.h>
#include <vector>

#include "Global.h"

#include "GuiSdl.h"
#include "SuiSdl.h"

#define TRUE 1
#define FALSE 0

using namespace Gui;

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

  // vSdlMapper[SDLK_NUMLOCK] = GUI_NUMLOCK;
  // vSdlMapper[SDLK_CAPSLOCK] = GUI_CAPSLOCK;
  // vSdlMapper[SDLK_SCROLLOCK] = GUI_SCRLOCK;

  vSdlMapper[SDLK_RSHIFT] = GUI_SHIFT;
  vSdlMapper[SDLK_LSHIFT] = GUI_SHIFT;
  vSdlMapper[SDLK_RCTRL] = GUI_CTRL;
  vSdlMapper[SDLK_LCTRL] = GUI_CTRL;

  vSdlMapper[SDLK_DELETE] = GUI_DELETE;
}

unsigned GetTicks() { return SDL_GetTicks(); }

SP<FileManager> GetFileManager() { return new StdFileManager(); }

int main(int argc, char *argv[])
// int main()
{
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

    /*const SDL_VideoInfo* pInf = SDL_GetVideoInfo();
    if(inf.bFlexibleResolution && inf.bFullScreen)
    {
            sBound = Rectangle(Point(0, 0), Size(pInf->current_w,
    pInf->current_h));

            if(!inf.bBlackBox)
            {
                    inf.szScreenRez.x = pInf->current_w;
                    inf.szScreenRez.y = pInf->current_h;
            }
            else
            {
                    rOffSet.p.x = (pInf->current_w - inf.szScreenRez.x)/2;
                    rOffSet.p.y = (pInf->current_h - inf.szScreenRez.y)/2;

                    if(rOffSet.p.x < 0)
                            rOffSet.p.x = 0;
                    if(rOffSet.p.y < 0)
                            rOffSet.p.y = 0;
            }
    }*/

    SP<SdlGraphicalInterface> pGraph =
        new SdlGraphicalInterface(sBound.sz, inf.bFullScreen, rOffSet);
    SP<GraphicalInterface<Index>> pGr =
        new SimpleGraphicalInterface<SdlImage *>(pGraph);

    SP<SdlSoundInterface> pSound = new SdlSoundInterface();
    SP<SoundInterface<Index>> pSndMng =
        new SimpleSoundInterface<Mix_Chunk *>(pSound);

    // SDL_WM_SetIcon(SDL_LoadBMP("icon\\game_icon.bmp"), NULL);
    // pGraph->SetIcon("icon\\game_icon.bmp");
    pGraph->SetTitle(inf.strTitle.c_str());

    pGr->DrawRectangle(sBound, Color(0, 0, 0), true);

    if (inf.bMouseCapture) {
      SDL_ShowCursor(SDL_DISABLE);
      // SDL_WM_GrabInput(SDL_GRAB_ON);
      SDL_SetWindowGrab(pGraph->pScreenWindow, SDL_TRUE);
    }

    ProgramEngine pe(NewSwitchEvent(bExit, bTrue), pGr, pSndMng, new IoWriter(),
                     inf.szScreenRez);

    // if(inf.bFlexibleResolution && inf.bFullScreen && inf.bBlackBox)
    //	pe.szActualRez = Size(pInf->current_w, pInf->current_h);

    SP<GlobalController> pGl = GetGlobalController(pe);

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
          // std::cout << "Evn: " << event.motion.x << " " << event.motion.y <<
          // "\n";

          int x, y;
          SDL_GetRelativeMouseState(&x, &y);
          // std::cout << "Sdl: " << x << " " << y << "\n";

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
