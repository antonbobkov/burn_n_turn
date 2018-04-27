#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){

    Gui::Size sz = GetProgramInfo().szScreenRez;
    
    ofAppGlutWindow window;
	ofSetupOpenGL(&window, sz.x, sz.y, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	
    try
    {
        ofRunApp( new testApp());
    }
    catch(MyException& me)
    {
        std::cout << me.GetDescription(true) << "\n";
    }
    catch(...)
    {
        std::cout << "Unknown error!\n";
    }

}
