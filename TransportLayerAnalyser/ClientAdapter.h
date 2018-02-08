#pragma once

#pragma comment(lib, "ws2_32.lib")

#include <QThread>

#include <string>
#include <winsock2.h>

#include "global.h"

using namespace std;

class ClientAdapter : public QThread
{
	Q_OBJECT
public:
	ClientAdapter(const string host, const int port, const int protocol, 
		const string msg, const int packetSize, const int packetCount, QObject * parent = nullptr);
	~ClientAdapter();

	bool mRunning;
	int mProtocol;
	struct hostent * mpHost;
	struct in_addr * mpAddress;
	SOCKET mSocket;
	sockaddr_in mServer;

	char * mMessage;
	int mPacketCount;
	int mPacketSize;

	string mErrMsg;

	const string GetLastErrorMessage();

	void StopSending();

protected:
	void run();

private:
	void SetErrorMessage();
};

