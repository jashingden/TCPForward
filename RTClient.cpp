// RTClient.cpp
//
#include "RTClient.h"
#include "RTManager.h"
#include "DataSocket.h"
#include <iostream>
#include <boost/format.hpp>
#include <boost/endian/conversion.hpp>

#define SOCK_RECVCHK		60		// 60 sec

using namespace std;
using boost::format;
using namespace boost::endian;

//////////////////////////////////////////////////////////////////////////////////////
//

CClientSocket::CClientSocket(CRealtimeClient* client)
: CSocketObj(m_ios)
{
	m_pClient = client;
}

CClientSocket::~CClientSocket()
{
}

void CClientSocket::Link()
{
	m_pSocket = new CDataSocket(this);
	m_pSocket->SetHost("127.0.0.1", m_pClient->GetDataPort());
	m_pSocket->Connect();
	m_pSocket->Start();
}

void CClientSocket::Unlink()
{
	if (m_pSocket != NULL) {
		m_pSocket->Finish();
		delete m_pSocket;
		m_pSocket = NULL;
	}
}

void CClientSocket::Close()
{
	cout << str(format("Client Socket[%x] %s is closed") % this % m_sRemoteIP) << endl;
	__super::Close();
}

bool CClientSocket::OnParsing()
{
	if (m_nBufSize <= 0) {
		return true;
	}

	if (m_pSocket != NULL) {
		bool dump = false;
		if (m_nBufSize > DUMP_SIZE)
			cout << "f: " << (dump ? string(m_readBuf, m_nBufSize) : to_string(m_nBufSize)) << endl;
		m_pSocket->SendRequest(m_readBuf, m_nBufSize);
	}

	m_nBufSize = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////
//

CRealtimeClient::CRealtimeClient(CRealtimeManager* mgr, tcp::endpoint& accept_endpoint, int to_port)
: CSocketThdObj(SOCK_RECVCHK), m_acceptor(m_ios, accept_endpoint)
{
	m_pRTMgr = mgr;
	m_nDataPort = to_port;
}

CRealtimeClient::~CRealtimeClient()
{
}
/*
bool CRealtimeClient::FromData(const char* sFrame, int nFrame)
{
	if (m_pRTMgr != NULL) {
		return m_pRTMgr->FromData(sFrame, nFrame);
	}
	return false;
}

bool CRealtimeClient::ToData(const char* sFrame, int nFrame)
{
	boost::lock_guard<CRealtimeClient> guard(*this);

	for (CClientSocket* client : m_arClient) {
		client->SendRequest(sFrame, nFrame);
	}
	return true;
}
*/
void CRealtimeClient::AsyncAccept()
{
	CClientSocket* client = new CClientSocket(this);
	m_acceptor.async_accept(client->GetSocket(), [this, client](boost::system::error_code ec) {
		if (!ec) {
			AddClient(client);
		} else {
			delete client;
		}
		AsyncAccept();
	});
}

bool CRealtimeClient::AddClient(CClientSocket* client)
{
	if (client == NULL) {
		return false;
	}

	boost::lock_guard<CRealtimeClient> guard(*this);
	try
	{
		if (client->Create()) {
			client->SetRemoteIP();
			client->Link();
			//client->Start();
			m_arClient.push_back(client);
			cout << str(format("Add Client Socket[%x] %s") % client % client->GetRemoteIP()) << endl;
			return true;
		}
	}
	catch (...)
	{
	}
	return false;
}

void CRealtimeClient::OnClearProc()
{
	for (int i = 0; i < m_arClient.size(); i++)
	{
		CClientSocket* client = (CClientSocket*)m_arClient.at(i);
		if (client != NULL)
		{
			client->Unlink();
			client->Finish();
			client->Close();
			delete client;
		}
	}
	m_arClient.clear();

	for (int i = 0; i < m_arRemoved.size(); i++)
	{
		delete (CClientSocket*)m_arRemoved.at(i);
	}
	m_arRemoved.clear();
}

void CRealtimeClient::OnRunProc()
{
	AsyncAccept();
}

void CRealtimeClient::OnTimerProc()
{
}

