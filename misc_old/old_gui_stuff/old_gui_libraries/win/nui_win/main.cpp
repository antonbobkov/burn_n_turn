#include <string>
#include <list>
#include <iostream>

#include "NuiWin.h"

using namespace std;

Mutex pMtx;
list<string> lsOutput;

void AddMessage(string s)
{
	MutexLocker lckr(pMtx);
	lsOutput.push_back(s);
}

void OutputMessages()
{
	MutexLocker lckr(pMtx);

	while(lsOutput.size())
	{
		cout << lsOutput.front() << "\n";
		lsOutput.pop_front();
	}
}

struct SimpleConnectionHandler: public ConnectionHandler
{
	/*virtual*/ void OnNewMessage(SP<Connection> pCn, SP<Message> pMsg)
	{
		AddMessage(pMsg->sMessage);
		OutputMessages();
	}
	/*virtual*/ void OnDisconnect(SP<Connection> pCn, NuiDisconnectType ndt){}
};

struct SimpleServerHandler: public ServerHandler
{
	SimpleServerHandler(SP<ConnectionHandler> pCntHndl_):ServerHandler(pCntHndl_){}

	/*virtual*/ void OnNewConnection(SP<Connection>){}
	/*virtual*/ void OnDisconnect(SP<Server> pSv, NuiDisconnectType ndt){}
};

int main()
{
	pMtx = NewMutex();

	SP<SimpleConnectionHandler> pCnHndl = new SimpleConnectionHandler();
	SP<SimpleServerHandler> pServHndl = new SimpleServerHandler(pCnHndl);

	string sData;
	
	cin >> sData;

	if(sData == "server")
	{
		NewServer(Address("127.0.0.1", 100), pServHndl);
	}
	else
	{
		SP<Connection> pCn = NewConnection(Address("127.0.0.1", 100), pCnHndl);
		pCn->SendMessage(new Message(sData));
	}
	
	cin.get();
	cin.get();

	DeleteMutex(pMtx);

	return 0;
}