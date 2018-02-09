#pragma once

#pragma comment(lib, "ws2_32.lib")

#include <QString>
#include <QThread>

#include <fstream>
#include <string>
#include <winsock2.h>

#include "global.h"

using namespace std;

class ServerAdapter : public QThread
{
	Q_OBJECT
public:
	ServerAdapter(const string host, const int port, const int protocol,
		const string filename, QObject * parent = nullptr);
	~ServerAdapter();

	bool mRunning;
	int mProtocol;
	struct hostent * mpHost;
	struct in_addr * mpAddress;
	SOCKET mSocket;
	SOCKET mListenSocket;
	sockaddr_in mClient;

	char mBuffer[MAX_BUFFER_LEN];
	ofstream mDestFile;

	string mErrMsg;

protected:
	void run();

private:
	void SetErrorMessage();

signals:
	void ErrorOccured(QString error);
};

