#include "TuiSdl.h"

#include "SDL.h"

int SdlThreadShell(void* pData)
{
	ThreadFunctor* pF = reinterpret_cast<ThreadFunctor*>(pData);

	pF->Apply();

	delete pF;

	return 0;
}

void SystemNewThread(ThreadFunctor* pF)
{
	void* pData = pF;

	SDL_CreateThread(SdlThreadShell, pData);
}

class MutexRaw
{
	MutexRaw(const MutexRaw&);
	void operator =(const MutexRaw&);

public:

	SDL_mutex* pSdlMtx;

	MutexRaw()
	{
		pSdlMtx = SDL_CreateMutex();
	}

	~MutexRaw()
	{
		SDL_DestroyMutex(pSdlMtx);
	}
};

Mutex NewMutex()
{
	return new MutexRaw();
}

void LockMutex(Mutex m)
{
	SDL_mutexP(m->pSdlMtx);
}

void ReleaseMutex(Mutex m)
{
	SDL_mutexV(m->pSdlMtx);
}

void DeleteMutex(Mutex m)
{
	delete m;
}
