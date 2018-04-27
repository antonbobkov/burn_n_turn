#ifndef NOISYFLY
#define NOISYFLY

#include "SDL.h"
#include "SDL_mixer.h"

#include "SmartPointer.h"
#include "Preloader.h"

#include <string>
#include <map>
#include <iostream>

struct SoundManager: public SP_Info
{
	std::map<std::string, Mix_Chunk *> mChunks;
	std::map<int, std::string> mIds;
	int counter;

    FilePath fp;

    SoundManager(FilePath fp_ = FilePath()):fp(fp_)
	{
        counter = 0;
        Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 65536 );
		//Mix_Volume(0,999999);
	}
	~SoundManager()
	{
		for(std::map<std::string, Mix_Chunk *>::iterator it = mChunks.begin(); it != mChunks.end(); it++)
			Mix_FreeChunk(it->second);
	}

	int loadSound(std::string filename)
	{
		fp.Parse(filename);

        Mix_Chunk * m = Mix_LoadWAV(filename.c_str());
		if(!m)
			return -1;
		counter++;
		mChunks[filename] = m;
		mIds[counter] = filename;
		return counter;
	}

	void playSound(int id)
	{
        if(mIds.find(id) != mIds.end())
		{
			Mix_PlayChannel( -1, getChunk(id), 0 );
			//std::cout << "playing sound " << Mix_PlayChannel( -1, getChunk(id), 0 ) << std::endl;
			//std::cout << Mix_Playing(0);
		}
	}

    void playSound(std::string str)
	{
		fp.Parse(str);

        playSound(loadSound(str));
	}
    
    Mix_Chunk * getChunk(int id)
	{
		return mChunks[mIds[id]];
	}
};
#endif
