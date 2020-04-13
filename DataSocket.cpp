// DataSocket.cpp
//
#include "DataSocket.h"
#include "RTClient.h"
#include <iostream>
#include <boost/locale.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/endian/conversion.hpp>

#define MONITOR_TIMEOUT		1		// 1 sec
#define SOCK_SENDCHK		10		// 10 sec

using namespace std;
using namespace boost::posix_time;
using namespace boost::endian;
using boost::format;

//////////////////////////////////////////////////////////////////////////////////////
//

CDataSocket::CDataSocket(CClientSocket* client)
: CSocketObj(m_ios)
{
	m_pClient = client;
}

CDataSocket::~CDataSocket()
{

}

bool CDataSocket::Create()
{
	cout << "Data Socket is connected" << endl;
	bool ret = __super::Create();
	if (m_pClient != NULL) {
		m_pClient->Start();
		return ret;
	}
	return false;
}

void CDataSocket::Close()
{
	cout << "Data Socket is closed" << endl;
	__super::Close();
}

bool CDataSocket::OnParsing()
{
	if (m_nBufSize <= 0) {
		return true;
	}

	if (m_pClient != NULL) {
		bool dump = false;
		if (m_nBufSize > DUMP_SIZE)
			cout << "t: " << (dump ? string(m_readBuf, m_nBufSize) : to_string(m_nBufSize)) << endl;
		m_pClient->SendRequest(m_readBuf, m_nBufSize);
	}

	m_nBufSize = 0;
	return true;
}

