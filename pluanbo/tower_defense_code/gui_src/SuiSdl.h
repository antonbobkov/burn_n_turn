#ifndef SUI_SDL_INCLUDE_GUARD_09_05_10_03_00_PM
#define SUI_SDL_INCLUDE_GUARD_09_05_10_03_00_PM

#include "SuiGen.h"

#include "SDL_mixer.h"

namespace Gui
{
    class SdlSoundInterface: public SoundInterface<Mix_Chunk*>
    {
    public:
        SdlSoundInterface();

        /*virtual*/ void       DeleteSound(Mix_Chunk* pSnd);
        /*virtual*/ Mix_Chunk* LoadSound(std::string sFile);

        /*virtual*/ void PlaySound(Mix_Chunk* pSnd, int nChannel = -1, bool bLoop = false);
		/*virtual*/ void StopSound(int Channel = -1);
		/*virtual*/ void SetVolume(float fVolume);
    };
}

#endif // SUI_SDL_INCLUDE_GUARD_09_05_10_03_00_PM