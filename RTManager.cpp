// RTManager.cpp
//
#include "RTManager.h"
#include "RTClient.h"
#include "DataSocket.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////
//

CRealtimeManager::CRealtimeManager()
{
	m_pRTClient = NULL;
}

CRealtimeManager::~CRealtimeManager()
{
}

bool CRealtimeManager::Init(int from_port, int to_port)
{
	m_pSocket = new CDataSocket(this);
	m_pSocket->SetHost("127.0.0.1", to_port);

	// Accept Client Socket
	tcp::endpoint accept_endpoint(tcp::v4(), from_port);
	m_pRTClient = new CRealtimeClient(this, accept_endpoint);
	return true;
}

void CRealtimeManager::Start()
{
	if (m_pSocket != NULL) {
		m_pSocket->Connect();
		m_pSocket->Start();
	}
	if (m_pRTClient != NULL) {
		m_pRTClient->Start();
	}
}

void CRealtimeManager::Finish()
{
	if (m_pSocket != NULL) {
		m_pSocket->Finish();
		delete m_pSocket;
		m_pSocket = NULL;
	}
	if (m_pRTClient != NULL) {
		m_pRTClient->Finish();
		// wait for client socket closed
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		delete m_pRTClient;
		m_pRTClient = NULL;
	}
}

bool CRealtimeManager::FromData(const char* sFrame, int nFrame)
{
	if (m_pSocket != NULL) {
		return m_pSocket->SendRequest(sFrame, nFrame);
	}
	return false;
}

bool CRealtimeManager::ToData(const char* sFrame, int nFrame)
{
	if (m_pRTClient != NULL) {
		return m_pRTClient->ToData(sFrame, nFrame);
	}
	return false;
}

