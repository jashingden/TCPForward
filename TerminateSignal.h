
#pragma once

#include <signal.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::io_service;
using boost::asio::signal_set;

class ISignalCallback
{
public:
	virtual void Terminate() = 0;
};

class TerminateSignal
{
public:
	TerminateSignal(ISignalCallback* callback) : signals(ios) {
		this->callback = callback;
		// Register to handle the signals that indicate when the server should exit.
		// It is safe to register for the same signal multiple times in a program,
		// provided all registration for the specified signal is made through Asio.
		signals.add(SIGINT);
		signals.add(SIGTERM);
#if defined(SIGQUIT)
		signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
		signals.async_wait([this](boost::system::error_code ec, int) {
			this->callback->Terminate();
		});
		ios.run();
	}

private:
	io_service ios;
	signal_set signals;
	ISignalCallback* callback;
};
