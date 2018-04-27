#ifndef GLOBAL_GUI_INCLUDE_GUARD_09_04_10_04_01_PM
#define GLOBAL_GUI_INCLUDE_GUARD_09_04_10_04_01_PM 

#include "event.h"

#include "GuiGen.h"
#include "SuiGen.h"
#include "MessageWriter.h"

namespace Gui
{
    struct ProgramInfo
    {
        Size szScreenRez;
        std::string strTitle;
        unsigned nFramerate;
        bool bMouseCapture;
		bool bFullScreen;
		bool bFlexibleResolution;
		bool bBlackBox;

        ProgramInfo():bMouseCapture(false), bFullScreen(false), bFlexibleResolution(false){}
        ProgramInfo(Size szScreenRez_, std::string strTitle_, unsigned nFramerate_,
			bool bMouseCapture_ = false, bool bFullScreen_ = false, bool bFlexibleResolution_ = false, bool bBlackBox_ = false)
            :szScreenRez(szScreenRez_), strTitle(strTitle_), nFramerate(nFramerate_), bBlackBox(bBlackBox_),
			bMouseCapture(bMouseCapture_), bFullScreen(bFullScreen_), bFlexibleResolution(bFlexibleResolution_){}
    };

}

Gui::ProgramInfo GetProgramInfo();

namespace Gui
{

    struct ProgramEngine
    {
        SP<Event> pExitProgram;

        SP< GraphicalInterface<Index> > pGr;
        SP< SoundInterface<Index> > pSndMng;
        SP< MessageWriter > pMsg;

		Size szScreenRez;
		Size szActualRez;

        ProgramEngine(SP<Event> pExitProgram_, SP< GraphicalInterface<Index> > pGr_, SP< SoundInterface<Index> > pSndMng_,
                    SP< MessageWriter > pMsg_)
                    :pExitProgram(pExitProgram_), pGr(pGr_), pSndMng(pSndMng_), pMsg(pMsg_)
		{
			szScreenRez = GetProgramInfo().szScreenRez;
			szActualRez = szScreenRez;
		}

        ProgramEngine(SP<Event> pExitProgram_, SP< GraphicalInterface<Index> > pGr_, SP< SoundInterface<Index> > pSndMng_,
                SP< MessageWriter > pMsg_, Size szScreenRez_)
                :pExitProgram(pExitProgram_), pGr(pGr_), pSndMng(pSndMng_), pMsg(pMsg_), szScreenRez(szScreenRez_)
		{
			szActualRez = szScreenRez;
		}
};


    class GlobalController: virtual public SP_Info
    {
    public:
        virtual void Update()=0;
        
        virtual void KeyDown(GuiKeyType nCode){}
        virtual void KeyUp(GuiKeyType nCode){}
        virtual void MouseMove(Point p){}
        virtual void MouseClick(Point p){}
		virtual void MouseDown(Point p){}
		virtual void MouseUp(){}
		virtual void DoubleClick(){}
		virtual void Fire(){}
    };

}

SP<Gui::GlobalController> GetGlobalController(Gui::ProgramEngine pe);

unsigned GetTicks();

#endif // GLOBAL_GUI_INCLUDE_GUARD_09_04_10_04_01_PM