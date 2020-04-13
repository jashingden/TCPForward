
#pragma once

#include "SockThdObj.h"
#include <vector>

class CRealtimeManager;
class CRealtimeClient;
class CDataSocket;

class CClientSocket : public CSocketObj
{
public:
	CClientSocket(CRealtimeClient* client);
	~CClientSocket();

	void Link();
	void Unlink();
	void Close();
	bool OnParsing();

private:
	CRealtimeClient* m_pClient;
	CDataSocket* m_pSocket;
};

class CRealtimeClient : public CSocketThdObj
{
public:
	CRealtimeClient(CRealtimeManager* mgr, tcp::endpoint& accept_endpoint, int to_port);
	~CRealtimeClient();

	//bool FromData(const char* sFrame, int nFrame);
	//bool ToData(const char* sFrame, int nFrame);

	int GetDataPort() {
		return m_nDataPort;
	}

	void OnClearProc();
	void OnRunProc();
	void OnTimerProc();

private:
	void AsyncAccept();
	bool AddClient(CClientSocket* client);

private:
	CRealtimeManager* m_pRTMgr;
	int m_nDataPort = 5555;
	tcp::acceptor m_acceptor;
	std::vector<CClientSocket*> m_arClient;
	std::vector<CClientSocket*> m_arRemoved;
};
