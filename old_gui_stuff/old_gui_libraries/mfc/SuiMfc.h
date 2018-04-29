#ifndef SUI_MFC_INCLUDE_GUARD_09_05_10_03_01_PM
#define SUI_MFC_INCLUDE_GUARD_09_05_10_03_01_PM

#include "SuiGen.h"

#include <string>

namespace Gui
{
    class MfcSoundInterface: public SoundInterface<std::string>
    {
    public:
        /*virtual*/ void       DeleteSound(std::string sSnd){}
        /*virtual*/ std::string LoadSound(std::string sFile){return sFile;}

        /*virtual*/ void PlaySound(std::string pSnd, int nChannel = -1, bool bLoop = false);
    };
}

#endif // SUI_MFC_INCLUDE_GUARD_09_05_10_03_01_PM