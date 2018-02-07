#pragma once

#define MAX_BUFFER_LEN 65000

#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <winsock2.h>

using namespace std;

class NetworkManager
{
public:
	NetworkManager() = default;
	NetworkManager(const string protocol, const string host, const int port);
	~NetworkManager();

	const int Connect();
	void Disconnect();

	const int Send(const string data, const int size);
	const string Read();

	const string ErrorMessage();

private:
	WORD wVersionRequested;
	WSADATA wsaData;

	string mProtocol;
	int mPort;
	struct hostent * mpHost;

	SOCKET mSocket;
	int mConnectionStatus;
	struct sockaddr_in mServer;

	int mErr;

	const int connectTCP();
	const int connectUDP();







	bool mCheese;
};