/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 		ServerAdapter.cpp - An application that allows the user to interface with the transpor layer.
--
-- PROGRAM: 			TransportLayerAnalyser
--
-- FUNCTIONS:			ServerAdapter(QObject * parent)
--						~ServerAdapter()	
--						void Init(const string host, const int port, const int protocol, const string filename)
--						void StopRunning()
--						void connect()
--						void disconnect()
--						void listenTCP()
--						void listenUDP()
--						void SetErrorMessage()
--						void StopListening()
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang	
--
-- PROGRAMMER: 			Benny Wang	
--
-- NOTES:
-- This is a wrapper class for the receiving portion of Winsock2.0.
----------------------------------------------------------------------------------------------------------------------*/
#include "ServerAdapter.h"

ServerAdapter::ServerAdapter(QObject * parent)
	: QThread(parent)
	, mRunning(true)
	, mWaiting(false)
{
}

ServerAdapter::~ServerAdapter()
{
	mRunning = false;
	mWaiting = false;
}

void ServerAdapter::Init(const string host, const int port, const int protocol, const string filename)
{
	mHost = host;
	mPort = port;
	mProtocol = protocol;
	mFile = filename;
	mWaiting = true;
}

void ServerAdapter::connect()
{
	mpHost = (struct hostent *)malloc(sizeof(struct hostent));

	WORD versionRequested;
	WSADATA wsaData;

	memset(mBuffer, 0, MAX_BUFFER_LEN);
	mDestFile.open(mFile, fstream::out | fstream::binary);

	versionRequested = MAKEWORD(2, 2);
	WSAStartup(versionRequested, &wsaData);

	if (mProtocol == TCP)
	{
		if ((mSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			SetErrorMessage();
			return;
		}
	}
	else if (mProtocol == UDP)
	{
		if ((mSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
		{
			SetErrorMessage();
			return;
		}
	}
	else
	{
		mErrMsg = "Problem with selecting protocol";
		return;
	}

	if (mProtocol == TCP)
	{
		if (isdigit(*(mHost.c_str())))
		{
			mpAddress = (struct in_addr *)malloc(sizeof(in_addr));
			mpAddress->s_addr = inet_addr(mHost.c_str());
			mpHost = gethostbyaddr((char *)mpAddress, PF_INET, sizeof(*mpAddress));
			delete mpAddress;
		}
		else
		{
			mpHost = gethostbyname(mHost.c_str());
		}
	}

	if (!mpHost)
	{
		SetErrorMessage();
		return;
	}

	memset((char *)&mClient, 0, sizeof(struct sockaddr_in));
	mClient.sin_family = AF_INET;
	mClient.sin_port = htons(mPort);
	mClient.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((::bind(mSocket, (struct sockaddr *)&mClient, sizeof(mClient))) == -1)
	{
		SetErrorMessage();
		return;
	}

	if (mProtocol == TCP)
	{
		listen(mSocket, 5);
	}

	mWaiting = true;
}

void ServerAdapter::disconnect()
{

	if (mDestFile.is_open())
	{
		mDestFile.close();
	}
	if (mSocket)
	{
		closesocket(mSocket);
	}
	if (mListenSocket)
	{
		closesocket(mListenSocket);
	}
	WSACleanup();
	emit ListeningFinished();

	mWaiting = false;
}

void ServerAdapter::SetErrorMessage()
{
	int lastError = WSAGetLastError();
	string msg;

	mWaiting = false;

	switch (lastError)
	{
	case WSAEINTR:
		return;
	case WSAENOTSOCK:
		msg = "No socket given. If you get this error after stopping a TCP server please ignore this.";
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
	case WSAECONNRESET:
		msg = "Current connection was reset by the other side.";
		break;
	default:
		msg = "Error getting error";
		break;
	}

	mErrMsg = msg + "(" + to_string(lastError) + ")";
	emit ErrorOccured(QString::fromStdString(msg));
}

void ServerAdapter::listenTCP()
{
	int n = 1;
	int clientLen;

	clientLen = sizeof(mClient);
	if ((mListenSocket = accept(mSocket, (struct sockaddr *)&mClient, &clientLen)) == -1)
	{
		SetErrorMessage();
	}

	memset(mBuffer, 0, MAX_BUFFER_LEN);

	emit ReadingStarted();
	while ((n = recv(mListenSocket, mBuffer, MAX_BUFFER_LEN, 0)) < MAX_BUFFER_LEN)
	{
		if (n > 0)
		{
			mDestFile.write(mBuffer, n);
			memset(mBuffer, 0, n);
			emit BytesReceived(n);
		}

		if (n == 0)
		{
			break;
		}

		if (n == -1)
		{
			SetErrorMessage();
			break;
		}
	}

	emit ReadingStopped();
	return;
}

void ServerAdapter::listenUDP()
{
	int n;
	int serverLength = sizeof(mClient);

	emit ReadingStarted();
	while (mWaiting)
	{
		memset(mBuffer, 0, MAX_BUFFER_LEN);
		if ((n = recvfrom(mSocket, mBuffer, MAX_BUFFER_LEN, 0, (struct sockaddr *)&mClient, &serverLength)) < 0)
		{
			SetErrorMessage();
		}
		else
		{
			mDestFile.write(mBuffer, n);
			emit BytesReceived(n);
		}
	}
	emit ReadingStopped();
}

void ServerAdapter::run()
{
	int n;
	int clientLen;
	int bytesLeft = MAX_BUFFER_LEN;
	char * bp = mBuffer;

	while (mRunning)
	{
		if (mWaiting)
		{
			connect();

			if (mProtocol == TCP)
			{
				listenTCP();
			}
			else if (mProtocol == UDP)
			{
				listenUDP();
			}
			else
			{
				mRunning = false;
				mErrMsg = "Protocol error";
			}

			disconnect();
		}
		sleep(1);
	}
}

void ServerAdapter::StopRunning()
{
	mWaiting = false;
}

void ServerAdapter::StopListening()
{
	if (mDestFile.is_open())
	{
		mDestFile.close();
	}

	if (mProtocol == TCP)
	{
		if (mListenSocket)
		{
			closesocket(mListenSocket);
		}
	}

	if (mSocket)
	{
		closesocket(mSocket);
	}

	mWaiting = false;
}