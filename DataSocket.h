
#pragma once

#include "SockThdObj.h"
#include <fstream>

#define MAX_BUFSIZE		32768

class CRealtimeManager;

class CDataSocket : public CSocketObj
{
public:
	CDataSocket(CRealtimeManager* mgr);
	~CDataSocket();

	bool Create();
	void Close();
	bool OnParsing();

private:
	CRealtimeManager* m_pRTMgr;
};
