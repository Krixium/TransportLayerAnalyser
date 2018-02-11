#pragma once

#pragma comment(lib, "ws2_32.lib")

#include <QString>
#include <QThread>

#include <fstream>
#include <math.h>
#include <string>
#include <winsock2.h>

#include "global.h"

using namespace std;

class ClientAdapter : public QThread
{
	Q_OBJECT
public:
	ClientAdapter(QObject * parent = nullptr);
	~ClientAdapter();

	bool mRunning;
	bool mSending;

	string mHostname;
	int mPort;
	int mProtocol;
	struct hostent * mpHost;
	struct in_addr * mpAddress;
	SOCKET mSocket;
	sockaddr_in mServer;

	char * mMessage;
	int mPacketCount;
	int mPacketSize;
	ifstream mSrcFile;
	int mFileSize;

	string mErrMsg;

	void InitWithFile(const string host, const int port, const int protocol, const string filename, const int packetSize);
	void InitWithMsg(const string host, const int port, const int protocol, const string msg, const int packetSize, const int packetCount);

	void StopRunning();

protected:
	void run();

private:
	void connect();
	void disconnect();
	void sendFile();
	void sendPackets();
	void SetErrorMessage();

signals:
	void AmountSent(int progress);
	void SendingFinished();
	void ErrorOccured(QString error);
};

