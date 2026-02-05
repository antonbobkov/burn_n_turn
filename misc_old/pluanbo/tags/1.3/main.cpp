#include <stdlib.h>

#include <map>
#include <vector>

#include "GuiSdl.h"
#include "tower_defense.h"

#include <time.h>
#include <iostream>

#define TRUE 1
#define FALSE 0


int main(int argc, char *argv[])
{
    srand( (unsigned)time( NULL ));

	SDL_Event event;

    SP_Info* pCleanUp = 0;

    {
    SP< GraphicalInterface<IndexImg> > pGr_copy;

    try
    {
        FilePath fp;
        {
            std::ifstream ifs("config.txt");
            ifs >> fp;
        }
        
        Rectangle rBound = Rectangle(0, 0, 320, 240);
		unsigned nScale = 2;

        SP<Gui::SdlGraphicalInterface> pGraph = new SdlGraphicalInterface(Gui::Size(rBound.sz.x*nScale, rBound.sz.y*nScale));

        SP< GraphicalInterface<IndexImg> > pGr = 
        new Gui::SimpleGraphicalInterface<Gui::SdlImage*>(pGraph);

        SDL_ShowCursor(SDL_DISABLE);
        SDL_WM_GrabInput(SDL_GRAB_ON);
        
        SDL_WM_SetCaption("TOWER DEFENSE", NULL);

        SP<ScalingDrawer> pDr = new ScalingDrawer(pGr, nScale);


        SP<NumberDrawer> pNum = new NumberDrawer(pDr, fp.sPath + "numbers.bmp");

    	SP<SoundManager> pSm = new SoundManager(fp);

        SP<GlobalController> pCnt = new GlobalController(pDr, pNum, pSm, fp);

        
        //ImageSequence vBck(Cnt.pDr->LoadImage("upbar.bmp"));
        //Cnt.pDr->Scale(vBck.vImage[0]);
        //SP<Animation> pBkg2 = new Animation(0, vBck, 1, Point(0,0)); 

        SP<Animation> pStr = new Animation(0, pCnt->pr("start"), 10, Point(rBound.sz.x/2,rBound.sz.y*3/4), true);

        SP<BasicController> pCnt1 = new BasicController(pCnt, rBound, Color(0, 0, 0));
        SP<BasicController> pCnt2 = new AlmostBasicController( BasicController(pCnt, rBound, Color(128, 0, 255)));
        SP<BasicController> pCnt3 = new BasicController(pCnt, rBound, Color(0, 0, 0));
        
        //SP<Countdown> pCnn = new Countdown(pNum, 15);

        SP<Animation> pWin = new Animation(0, pCnt->pr("win"), 3, Point(rBound.sz.x/2,rBound.sz.y/2), true);
        SP<StaticImage> pL = new StaticImage(pCnt->pr["logo"],Point(rBound.sz.x/2,rBound.sz.y/3), true);
        SP<StaticImage> pO = new StaticImage(pCnt->pr["over"],Point(rBound.sz.x/2,rBound.sz.y/3), true);
        
        pCnt1->AddBoth(pStr);
        pCnt1->AddV(pL);
        pCnt3->AddBoth(pWin);
        pCnt2->AddV(pO);
        
        PushBackASSP(pCnt.GetRawPointer(), pCnt->vCnt, pCnt1);
        PushBackASSP(pCnt.GetRawPointer(), pCnt->vCnt, new AdvancedController(pCnt, rBound, Color(0, 0, 0), 1));
        PushBackASSP(pCnt.GetRawPointer(), pCnt->vCnt, new AdvancedController(pCnt, rBound, Color(0, 0, 0), 2));
        PushBackASSP(pCnt.GetRawPointer(), pCnt->vCnt, new AdvancedController(pCnt, rBound, Color(0, 0, 0), 3));
        PushBackASSP(pCnt.GetRawPointer(), pCnt->vCnt, pCnt3);
        PushBackASSP(pCnt.GetRawPointer(), pCnt->vCnt, pCnt2);
        
        pSm->playSound(pSm->loadSound("earthquake01.wav"));

        Uint32 nTimer = SDL_GetTicks();
        
        bool bExit = false;
        while(!bExit) {
    		
		    if(SDL_GetTicks() - nTimer > 100)
            {
                nTimer = SDL_GetTicks();
                pCnt->vCnt[pCnt->nActive]->Update();
            }
            
            if( SDL_PollEvent( &event ) )
            {
			    int i = event.type;
                
                if(event.type == SDL_QUIT)
                    break;
                
                /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
			    switch( event.type ){
                    case SDL_QUIT:
                        bExit = true;
                        break;
                    case SDL_KEYDOWN:
                        pCnt->vCnt[pCnt->nActive]->OnKey(event.key.keysym.sym, false);
                        
                        switch(event.key.keysym.sym) {
                            case SDLK_ESCAPE:
                              bExit = true;
                              break;
                            case SDLK_EQUALS:
                              std::cout << "Global: " << nGlobalSuperMegaCounter << "\n"; 
                              break;
                            default:
                              break;
                        }
                        break;

                    case SDL_KEYUP:
                        pCnt->vCnt[pCnt->nActive]->OnKey(event.key.keysym.sym, true);
                    case SDL_MOUSEBUTTONDOWN:
                        //wp.OnMouse();
                        break;
                    default:
                        break;
			    }
		    }
	    }

        pCleanUp = pCnt.GetRawPointer();
        pGr_copy = pGr;
    }
    catch(Gui::MyException& me)
    {
        std::cout << me.GetDescription(true) << "\n";
    }

    CleanIslandSeeded(pCleanUp);

    }

    if(nGlobalSuperMegaCounter != 0)
        std::cout << "Memory Leak: " << nGlobalSuperMegaCounter << "\n"; 

    return 0;
}
