#include "NuiWin.h"

WinServer::WinServer(Address adr_, SP<ServerHandler> pHndl_)
	:Server(adr_, pHndl_)
{
	pMtx = NewMutex();
	
	sockaddr_in local;

	local.sin_family = AF_INET;
	local.sin_addr.s_addr = inet_addr(adr.sIp.c_str());
	local.sin_port = htons(adr.nPort);

	sctListen = socket(AF_INET, SOCK_STREAM, 0);

	if(sctListen == INVALID_SOCKET)
	{
		int nRet = WSAGetLastError();
		throw DefaultException();
	}
	
	int nRet = bind(sctListen, (struct sockaddr*)&local, sizeof(local));

	if(nRet == SOCKET_ERROR)
	{
		int nRet = WSAGetLastError();
		throw DefaultException();
	}

	nRet = listen(sctListen, 5); // size of connection stack (1-5)

	if(nRet == SOCKET_ERROR)
	{
		int nRet = WSAGetLastError();
		throw DefaultException();
	}

	NewThread(WinServerThread, SP<WinServer>(this)); // note: unsafe smart pointer usage
}

WinServer::~WinServer()
{
	Disconnect();
	
	DeleteMutex(pMtx);
}


/*virtual*/ void WinServer::Disconnect()
{
	DisconnectTp(NDT_OUR_CLOSE);
}


void WinServer::DisconnectTp(NuiDisconnectType ndt)
{
	MutexLocker lck(pMtx);

	if(!bActive)
		return;

	bActive = false;
	closesocket(sctListen);

	pHndl->OnDisconnect(this, ndt);
}

WinConnection::WinConnection(Address adrOur_, Address adrTheir_, SP<ConnectionHandler> pHndl_, SOCKET sctMessage_)
	:Connection(adrOur_, adrTheir_, pHndl_), sctMessage(sctMessage_)
{
	pMtx = NewMutex();

	NewThread(WinConnectionReadThread, SP<WinConnection>(this));
}

WinConnection::~WinConnection()
{
	Disconnect();

	DeleteMutex(pMtx);
}

/*virtual*/ void WinConnection::SendMessage(SP<Message> pMsg)
{
	NewThread(WinConnectionWriteThread, SP<WinConnection>(this), pMsg);
}

/*virtual*/ void WinConnection::Disconnect()
{
	DisconnectTp(NDT_OUR_CLOSE);
}

void WinConnection::DisconnectTp(NuiDisconnectType ndt)
{
	MutexLocker lck(pMtx);

	if(!bActive)
		return;

	bActive = false;
	closesocket(sctMessage);

	pHndl->OnDisconnect(this, ndt);
}


void WinServerThread(SP<WinServer> pWs)
{
	sockaddr_in from;
	int fromlen;
	SOCKET msgsock;

	while(true)
	{
        fromlen = sizeof(from);

		msgsock = accept(pWs->sctListen, (struct sockaddr*)&from, &fromlen);

        if (msgsock == INVALID_SOCKET)
        {
            int nErr = WSAGetLastError();
			pWs->DisconnectTp(NDT_FAIL);
			return;
        }

		Address adrTheir(inet_ntoa(from.sin_addr), htons(from.sin_port));

		SP<WinConnection> pCnt = new WinConnection(pWs->adr, adrTheir, pWs->pHndl->pCntHndl, msgsock);
		pWs->pHndl->OnNewConnection(pCnt);
	}
	
}

void WinConnectionReadThread(SP<WinConnection> pCnt)
{
	char Buffer[nBufferSize];

	while(true)
	{
		int nRet = recv(pCnt->sctMessage, Buffer, sizeof(Buffer), 0);

		if(nRet == SOCKET_ERROR)
		{
			pCnt->DisconnectTp(NDT_FAIL);
			return;
		}

		if(nRet == 0)
		{
			pCnt->DisconnectTp(NDT_THEIR_CLOSE);
			return;
		}

		SP<Message> pMsg = new Message(Buffer, nRet);

		pCnt->pHndl->OnNewMessage(pCnt, pMsg);
	}
}

void WinConnectionWriteThread(SP<WinConnection> pCnt, SP<Message> pMsg)
{
	int nRet = send(pCnt->sctMessage, pMsg->sMessage.c_str(), pMsg->sMessage.size() + 1, 0);

	if(nRet == SOCKET_ERROR)
	{
		int nErr = WSAGetLastError();
		pCnt->DisconnectTp(NDT_SEND_FAIL);
	}
}

NetworkInitializer::NetworkInitializer()
{
	WSADATA wsaData;
	int nRet = WSAStartup(0x202, &wsaData);

	if(nRet != 0)
		throw DefaultException();
}

NetworkInitializer::~NetworkInitializer()
{
	WSACleanup();
}

void InitializeNetworkIfNotDoneSoAlready()
{
	static NetworkInitializer nt;
}

SP<Server> NewServer(Address adr, SP<ServerHandler> pHndl)
{
	InitializeNetworkIfNotDoneSoAlready();
	return new WinServer(adr, pHndl);
}

SP<Connection> NewConnection(Address adr, SP<ConnectionHandler> pHndl)
{
	InitializeNetworkIfNotDoneSoAlready();

	unsigned int addr;
	struct sockaddr_in server;
	struct hostent *hp;
    SOCKET  conn_socket;

	addr = inet_addr(adr.sIp.c_str());
    hp = gethostbyaddr((char *)&addr, 4, AF_INET);

    memset(&server, 0, sizeof(server));
    memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
	server.sin_port = htons(adr.nPort);
 
    conn_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (conn_socket < 0)
    {
        int nErr = WSAGetLastError();
		throw DefaultException();
    }
 
    int nRet = connect(conn_socket, (struct sockaddr*)&server, sizeof(server));

	if(nRet == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
		throw DefaultException();
    }

	return new WinConnection(Address("", 0), adr, pHndl, conn_socket);
}