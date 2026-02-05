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
        Gui::SdlGraphicalInterface Graph(Gui::Size(600, 300));
        Gui::SimpleGraphicalInterface<Gui::SdlImage*> IntGraph(&Graph);

        SDL_WM_SetCaption("TOWER DEFENSE", NULL);

        ScalingDrawer Dr(&IntGraph, 3);

        NumberDrawer Num(&Dr);
        
        GlobalController Cnt(&Dr, &Num);

        Rectangle rBound = Rectangle(0, 0, 200, 100);
        
        ImageSequence vBck(Cnt.pDr->LoadImage("upbar.bmp"));
        Cnt.pDr->Scale(vBck.vImage[0]);
        Animation* pBkg2 = new Animation(0, vBck, 1, Point(0,0)); 

        ImageSequence vStr(Cnt.pDr->LoadImage("start.bmp"), Cnt.pDr->LoadImage("start2.bmp"));
        Cnt.pDr->Scale(vStr.vImage[0]);
        Cnt.pDr->Scale(vStr.vImage[1]);       
        
        Animation* pStr = new Animation(0, vStr, 10, Point(0,0));

        BasicController* pCnt = new BasicController(&Cnt, rBound, Color(255, 255, 0));
        BasicController* pCnt2 = new BasicController(&Cnt, rBound, Color(115, 115, 115));
        
        Countdown* pCnn = new Countdown(&Num, 15);
        
        pCnt->AddBoth(pStr);
        pCnt2->AddBoth(pBkg2);
        pCnt2->AddBoth(pCnn);
        
        Cnt.vCnt.push_back(pCnt);
        Cnt.vCnt.push_back(pCnt2);
        Cnt.vCnt.push_back(new AdvancedController(&Cnt, rBound, Color(0, 0, 0)));


        Uint32 nTimer = SDL_GetTicks();
        
        while(true) {
    		
		    if(SDL_GetTicks() - nTimer > 100)
            {
                nTimer = SDL_GetTicks();
                Cnt.vCnt[Cnt.nActive]->Update();
            }
            
            if( SDL_PollEvent( &event ) )
            {
			    int i = event.type;
                /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
			    switch( event.type ){
                    case SDL_QUIT  :
		                  exit(0);
                    case SDL_KEYDOWN:
                        Cnt.vCnt[Cnt.nActive]->OnKey(char(event.key.keysym.sym), false);
                        
                        switch(event.key.keysym.sym) {
                            case SDLK_ESCAPE:
                              exit(0);
                            default:
                              break;
                        }
                        break;

                    case SDL_KEYUP:
                        Cnt.vCnt[Cnt.nActive]->OnKey(char(event.key.keysym.sym), true);
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