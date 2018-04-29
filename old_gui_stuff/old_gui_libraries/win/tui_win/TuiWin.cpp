#include "TuiWin.h"

#include <windows.h>

DWORD WINAPI WinThreadShell(LPVOID args)
{
	ThreadFunctor* pF = reinterpret_cast<ThreadFunctor*>(args);

	pF->Apply();

	delete pF;

	return 0;
}

void SystemNewThread(ThreadFunctor* pF)
{
	void* pData = pF;

	DWORD threadId;
	CreateThread( NULL, 0, WinThreadShell, pData, 0, &threadId);
}

class MutexRaw
{
	MutexRaw(const MutexRaw&);
	void operator =(const MutexRaw&);

public:

	HANDLE ghMutex;

	MutexRaw()
	{
		ghMutex = CreateMutex(NULL, FALSE, NULL);
	}

	~MutexRaw()
	{
		CloseHandle(ghMutex);
	}
};

Mutex NewMutex()
{
	return new MutexRaw();
}

void LockMutex(Mutex m)
{
	WaitForSingleObject(m->ghMutex, INFINITE);
}

void ReleaseMutex(Mutex m)
{
	ReleaseMutex(m->ghMutex);
}

void DeleteMutex(Mutex m)
{
	delete m;
}
