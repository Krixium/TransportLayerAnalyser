/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 		ClientAdpater.cpp - An application that allows the user to interface with the transpor layer.
--
-- PROGRAM: 			TransportLayerAnalyser
--
-- FUNCTIONS:			ClientAdapter(QObject * parent)
--						~ClientAdapter()
--						void InitWithFile(const string host, const int port, const int protocol, const string filename,
--											const int packetSize)
--						void InitWithMsg(const string host, const int port, const int protocol, const string msg,
--											const int packetSize, const int packetCount)
--						void run()
--						void connect()
--						void disconnect()
--						void sendFile()
--						void sendPackets()
--						void SetErrorMessage()
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang	
--
-- PROGRAMMER: 			Benny Wang	
--
-- NOTES:
-- This is a wrapper class for the sending portion of Winsock2.0.
----------------------------------------------------------------------------------------------------------------------*/
#include "ClientAdapter.h"


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			ClientAdapter
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			ClientAdapter (QWidget * parent)
--							QWidget * parent: A pointer to the parent QWidget.
--
-- RETURNS: 			Void.
--
-- NOTES:
-- Generic thread constuctor.
----------------------------------------------------------------------------------------------------------------------*/
ClientAdapter::ClientAdapter(QObject * parent)
	: QThread(parent)
{
	mRunning = true;
	mSending = false;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			~ClientAdapter
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			~ClientAdapter ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- Sets both control booleans to false when the class is destroyed.
----------------------------------------------------------------------------------------------------------------------*/
ClientAdapter::~ClientAdapter()
{
	mRunning = false;
	mSending = false;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			InitWithFile
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			InitWithFile (const string host, const int port, const int protocol, const string filename, const int packetSize)
--							const string host: The host name of the sender.
--							const int port: The port to use for receiving.
--							const int protocol: The protocol to use.
--							const string filename: The file to read from.
--							const int packetSize: The size of packets to send.
--
-- RETURNS: 			void.
--
-- NOTES:
-- This funciton sets all variables required for sending a file and nothing else.
----------------------------------------------------------------------------------------------------------------------*/
void ClientAdapter::InitWithFile(const string host, const int port, const int protocol, const string filename, const int packetSize)
{
	mHostname = host;
	mPort = port;
	mProtocol = protocol;
	mPacketSize = packetSize;
	mSrcFile.open(filename, fstream::in | fstream::binary);
	mMessage = nullptr;
	mPacketCount = -1;

	mSrcFile.seekg(0, mSrcFile.end);
	mFileSize = mSrcFile.tellg();
	mSrcFile.seekg(0, mSrcFile.beg);

	mSending = true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			InitWithMsg
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			InitWithMsg (const string host, const int port, const int protocol, const string msg, const int packetSize, const int packtCount)
--							const string host: The host name of the sender.
--							const int port: The port to use for receiving.
--							const int protocol: The protocol to use.
--							const int packetSize: The size of packets to send.
--							const int packetCount: The amount of packets to send.
--
-- RETURNS: 			void.
--
-- NOTES:
-- This funciton sets all variables required for sending a batch of packets and nothing else.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			connect
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			connect ()
--
-- RETURNS: 			void.
--
-- NOTES:
-- This function creates the connection for winsock. This function will start winsock, get a socket, grab the host.
-- If the mode is TCP, it will also attempt to connect.
----------------------------------------------------------------------------------------------------------------------*/
void ClientAdapter::connect()
{
	WORD versionRequested;
	WSADATA wsaData;

	versionRequested = MAKEWORD(2, 2);
	WSAStartup(versionRequested, &wsaData);

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
		if ((mSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			SetErrorMessage();
			return;
		}
	}
	else
	{
		return;
	}

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
	
	memset((char *)&mServer, 0, sizeof(struct sockaddr_in));
	mServer.sin_family = AF_INET;
	mServer.sin_port = htons(mPort);
	memcpy((char *)&mServer.sin_addr, mpHost->h_addr, mpHost->h_length);

	if (mProtocol == TCP)
	{
		if (::connect(mSocket, (struct sockaddr *)&mServer, sizeof(mServer)) == -1)
		{
			SetErrorMessage();
			return;
		}
	}

	emit SendingStarted();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			disconnect
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			disconnect ()
--
-- RETURNS: 			void.
--
-- NOTES:
-- This function disconnects the program from winsock. It will close the read file if it was open and also close any
-- sockets that were used.
----------------------------------------------------------------------------------------------------------------------*/
void ClientAdapter::disconnect()
{
	emit SendingFinished();

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
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			SetErrorMessage
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			SetErrorMessage ()
--
-- RETURNS: 			void.
--
-- NOTES:
-- This function coverts a winsock error code into a human readable message and stops the worker thread, but doesn't
-- kill it. The error message is then saved to the object for when it needs to be read and emits a signal for someone 
-- to do something with the error.
----------------------------------------------------------------------------------------------------------------------*/
void ClientAdapter::SetErrorMessage()
{
	int lastError = WSAGetLastError();
	string msg;

	switch (lastError)
	{
	case WSAEINTR:
		return;
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			sendFile
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			sendFile ()
--
-- RETURNS: 			void.
--
-- NOTES:
-- This is the function that handles the sending of the file. Sending is specific to the specificed protocol, but this 
-- funciton will grab the users specificed packetSize amount of bytes from the file and send it. If an error is encounted
-- during sending, transmission is stopped.
----------------------------------------------------------------------------------------------------------------------*/
void ClientAdapter::sendFile()
{
	int n;
	double progress = 0;
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
				emit SendingProgress(floor((progress / mFileSize) * 100));
				emit BytesSent(n);
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
				emit SendingProgress(floor((progress / mFileSize) * 100));
				emit BytesSent(n);
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			sendPackets
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			sendPackets ()
--
-- RETURNS: 			void.
--
-- NOTES:
-- This funciton handles the sending of user specified packets. Sending is specific to each protocol but, this funciton
-- will transmit packets of packetSize, packetCount amount of times. If an error is encounted during sending, transmission
-- is stopped.
----------------------------------------------------------------------------------------------------------------------*/
void ClientAdapter::sendPackets()
{
	int n;
	double progress = 0;
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
				emit SendingProgress(floor((progress / total) * 100));
				emit BytesSent(n);
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
				emit SendingProgress(floor((progress / total) * 100));
				emit BytesSent(n);
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			run
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			run ()
--
-- RETURNS: 			void.
--
-- NOTES:
-- This is the main function of a QThead. This function will run until the program is closed. When the mWaiting flag is
-- set, this function will connect, start sending based on the protocol specified, disconnect, and repeat.
----------------------------------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			StopRunning
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			StopRunning ()
--
-- RETURNS: 			void.
--
-- NOTES:
-- This function sets the running variable to false to stop the thread from working. The thread is not killed.
----------------------------------------------------------------------------------------------------------------------*/
void ClientAdapter::StopRunning()
{
	mRunning = false;
}