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
	ServerAdapter(QObject * parent = nullptr);
	~ServerAdapter();

	bool mRunning;
	bool mWaiting;

	int mProtocol;
	int mPort;
	string mHost;
	struct hostent * mpHost;
	struct in_addr * mpAddress;
	SOCKET mSocket;
	SOCKET mListenSocket;
	sockaddr_in mClient;

	char mBuffer[MAX_BUFFER_LEN];
	string mFile;
	ofstream mDestFile;

	string mErrMsg;

	void Init(const string host, const int port, const int protocol, const string filename);

	void StopRunning();

protected:
	void run();

private:
	void connect();
	void disconnect();
	void listenTCP();
	void listenUDP();

	void SetErrorMessage();

signals:
	void ReadingStarted();
	void ReadingStopped();

	void ErrorOccured(QString error);
	void ListeningFinished();
	void BytesReceived(int bytes);

public slots:
	void StopListening();
};

