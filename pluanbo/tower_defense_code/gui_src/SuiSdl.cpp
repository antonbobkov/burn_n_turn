#include "SuiSdl.h"

namespace Gui
{

    SdlSoundInterface::SdlSoundInterface()
    {
        Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 65536 );
    }

    void SdlSoundInterface::DeleteSound(Mix_Chunk* pSnd)
    {
        Mix_FreeChunk(pSnd);
    }

    Mix_Chunk* SdlSoundInterface::LoadSound(std::string sFile)
    {
        Mix_Chunk * pSnd = Mix_LoadWAV(sFile.c_str());

        if(!pSnd)
            throw SimpleException("SdlSoundInterface", "LoadSound", "Cannot load " + sFile);

        return pSnd;
    }

    void SdlSoundInterface::PlaySound(Mix_Chunk* pSnd, int nChannel, bool bLoop)
    {
        Mix_PlayChannel(nChannel, pSnd, bLoop ? -1 : 0);
    }

	void SdlSoundInterface::StopSound(int channel)
	{
		if (channel == -1)
		{
		Mix_HaltMusic(); //doesn't work as expected...
		//Mix_CloseAudio();
		}
		else
		{
		Mix_HaltChannel(channel);
		}
	}
	
	void SdlSoundInterface::SetVolume(float fVolume)
	{
		Mix_Volume(-1, 128*fVolume);
	}
}