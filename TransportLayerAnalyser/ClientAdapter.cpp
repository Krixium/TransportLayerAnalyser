#include "ClientAdapter.h"

#include <QDebug>

ClientAdapter::ClientAdapter(const string host, const int port, const int protocol,
	const string filename, const int packetSize, QObject * parent)
	: QThread(parent)
	, mProtocol(protocol)
	, mPacketSize(packetSize)
	, mPacketCount(-1)
{
	mSrcFile.open(filename, fstream::in | fstream::binary);

	mMessage = nullptr;

	connect(host, port);
}

ClientAdapter::ClientAdapter(const string host, const int port, const int protocol, 
	const string msg, const int packetSize, const int packetCount, QObject * parent)
	: QThread(parent)
	, mProtocol(protocol)
	, mPacketSize(packetSize)
	, mPacketCount(packetCount)
{
	mMessage = new char[packetSize];
	memset(mMessage, 0, packetSize);
	strcpy(mMessage, msg.c_str());

	connect(host, port);
}

void ClientAdapter::connect(const string host, const int port)
{
	WORD versionRequested;
	WSADATA wsaData;

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

	if (!mpHost)
	{
		SetErrorMessage();
		return;
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
	if (mSrcFile.is_open())
	{
		mSrcFile.close();
	}
	if (mSocket)
	{
		closesocket(mSocket);
	}
	WSACleanup();
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
	emit ErrorOccured(QString::fromStdString(msg));

}

void ClientAdapter::sendFile()
{
	int n;
	char * buffer = new char[mPacketSize + 1];
	memset(buffer, 0, mPacketSize + 1);

	if (mProtocol == TCP)
	{
		while (!mSrcFile.eof())
		{
			mSrcFile.read(buffer, mPacketSize);
			if (strlen(buffer) < mPacketCount)
			{
				n = send(mSocket, buffer, strlen(buffer), NULL);
			}
			else
			{
				n = send(mSocket, buffer, mPacketSize, NULL);
			}

			if (n == -1)
			{
				SetErrorMessage();
				mRunning = false;
				break;
			}
			memset(buffer, 0, mPacketSize);
		}
		closesocket(mSocket);
	}
	else if (mProtocol == UDP)
	{
		while (!mSrcFile.eof())
		{
			mSrcFile.get(buffer, mPacketSize, EOF);
			if (strlen(buffer) < mPacketCount)
			{
				n = sendto(mSocket, buffer, strlen(buffer), NULL, (struct sockaddr *)&mServer, sizeof(mServer));
			}
			else
			{
				n = sendto(mSocket, buffer, mPacketSize, NULL, (struct sockaddr *)&mServer, sizeof(mServer));
			}

			if (n == -1)
			{
				SetErrorMessage();
				mRunning = false;
				break;
			}
		}
	}
	else
	{
		mRunning = false;
		qDebug() << "Protocol select error";
	}
	delete buffer;
}

void ClientAdapter::sendPackets()
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
			else 
			{
				qDebug() << "sending udp packet";
			}
		}
	}
	else
	{
		mRunning = false;
		qDebug() << "Protocol select error";
	}
}

void ClientAdapter::run()
{
	if (mPacketCount == -1)
	{
		sendFile();
	}
	else
	{
		sendPackets();
	}
}