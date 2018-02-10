#include "ClientAdapter.h"

#include <QDebug>

ClientAdapter::ClientAdapter(QObject * parent)
	: QThread(parent)
{
	mRunning = true;
	mSending = false;
}

ClientAdapter::~ClientAdapter()
{
}

void ClientAdapter::InitWithFile(const string host, const int port, const int protocol, const string filename, const int packetSize)
{
	mHostname = host;
	mPort = port;
	mProtocol = protocol;
	mPacketSize = packetSize;
	mSrcFile.open(filename, fstream::in | fstream::binary);
	mMessage = nullptr;
	mPacketCount = -1;

	mSending = true;
}

void ClientAdapter::InitWithMsg(const string host, const int port, const int protocol, const string msg, const int packetSize, const int packetCount)
{
	mHostname = host;
	mPort = port;
	mProtocol = protocol;
	mPacketSize = packetSize;
	mPacketCount = packetCount;

	mMessage = new char[packetSize];
	memset(mMessage, 0, packetSize);
	strcpy(mMessage, msg.c_str());

	mSending = true;
}

void ClientAdapter::connect()
{
	WORD versionRequested;
	WSADATA wsaData;

	// Initialize winsock
	versionRequested = MAKEWORD(2, 2);
	WSAStartup(versionRequested, &wsaData);

	if (isdigit(*(mHostname.c_str())))
	{
		mpAddress = (struct in_addr *)malloc(sizeof(in_addr));
		mpAddress->s_addr = inet_addr(mHostname.c_str());
		mpHost = gethostbyaddr((char *)mpAddress, PF_INET, sizeof(*mpAddress));
		delete mpAddress;
	}
	else
	{
		mpHost = gethostbyname(mHostname.c_str());
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
		mServer.sin_port = htons(mPort);
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
		mServer.sin_port = htons(mPort);
		memcpy((char *)&mServer.sin_addr, mpHost->h_addr, mpHost->h_length);
	}
	else
	{
		return;
	}
}

void ClientAdapter::disconnect()
{
	mSending = false;
	if (mSrcFile.is_open())
	{
		mSrcFile.close();
	}
	if (mSocket)
	{
		closesocket(mSocket);
	}
	WSACleanup();
	emit SendingFinished();
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
	case WSAEMSGSIZE:
		msg = "Packet size is too long";
		break;
	default:
		msg = "Error getting error";
		break;
	}

	mErrMsg = msg + "(" + to_string(lastError) + ")";

	emit ErrorOccured(QString::fromStdString(msg));
}

void ClientAdapter::sendFile()
{
	int n;
	int progress = 0;
	double total = mPacketSize * mPacketCount;
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
				mSending = false;
				break;
			}

			if (n > 0)
			{
				progress += n;
				emit AmountSent(floor((progress / total) * 100));
			}

			if (!mSending)
			{
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

			if (n < 0)
			{
				SetErrorMessage();
				mSending = false;
				break;
			}
			else
			{
				progress += n;
				emit AmountSent(floor((progress / total) * 100));
			}
			if (!mSending)
			{
				break;
			}
		}
	}
	else
	{
		mSending = false;
	}
	delete buffer;
}

void ClientAdapter::sendPackets()
{
	int n;
	int progress = 0;
	double total = mPacketSize * mPacketCount;

	if (mProtocol == TCP)
	{
		for (int i = 0; i < mPacketCount; i++)
		{
			if ((n = send(mSocket, mMessage, mPacketSize, NULL)) == -1)
			{
				SetErrorMessage();
				mSending = false;
				break;
			}
			else
			{
				progress += n;
				emit AmountSent(floor((progress / total) * 100));
			}
			if (!mSending)
			{
				break;
			}
		}
		closesocket(mSocket);
	}
	else if (mProtocol == UDP)
	{
		for (int i = 0; i < mPacketCount; i++)
		{
			if ((n = sendto(mSocket, mMessage, mPacketSize, NULL, (struct sockaddr *)&mServer, sizeof(mServer))) == -1)
			{
				SetErrorMessage();
				mSending = false;
				break;
			}
			else
			{
				progress += n;
				emit AmountSent(floor((progress / total) * 100));
			}
			if (!mSending)
			{
				break;
			}
		}
	}
	else
	{
		mSending = false;
	}
}

void ClientAdapter::run()
{
	while (mRunning)
	{
		if (mSending)
		{
			connect();

			if (mPacketCount == -1)
			{
				sendFile();
			}
			else
			{
				sendPackets();
			}

			disconnect();
		}
		sleep(1);
	}
}

void ClientAdapter::StopRunning()
{
	mRunning = false;
}