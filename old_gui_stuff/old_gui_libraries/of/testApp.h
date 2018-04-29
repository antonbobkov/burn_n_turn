#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"

#include "GuiOf.h"

#include "Global.h"

using namespace Gui;


class testApp : public ofBaseApp{

    SP< OfGraphicalInterface > pGraph;
    SP< GraphicalInterface<Index> > pGr;
    SP< SoundInterface<Index> > pSn;
    SP< MessageWriter > pMsg;

    SP<GlobalController> pGl;

    int nTime;

    ProgramInfo inf;

    std::vector<GuiKeyType> vOfMapper;

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

};

#endif
