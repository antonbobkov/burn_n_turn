#include "testApp.h"

#include <iostream>

void OfMapKeys(std::vector<GuiKeyType>& vOfMapper)
{
    vOfMapper.resize(400);
    for(unsigned i = 0; i < vOfMapper.size(); ++i)
        vOfMapper[i] = GuiKeyType(i);
    
    vOfMapper[OF_KEY_F1] = GUI_F1;
    vOfMapper[OF_KEY_F2] = GUI_F2;
    vOfMapper[OF_KEY_F3] = GUI_F3;
    vOfMapper[OF_KEY_F4] = GUI_F4;
    vOfMapper[OF_KEY_F5] = GUI_F5;
    vOfMapper[OF_KEY_F6] = GUI_F6;
    vOfMapper[OF_KEY_F7] = GUI_F7;
    vOfMapper[OF_KEY_F8] = GUI_F8;
    vOfMapper[OF_KEY_F9] = GUI_F9;
    vOfMapper[OF_KEY_F10] = GUI_F10;
    vOfMapper[OF_KEY_F11] = GUI_F11;
    vOfMapper[OF_KEY_F12] = GUI_F12;

    vOfMapper[OF_KEY_UP] = GUI_UP;
    vOfMapper[OF_KEY_DOWN] = GUI_DOWN;
    vOfMapper[OF_KEY_LEFT] = GUI_LEFT;
    vOfMapper[OF_KEY_RIGHT] = GUI_RIGHT;

    vOfMapper[OF_KEY_INSERT] = GUI_INSERT;
    vOfMapper[OF_KEY_HOME] = GUI_HOME;
    vOfMapper[OF_KEY_END] = GUI_END;
    vOfMapper[OF_KEY_PAGE_UP] = GUI_PGUP;
    vOfMapper[OF_KEY_PAGE_DOWN] = GUI_PGDOWN;

    //vOfMapper[SDLK_NUMLOCK] = GUI_NUMLOCK;
    //vOfMapper[SDLK_CAPSLOCK] = GUI_CAPSLOCK;
    //vOfMapper[SDLK_SCROLLOCK] = GUI_SCRLOCK;

    //vOfMapper[SDLK_RSHIFT] = GUI_SHIFT;
    //vOfMapper[SDLK_LSHIFT] = GUI_SHIFT;
    //vOfMapper[SDLK_RCTRL] = GUI_CTRL;
    //vOfMapper[SDLK_LCTRL] = GUI_CTRL;

    //vOfMapper[SDLK_DELETE] = GUI_DELETE;
}


//--------------------------------------------------------------
void testApp::setup()
{
    OfMapKeys(vOfMapper);

    ofDisableDataPath();

    inf = GetProgramInfo();

    ofSetWindowTitle(inf.strTitle);
    if(inf.bMouseCapture)
        ofHideCursor();

    pGraph = new OfGraphicalInterface(inf.szScreenRez);
    pGr = new SimpleGraphicalInterface<OpnFrmImage*>(pGraph);

    pSn = new DummySoundInterface<Index>();
    pMsg = new EmptyWriter();

    ProgramEngine pe(0, pGr, pSn, pMsg);

    pGr->DrawRectangle(inf.szScreenRez, Color(0,0,0));

    pGl = GetGlobalController(pe);

    nTime = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void testApp::update()
{
    if(ofGetElapsedTimeMillis() - nTime > inf.nFramerate)
    {
        nTime = ofGetElapsedTimeMillis();
        pGl->Update();
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    pGraph->Draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
    pGl->KeyDown(vOfMapper[key]);
}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{
    pGl->KeyUp(vOfMapper[key]);
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
    pGl->MouseMove(Gui::Point(x, y));
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

