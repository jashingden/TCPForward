
#pragma once

class CDataSocket;
class CRealtimeClient;

class CRealtimeManager
{
public:
	CRealtimeManager();
	~CRealtimeManager();

	bool Init(int from_port, int to_port);
	void Start();
	void Finish();

	bool FromData(const char* sFrame, int nFrame);
	bool ToData(const char* sFrame, int nFrame);

private:
	CDataSocket* m_pSocket;
	CRealtimeClient* m_pRTClient;
};
