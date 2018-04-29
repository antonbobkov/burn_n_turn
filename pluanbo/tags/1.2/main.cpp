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

	try
    {
        SP<Gui::SdlGraphicalInterface> pGraph = new SdlGraphicalInterface(Gui::Size(600, 400));
        
        SP< GraphicalInterface<IndexImg> > pGr = 
        new Gui::SimpleGraphicalInterface<Gui::SdlImage*>(pGraph);

        SDL_ShowCursor(SDL_DISABLE);
        SDL_WM_GrabInput(SDL_GRAB_ON);
        
        SDL_WM_SetCaption("TOWER DEFENSE", NULL);

        SP<ScalingDrawer> pDr = new ScalingDrawer(pGr, 2);

        SP<NumberDrawer> pNum = new NumberDrawer(pDr);
        
        SP<GlobalController> pCnt = new GlobalController(pDr, pNum);

        Rectangle rBound = Rectangle(0, 0, 300, 200);
        
        //ImageSequence vBck(Cnt.pDr->LoadImage("upbar.bmp"));
        //Cnt.pDr->Scale(vBck.vImage[0]);
        //SP<Animation> pBkg2 = new Animation(0, vBck, 1, Point(0,0)); 

        SP<Animation> pStr = new Animation(0, pCnt->pr("start"), 10, Point(rBound.sz.x/2,rBound.sz.y*3/4), true);

        SP<BasicController> pCnt1 = new BasicController(pCnt, rBound, Color(255, 255, 0));
        SP<BasicController> pCnt2 = new AlmostBasicController( BasicController(pCnt, rBound, Color(128, 0, 255)));
        
        //SP<Countdown> pCnn = new Countdown(pNum, 15);

        SP<Animation> pWin = new Animation(0, pCnt->pr("win"), 3, Point(rBound.sz.x/2,rBound.sz.y/2), true);
        
        pCnt1->AddBoth(pStr);
        pCnt2->AddBoth(pWin);
        
        pCnt->vCnt.push_back(pCnt1);
        pCnt->vCnt.push_back(new AdvancedController(pCnt, rBound, Color(0, 0, 0), 1));
        pCnt->vCnt.push_back(new AdvancedController(pCnt, rBound, Color(0, 0, 0), 2));
        pCnt->vCnt.push_back(new AdvancedController(pCnt, rBound, Color(0, 0, 0), 3));
        pCnt->vCnt.push_back(pCnt2);


        Uint32 nTimer = SDL_GetTicks();
        
        while(true) {
    		
		    if(SDL_GetTicks() - nTimer > 100)
            {
                nTimer = SDL_GetTicks();
                pCnt->vCnt[pCnt->nActive]->Update();
            }
            
            if( SDL_PollEvent( &event ) )
            {
			    int i = event.type;
                /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
			    switch( event.type ){
                    case SDL_QUIT  :
		                  exit(0);
                    case SDL_KEYDOWN:
                        pCnt->vCnt[pCnt->nActive]->OnKey(event.key.keysym.sym, false);
                        
                        switch(event.key.keysym.sym) {
                            case SDLK_ESCAPE:
                              exit(0);
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
    }
    catch(Gui::MyException& me)
    {
        std::cout << me.GetDescription(true) << "\n";
    }

	return 0;
}