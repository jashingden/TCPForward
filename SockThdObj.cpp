//
// SockThdObj.cpp
//
#include "SockThdObj.h"
#include <algorithm>
#include <iostream>
#include <boost/format.hpp>

using namespace std;
using namespace boost::asio;
using boost::format;

///////////////////////////////////////////////////////////////////////////////
//

CSocketObj::CSocketObj(io_service& ios)
: m_socket(ios)
{
	m_nBufSize = 0;
	m_bConnected = false;
	m_bClosed = false;
}

CSocketObj::~CSocketObj()
{
}

bool CSocketObj::Create()
{
	if (!m_writeBuf.Create(MAX_BUFFER)){
		return false;
	}

	m_bClosed = false;
	AsyncReceive();
	return true;
}

void CSocketObj::Close()
{
	if (m_bClosed) {
		m_nBufSize = 0;
		m_bConnected = false;
		return;
	}
	m_bClosed = true;

	boost::system::error_code ec;
	m_socket.shutdown(tcp::socket::shutdown_both, ec);
	m_socket.close();
	m_nBufSize = 0;
	m_bConnected = false;
}

void CSocketObj::AsyncReceive()
{
	size_t lSize = MAX_BUFFER - m_nBufSize;
	m_socket.async_receive(buffer(m_readBuf + m_nBufSize, lSize), [this](boost::system::error_code ec, size_t bytes_transferred) {
		if (ec && bytes_transferred <= 0) {
			// socket closed
			Close();
			return;
		}
		m_nBufSize += bytes_transferred;
		if (!OnParsing()) {

		}
		AsyncReceive();
	});
}

void CSocketObj::Connect()
{
	m_bConnected = true;
	m_socket.async_connect(m_endpoint, [this](boost::system::error_code ec) {
		if (ec) {
			Close();
		} else {
			// socket connected
			Create();
		}
	});
}

bool CSocketObj::SendRequest(const char* pData, size_t nLength)
{
	boost::lock_guard<CSocketObj> guard(*this);

	try
	{
		return SendRequest2(pData, nLength);
	}
	catch (...)
	{
	}
	return false;
}

bool CSocketObj::SendRequest2(const char* pData, size_t nLength)
{
	bool isSending = m_writeBuf.GetQueuedByteCount() > 0;
	bool isOverrun = false;
	m_writeBuf.Write(pData, nLength, isOverrun);
	if (!isSending) {
		BufferSend();
	}
	return !isOverrun;
}

void CSocketObj::BufferSend()
{
	size_t lSize = min((size_t)MAX_LENGTH, m_writeBuf.GetQueuedByteCount());
	m_writeBuf.Read(m_bufOut, lSize, false);
	async_write(m_socket, buffer(m_bufOut, lSize), [this](boost::system::error_code ec, size_t bytes_transferred) {
		if (ec && bytes_transferred <= 0) {
			// socket closed
			Close();
			return;
		}
		boost::lock_guard<CSocketObj> guard(*this);

		m_writeBuf.Purge(bytes_transferred);
		if (m_writeBuf.GetQueuedByteCount() > 0) {
			BufferSend();
		}
	});
}

void CSocketObj::Start()
{
	m_thd = boost::thread([this]() {
		m_ios.run();
	});
}

void CSocketObj::Finish()
{
	m_ios.stop();
	m_thd.join();
}

///////////////////////////////////////////////////////////////////////////////
//

CSocketThdObj::CSocketThdObj(int nPeriod)
: m_timer(m_ios)
{
	m_nPeriod = nPeriod;
}

CSocketThdObj::~CSocketThdObj()
{
}

void CSocketThdObj::Start()
{
	AsyncWaitTimer();
	m_thd = boost::thread([this]() {
		OnRunProc();
		m_ios.run();
	});
}

void CSocketThdObj::Finish()
{
	m_timer.cancel();
	m_ios.stop();
	m_thd.join();

	OnClearProc();
}

void CSocketThdObj::AsyncWaitTimer()
{
	m_timer.expires_from_now(boost::posix_time::seconds(m_nPeriod));
	m_timer.async_wait([this](boost::system::error_code ec) {
		if (ec == boost::asio::error::operation_aborted) {
			cout << "CSocketThdObj AsyncWaitTimer operation_aborted" << endl;
		}
		
		OnTimerProc();
		AsyncWaitTimer();
	});
}

