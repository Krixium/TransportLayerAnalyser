#pragma once

#pragma comment(lib, "ws2_32.lib")

#include <QString>
#include <QThread>

#include <fstream>
#include <string>
#include <winsock2.h>

#include "global.h"

using namespace std;

class ClientAdapter : public QThread
{
	Q_OBJECT
public:
	ClientAdapter(const string host, const int port, const int protocol, 
		const string filename, const int packetSize, QObject * parent = nullptr);
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
	ifstream mSrcFile;

	string mErrMsg;

protected:
	void run();

private:
	void connect(const string host, const int port);
	void sendFile();
	void sendPackets();
	void SetErrorMessage();

signals:
	void ErrorOccured(QString error);
};

