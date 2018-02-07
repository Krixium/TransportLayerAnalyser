#include "NetworkManager.h"

#include <QDebug>

NetworkManager::NetworkManager(const string protocol, const string host, const int port)
	: mProtocol(protocol)
	, mPort(port)
	, mSocket(-1)
	, mConnectionStatus(-1)
	, mErr()
{
	struct in_addr * pAddress;

	// Initialize winsock
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	if (isdigit(*(host.c_str())))
	{
		pAddress->s_addr = inet_addr(host.c_str());
		mpHost = gethostbyaddr((char *)pAddress, PF_INET, sizeof(*pAddress));
	}
	else
	{
		mpHost = gethostbyname(host.c_str());
	}
}

NetworkManager::~NetworkManager()
{
	Disconnect();
	WSACleanup();
}

const int NetworkManager::Connect()
{
	if (mProtocol == "tcp")
	{
		return connectTCP();
	}
	else if (mProtocol == "udp")
	{
		return connectUDP();
	}
	else
	{
		return -1;
	}
}

void NetworkManager::Disconnect()
{
	if (mSocket)
	{
		closesocket(mSocket);
	}
}

const int NetworkManager::connectTCP()
{
	// Connect to socket
	if ((mSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		mErr = WSAGetLastError();
		return 0;
	}

	// Create Stuct for address
	memset((char *)&mServer, 0, sizeof(struct sockaddr_in));
	mServer.sin_family = AF_INET;
	mServer.sin_port = htons(mPort);

	// Check host
	if (mpHost == NULL)
	{
		mErr = WSAGetLastError();
		return 0;
	}

	// Save important information
	memcpy((char *)&mServer.sin_addr, mpHost->h_addr, mpHost->h_length);

	// Connect to server
	if ((mConnectionStatus = connect(mSocket, (struct sockaddr *)&mServer, sizeof(mServer))) == -1)
	{
		mErr = WSAGetLastError();
		return 0;
	}

	return 1;
}

const int NetworkManager::connectUDP()
{
	// Connect to socket
	if ((mSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		mErr = WSAGetLastError();
		return 0;
	}

	memset((char *)&mServer, 0, sizeof(mServer));
	mServer.sin_family = AF_INET;
	mServer.sin_port = htons(mPort);

	// Check host
	if (mpHost == NULL)
	{
		mErr = WSAGetLastError();
		return 0;
	}

	// Save important information and exit
	memcpy((char *)&mServer.sin_addr, mpHost->h_addr, mpHost->h_length);
	return 1;
}

const int NetworkManager::Send(const string data, const int size)
{
	char * buffer = new char[size];
	memset(buffer, 0, size);
	strcpy(buffer, data.c_str());
	int result;

	if (mProtocol == "tcp")
	{
		result = send(mSocket, buffer, size, NULL);
		if (result == -1)
		{
			mErr = WSAGetLastError();
		}
		delete buffer;
	}
	else if (mProtocol == "udp")
	{
		result = sendto(mSocket, buffer, size, NULL, (struct sockaddr *)&mServer, sizeof(mServer));
		if (result == -1)
		{
			mErr = WSAGetLastError();
		}
		delete buffer;
	}
	else
	{
		delete buffer;
		result = -1;
	}

	return result;
}

const string NetworkManager::Read()
{
	char buffer[MAX_BUFFER_LEN];
	memset(buffer, 0, MAX_BUFFER_LEN);
	char * bp = buffer;

	if (mProtocol == "tcp")
	{
		int n;
		int bytesLeft = MAX_BUFFER_LEN;

		while ((n = recv(mSocket, bp, bytesLeft, 0)) < MAX_BUFFER_LEN)
		{
			bp += n;
			bytesLeft -= n;
			if (n == 0)
			{
				break;
			}
		}

		return string(buffer);
	}
	else if (mProtocol == "udp")
	{
		int serverLength = sizeof(mServer);
		if (recvfrom(mSocket, buffer, MAX_BUFFER_LEN, 0, (struct sockaddr *)&mServer, &serverLength) < 0)
		{
			return "";
		}
		else
		{
			return string(buffer);
		}
	}
	else
	{
		return "";
	}
}

const string NetworkManager::ErrorMessage()
{
	string msg;

	qDebug() << "Error code:" << mErr;

	switch (mErr)
	{
	case WSAENOTSOCK:
		msg = "No socket given.";
		break;
	case WSAENOBUFS:
		msg = "No buffer space";
		break;
	case WSAENOTCONN:
		msg = "No connection";
		break;
	case WSAEDESTADDRREQ:
		msg = "Destination required";
		break;
	case WSAEHOSTUNREACH:
		msg = "Unreachable host";
		break;
	case WSAESHUTDOWN:
		msg = "Socket has already been shutdown";
		break;
	case WSAEADDRNOTAVAIL:
		msg = "The remote address is not a valid address";
		break;
	default:
		msg = "Error getting error";
		break;
	}

	return msg;
}