
#pragma once

#include "SockThdObj.h"
#include <vector>

class CRealtimeManager;
class CRealtimeClient;

class CClientSocket : public CSocketObj
{
public:
	CClientSocket(CRealtimeClient* client);
	~CClientSocket();

	void Close();
	bool OnParsing();

private:
	CRealtimeClient* m_pClient;
};

class CRealtimeClient : public CSocketThdObj
{
public:
	CRealtimeClient(CRealtimeManager* mgr, tcp::endpoint& accept_endpoint);
	~CRealtimeClient();

	bool FromData(const char* sFrame, int nFrame);
	bool ToData(const char* sFrame, int nFrame);

	void OnClearProc();
	void OnRunProc();
	void OnTimerProc();

private:
	void AsyncAccept();
	bool AddClient(CClientSocket* client);

private:
	CRealtimeManager* m_pRTMgr;
	tcp::acceptor m_acceptor;
	std::vector<CClientSocket*> m_arClient;
	std::vector<CClientSocket*> m_arRemoved;
};
