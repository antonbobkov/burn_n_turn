#include "SuiMfc.h"

#include <windows.h>
#include <mmsystem.h>

#undef PlaySound

namespace Gui
{
    void MfcSoundInterface::PlaySound(std::string pSnd, int nChannel, bool bLoop)
    {
        if(!bLoop)
            PlaySoundA(pSnd.c_str(), NULL, SND_FILENAME | SND_ASYNC);
        else
            PlaySoundA(pSnd.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    }
}

