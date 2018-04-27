#include <winsock2.h>

#include "GenTui.h"

#include "GenNui.h"

const int nBufferSize = 1024;

class DefaultException {};
// TODO: thread fail thingy

class NetworkInitializer
{
public:
	NetworkInitializer();
	~NetworkInitializer();
};

class WinServer;
class WinConnection;

void WinServerThread(SP<WinServer> pWs);
void WinConnectionReadThread(SP<WinConnection> pCnt);
void WinConnectionWriteThread(SP<WinConnection> pCnt, SP<Message> pMsg);

class WinServer: public Server
{
	friend void WinServerThread(SP<WinServer> pWs);

	SOCKET sctListen;  // asyncronized access (no choice)

	Mutex pMtx;

public:
	WinServer(Address adr_, SP<ServerHandler> pHndl_);
	~WinServer();

	/*virtual*/ void Disconnect();

private:
	void DisconnectTp(NuiDisconnectType ndt);
};

class WinConnection: public Connection
{
	friend void WinConnectionReadThread(SP<WinConnection> pCnt);
	friend void WinConnectionWriteThread(SP<WinConnection> pCnt, SP<Message> pMsg);

	SOCKET sctMessage; // asyncronized access (no choice)

	Mutex pMtx;
public:
	WinConnection(Address adrOur_, Address adrTheir_, SP<ConnectionHandler> pHndl_, SOCKET sctMessage_);
	~WinConnection();

	/*virtual*/ void SendMessage(SP<Message> pMsg);
	/*virtual*/ void Disconnect();

private:
	void DisconnectTp(NuiDisconnectType ndt);
};