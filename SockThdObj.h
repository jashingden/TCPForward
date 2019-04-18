
#pragma once

#include "RingBuf.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lockable_adapter.hpp>

#define MAX_LENGTH	1024000
#define	MAX_BUFFER	6553600
#define DUMP_SIZE	102400

using boost::asio::io_service;
using boost::thread;
using boost::asio::deadline_timer;
using boost::asio::ip::tcp;
using boost::recursive_mutex;
using boost::basic_lockable_adapter;

class CSocketObj : public basic_lockable_adapter<recursive_mutex>
{
public:
	CSocketObj(io_service& ios);
	virtual ~CSocketObj();

	tcp::socket& GetSocket() {
		return m_socket;
	}

	tcp::endpoint& GetHost() {
		return m_endpoint;
	}

	void SetHost(std::string szHostIP, int nHostPort) {
		m_endpoint = tcp::endpoint(boost::asio::ip::address::from_string(szHostIP), nHostPort);
	}

	void SetRemoteIP() {
		m_sRemoteIP = m_socket.remote_endpoint().address().to_string();
	}

	std::string GetRemoteIP() {
		return m_sRemoteIP;
	}

	bool IsConnect() {
		return m_bConnected;
	}

	virtual bool Create();
	virtual void Close();
	virtual void Connect();

	bool SendRequest(const char* pData, std::size_t nLength);
	bool SendRequest2(const char* pData, std::size_t nLength);

	virtual bool OnParsing() = 0;

	// using the thread itself
	//
	void Start();
	void Finish();

protected:
	void AsyncReceive();
	void BufferSend();

protected:
	// using the thread itself
	//
	io_service m_ios;
	thread m_thd;

protected:
	tcp::endpoint m_endpoint;
	bool m_bConnected;
	bool m_bClosed;
	std::string m_sRemoteIP;
	tcp::socket m_socket;
	char m_readBuf[MAX_BUFFER];
	int m_nBufSize;
	char m_bufOut[MAX_LENGTH];
	CBSRingBuffer m_writeBuf;
};

class CSocketThdObj : public basic_lockable_adapter<recursive_mutex>
{
public:
	CSocketThdObj(int nPeriod);
	virtual ~CSocketThdObj();

	void Start();
	void Finish();

	virtual void OnClearProc() = 0;
	virtual void OnRunProc() = 0;
	virtual void OnTimerProc() = 0;

protected:
	void AsyncWaitTimer();

protected:
	io_service m_ios;
	thread m_thd;
	deadline_timer m_timer;
	int m_nPeriod;
};
