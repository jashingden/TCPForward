
#pragma once

#include "SockThdObj.h"
#include <fstream>

#define MAX_BUFSIZE		32768

class CClientSocket;

class CDataSocket : public CSocketObj
{
public:
	CDataSocket(CClientSocket* client);
	~CDataSocket();

	bool Create();
	void Close();
	bool OnParsing();

private:
	CClientSocket* m_pClient;
};
