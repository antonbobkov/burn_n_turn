#include <string>
#include "SmartPointer.h"

class ServerHandler;
class ConnectionHandler;

struct Address
{
	std::string sIp;
	int nPort;

	Address(std::string sIp_, int nPort_)
		:sIp(sIp_), nPort(nPort_){}
};

struct Message: public SP_Info
{
	std::string sMessage;

	Message(std::string sMessage_):sMessage(sMessage_){}
	Message(char* c_str, int nLen):sMessage(c_str, nLen){}
};

class Server: public SP_Info
{
	Server(const Server&);
	void operator = (Server&);

protected:
	bool bActive;
	const Address adr;

	const SP<ServerHandler> pHndl;

public:
	Server(Address adr_, SP<ServerHandler> pHndl_)
		:adr(adr_), pHndl(pHndl_){}

	virtual void Disconnect()=0;

	bool IsActive() const {return bActive;}
	Address GetAddress() const {return adr;}
};

class Connection: public SP_Info
{
	Connection(const Connection&);
	void operator = (Connection&);

protected:
	bool bActive;
	
	const Address adrOur;
	const Address adrTheir;

	const SP<ConnectionHandler> pHndl;

public:
	Connection(Address adrOur_, Address adrTheir_, SP<ConnectionHandler> pHndl_)
		:adrOur(adrOur_), adrTheir(adrTheir_), pHndl(pHndl_), bActive(true){}

	virtual void SendMessage(SP<Message>)=0;	// in charge of deleting it
	virtual void Disconnect()=0;

	bool IsActive() const {return bActive;}
	Address GetOurAddress() const {return adrOur;}
	Address GetTheirAddress() const {return adrTheir;}
};

enum NuiDisconnectType {NDT_OUR_CLOSE, NDT_THEIR_CLOSE, NDT_FAIL, NDT_SEND_FAIL};

class ConnectionHandler: public SP_Info
{
public:
	virtual void OnNewMessage(SP<Connection>, SP<Message>)=0;				// asyncronized
																	// in charge of message deletion
	virtual void OnDisconnect(SP<Connection>, NuiDisconnectType)=0;	// asyncronized
																	// in charge of connection deletion
																	// ndt cases: all possible 
};

class ServerHandler: public SP_Info
{
public:
	const SP<ConnectionHandler> pCntHndl;

	ServerHandler(SP<ConnectionHandler> pCntHndl_):pCntHndl(pCntHndl_){}

	virtual void OnNewConnection(SP<Connection>)=0;				// asyncronized
	virtual void OnDisconnect(SP<Server>, NuiDisconnectType)=0;	// asyncronized
																// in charge of server deletion
																// ndt cases: NDT_OUR_CLOSE, NDT_FAIL
};

SP<Server> NewServer(Address adr, SP<ServerHandler> pHndl);
SP<Connection> NewConnection(Address adr, SP<ConnectionHandler> pHndl);