#include "ServerAdapter.h"

#include <QDebug>

ServerAdapter::ServerAdapter(QObject * parent)
	: QThread(parent)
{
}

ServerAdapter::~ServerAdapter()
{
}

void ServerAdapter::StartListening(const string host, const int port, const int protocol, const string filename)
{
	mProtocol = protocol;
	mpHost = (struct hostent *)malloc(sizeof(struct hostent));

	WORD versionRequested;
	WSADATA wsaData;

	memset(mBuffer, 0, MAX_BUFFER_LEN);

	mDestFile.open(filename, fstream::out | fstream::binary);

	// Initialize winsock
	versionRequested = MAKEWORD(2, 2);
	WSAStartup(versionRequested, &wsaData);

	if (mProtocol == TCP)
	{
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

	memset((char *)&mClient, 0, sizeof(struct sockaddr_in));
	mClient.sin_family = AF_INET;
	mClient.sin_port = htons(port);

	if (mProtocol == TCP)
	{
		mClient.sin_addr.s_addr = inet_addr(host.c_str());

	}
	else if (mProtocol == UDP)
	{
		mClient.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		qDebug() << "protocol error";
	}

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
}

void ServerAdapter::SetErrorMessage()
{
	int lastError = WSAGetLastError();
	string msg;

	mWaiting = false;

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

	emit ErrorOccured(QString::fromStdString(msg));
}

void ServerAdapter::listenTCP()
{
	int n;
	int clientLen;
	int bytesLeft = MAX_BUFFER_LEN;
	char * bp = mBuffer;

	clientLen = sizeof(mClient);
	if ((mListenSocket = accept(mSocket, (struct sockaddr *)&mClient, &clientLen)) == -1)
	{
		SetErrorMessage();
	}

	memset(mBuffer, 0, MAX_BUFFER_LEN);

	bytesLeft = MAX_BUFFER_LEN;
	bp = mBuffer;
	while ((n = recv(mListenSocket, bp, bytesLeft, 0)) < MAX_BUFFER_LEN)
	{

		bp += n;
		bytesLeft -= n;
		if (n == 0)
		{
			mDestFile.write(mBuffer, bp - mBuffer);
			break;
		}
	}

	if (n == -1)
	{
		SetErrorMessage();
	}
}

void ServerAdapter::listenUDP()
{
	int n;
	int serverLength = sizeof(mClient);

	memset(mBuffer, 0, MAX_BUFFER_LEN);

	if ((n = recvfrom(mSocket, mBuffer, MAX_BUFFER_LEN, 0, (struct sockaddr *)&mClient, &serverLength)) < 0)
	{
		SetErrorMessage();
	}
	else
	{
		mDestFile.write(mBuffer, n);
	}
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
		}
		else
		{
			disconnect();
		}

		sleep(1);
	}
}

void ServerAdapter::StopRunning()
{
	mRunning = false;
}