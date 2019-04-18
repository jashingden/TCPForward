// TCPForward.cpp : 定義主控台應用程式的進入點。
//
#include <iostream>
#include "TerminateSignal.h"
#include "RTManager.h"

using namespace std;

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			cout << "Usage: TCPForward <from_port> <to_port>\n";
			cout << "  Example:\n";
			cout << "    TCPForward 5585 5555\n";
			return 1;
		}

		int from_port = atoi(argv[1]);
		int to_port = atoi(argv[2]);
		CRealtimeManager mgr;
		if (!mgr.Init(from_port, to_port)) {
			cout << "CRealtimeManager initialize failed." << endl;
			return 2;
		}
		mgr.Start();

		class CB : public ISignalCallback {
		private:
			CRealtimeManager* mgr;
		public:
			CB(CRealtimeManager* mgr) {
				this->mgr = mgr;
			}
			void Terminate() {
				cout << "TCPForward has shutdown." << endl;
				mgr->Finish();
			}
		} callback(&mgr);
		TerminateSignal signal(&callback);
	}
	catch (exception& e)
	{
		cout << "Exception: " << e.what() << endl;
	}
	return 0;
}

