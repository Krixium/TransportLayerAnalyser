#include "ClientAdapter.h"

#include <QDebug>

ClientAdapter::ClientAdapter(const string host, const int port, const int protocol, 
	const string msg, const int packetSize, const int packetCount, QObject * parent)
	: QThread(parent)
	, mProtocol(protocol)
	, mPacketSize(packetSize)
	, mPacketCount(packetCount)
{
	WORD versionRequested;
	WSADATA wsaData;

	mMessage = new char[packetSize];
	memset(mMessage, 0, packetSize);
	strcpy(mMessage, msg.c_str());

	// Initialize winsock
	versionRequested = MAKEWORD(2, 2);
	WSAStartup(versionRequested, &wsaData);

	if (isdigit(*(host.c_str())))
	{
		mpAddress = (struct in_addr *)malloc(sizeof(in_addr));
		mpAddress->s_addr = inet_addr(host.c_str());
		mpHost = gethostbyaddr((char *)mpAddress, PF_INET, sizeof(*mpAddress));
		delete mpAddress;
	}
	else
	{
		mpHost = gethostbyname(host.c_str());
	}

	// Connect to socket
	if (mProtocol == TCP)
	{
		if ((mSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			SetErrorMessage();
			return;
		}

		// Create Stuct for address
		memset((char *)&mServer, 0, sizeof(mServer));
		mServer.sin_family = AF_INET;
		mServer.sin_port = htons(port);
		memcpy((char *)&mServer.sin_addr, mpHost->h_addr, mpHost->h_length);

		if (::connect(mSocket, (struct sockaddr *)&mServer, sizeof(mServer)) == -1)
		{
			SetErrorMessage();
			return;
		}
	}
	else if (mProtocol == UDP)
	{
		if ((mSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			SetErrorMessage();
			return;
		}

		// Create Stuct for address
		memset((char *)&mServer, 0, sizeof(mServer));
		mServer.sin_family = AF_INET;
		mServer.sin_port = htons(port);
		memcpy((char *)&mServer.sin_addr, mpHost->h_addr, mpHost->h_length);
	}
	else
	{
		return;
	}
}

ClientAdapter::~ClientAdapter()
{
	delete mMessage;
	if (mSocket)
	{
		closesocket(mSocket);
	}
	WSACleanup();
}

const string ClientAdapter::GetLastErrorMessage()
{
	return mErrMsg;
}

void ClientAdapter::StopSending()
{
	mRunning = false;
}

void ClientAdapter::SetErrorMessage()
{
	int lastError = WSAGetLastError();
	string msg;

	switch (lastError)
	{
	case WSAENOTSOCK:
		msg = "No socket given.";
		break;
	case WSAENOBUFS:
		msg = "No buffer space.";
		break;
	case WSAENOTCONN:
		msg = "No connection.";
		break;
	case WSAEDESTADDRREQ:
		msg = "Destination required.";
		break;
	case WSAEHOSTUNREACH:
		msg = "Unreachable host.";
		break;
	case WSAESHUTDOWN:
		msg = "Socket has already been shutdown.";
		break;
	case WSAEADDRNOTAVAIL:
		msg = "The remote address is not a valid address.";
		break;
	case WSAECONNREFUSED:
		msg = "Connection was refused";
		break;
	default:
		msg = "Error getting error";
		break;
	}

	mErrMsg = msg + "(" + to_string(lastError) + ")";
	qDebug() << QString::fromStdString(mErrMsg);
}

void ClientAdapter::run()
{
	if (mProtocol == TCP)
	{
		for (int i = 0; i < mPacketCount; i++)
		{
			if ((send(mSocket, mMessage, mPacketSize, NULL)) == -1)
			{
				SetErrorMessage();
				mRunning = false;
			}
			else
			{
				qDebug() << "msg sent";
			}
		}
		closesocket(mSocket);
	}
	else if (mProtocol == UDP)
	{
		for (int i = 0; i < mPacketCount; i++)
		{
			if (sendto(mSocket, mMessage, mPacketSize, NULL, (struct sockaddr *)&mServer, sizeof(mServer)) == -1)
			{
				SetErrorMessage();
				mRunning = false;
			}
		}
	}
	else
	{
		mRunning = false;
		qDebug() << "Protocol select error";
	}
}

