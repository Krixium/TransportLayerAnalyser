/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 		TransportLayerAnalyser.cpp - An application that allows the user to interface with the transpor layer.
--
-- PROGRAM: 			TransportLayerAnalyser
--
-- FUNCTIONS:			TransportLayerAnalyser(QWidget *parent)
--						~TransportLayerAnalyser()
--						void setClientMode()
--						void setServerMode()
--						void toggleStartButton()
--						void actionToggle(bool checked)
--						void selectFile()
--						void selectOutputFolder()
--						void start()
--						void stop()
--						void updateBytesLabel(int bytes)
--						void displayError(QString error)
--						void startLogging()
--						void stopLogging()
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang	
--
-- PROGRAMMER: 			Benny Wang	
--
-- NOTES:
-- This is the main entry point of the prgoram. All UI elements are initialized here and all networking classes are 
-- created and started here. This class also handles all user input.
----------------------------------------------------------------------------------------------------------------------*/
#include "TransportLayerAnalyser.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			TransportLayerAnaylser
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			TransportLayerAnalyser (QWidget * parent)
--							QWidget * parent: A pointer to the parent QWidget.
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is where the first lines non auto-generated code is put.
--
-- The constructor for the main window will connect all the Qt signal and slots required for this application.
-- It also creates both the client thread and server thread starts both. Lastly, the contructor also defaults the
-- program's running mode to client mode.
----------------------------------------------------------------------------------------------------------------------*/
TransportLayerAnalyser::TransportLayerAnalyser(QWidget * parent)
	: QMainWindow(parent)
	, mOutputFileName("C:/")
	, mMode(CLIENT_MODE)
	, mClientAdapter(new ClientAdapter(this))
	, mServerAdapter(new ServerAdapter(this))
	, mBytesSent(0)
{
	ui.setupUi(this);

	connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui.actionSelect_File, &QAction::triggered, this, &TransportLayerAnalyser::selectFile);
	connect(ui.actionSelect_Ouput_Folder, &QAction::triggered, this, &TransportLayerAnalyser::selectOutputFolder);

	connect(ui.actionClient, &QAction::triggered, this, &TransportLayerAnalyser::modeToggled);
	connect(ui.actionServer, &QAction::triggered, this, &TransportLayerAnalyser::modeToggled);

	connect(ui.radioButton_text, &QRadioButton::toggled, ui.plainTextEdit_message, &QPlainTextEdit::setEnabled);
	connect(ui.radioButton_file, &QRadioButton::toggled, ui.label_filename, &QLabel::setEnabled);

	connect(ui.pushButton_start, &QPushButton::pressed, this, &TransportLayerAnalyser::start);
	connect(ui.pushButton_stop, &QPushButton::pressed, this, &TransportLayerAnalyser::stop);
	
	connect(mClientAdapter, &ClientAdapter::SendingStarted, this, &TransportLayerAnalyser::startLogging);
	connect(mClientAdapter, &ClientAdapter::SendingFinished, this, &TransportLayerAnalyser::stopLogging);
	connect(mClientAdapter, &ClientAdapter::ErrorOccured, this, &TransportLayerAnalyser::displayError);
	connect(mClientAdapter, &ClientAdapter::SendingProgress, ui.progressBar_send, &QProgressBar::setValue);
	connect(mClientAdapter, &ClientAdapter::BytesSent, this, &TransportLayerAnalyser::updateBytesLabel);

	connect(mServerAdapter, &ServerAdapter::ReadingStarted, this, &TransportLayerAnalyser::startLogging);
	connect(mServerAdapter, &ServerAdapter::ReadingStopped, this, &TransportLayerAnalyser::stopLogging);
	connect(mServerAdapter, &ServerAdapter::ErrorOccured, this, &TransportLayerAnalyser::displayError);
	connect(mServerAdapter, &ServerAdapter::ListeningFinished, this, &TransportLayerAnalyser::toggleStartButton);
	connect(mServerAdapter, &ServerAdapter::BytesReceived, this, &TransportLayerAnalyser::updateBytesLabel);
	connect(ui.pushButton_stop, &QPushButton::pressed, mServerAdapter, &ServerAdapter::StopListening);

	toggleStartButton();

	setClientMode();

	mClientAdapter->start();
	mServerAdapter->start();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			~TransportLayerAnaylser
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			~TransportLayerAnalyser ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- Deconstructor for the main window. Stops both the client and server threads, and deletes them.
----------------------------------------------------------------------------------------------------------------------*/
TransportLayerAnalyser::~TransportLayerAnalyser()
{
	mClientAdapter->StopRunning();
	mClientAdapter->terminate();

	mServerAdapter->StopRunning();
	mServerAdapter->terminate();

	delete mClientAdapter;
	delete mServerAdapter;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			setClientMode
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void setClientMode ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- Sets the program into client mode.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::setClientMode()
{
	ui.actionClient->setChecked(true);
	ui.actionServer->setChecked(false);
	ui.groupBox_packet->setEnabled(true);
	ui.groupBox_data->setEnabled(true);
	setWindowTitle(TITLE + " - Client Mode");
	mMode = CLIENT_MODE;
	ui.menuBar->setStyleSheet("background-color : lightblue;");
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			setClientMode
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void setClientMode ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- Sets the program into server mode.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::setServerMode()
{
	ui.actionClient->setChecked(false);
	ui.actionServer->setChecked(true);
	ui.groupBox_packet->setEnabled(false);
	ui.groupBox_data->setEnabled(false);
	setWindowTitle(TITLE + " - Server Mode");
	mMode = SERVER_MODE;
	ui.menuBar->setStyleSheet("background-color : orange;");
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			modeToggled
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void modeToggled (bool checked)
--							bool checked: The new state of the caller.
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is connected to both of the radio buttons that are responsible for selecting the mode.
-- Based on which one is checked, this function will switch the program to that mode.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::modeToggled(bool checked)
{
	QAction* sender = (QAction*)QObject::sender();

	if (sender == ui.actionClient)
	{
		setClientMode();
	}

	if (sender == ui.actionServer)
	{
		setServerMode();
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			selectFile
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void selectFile ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggered when the user clicks the menu item for selecting a file. Once the
-- QFileDialog returns a file's name, this function saves it so that it can be passed to the ClientAdapter when 
-- it is needed.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::selectFile()
{
	QString filename = QFileDialog::getOpenFileName(this, "Select File", "C:/");
	ui.label_filename->setText(filename);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			selectOutputFolder
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void selectOutputFolder ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggered when the user clicks the menu item to select the output folder where
-- the incoming data is written into. The QFileDialog grabs a the name of the folder where the file will be stored
-- and then this function appends "/output.txt" to it. That then becomes the output file and is then given to the
-- ServerAdapter when it is needed.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::selectOutputFolder()
{
	mOutputFileName = QFileDialog::getExistingDirectory(this, "Open Folder", "C:/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	mOutputFileName += "/output.txt";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			start
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void start ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggered when the user clicks start. All required information for the ClientAdapter
-- and ServerAdapter is grabbed at this point. All non-network input error handling is checked here. Once the required
-- information is grabbed, this function starts the adapter that the user requested.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::start()
{
	ui.pushButton_start->setEnabled(false);
	ui.pushButton_stop->setEnabled(true);

	int packetSize = ui.lineEdit_packet_size->text().toInt();
	int packetCount = ui.lineEdit_packet_count->text().toInt();
	int port = ui.lineEdit_port->text().toInt();
	int protocol;
	string src = ui.lineEdit_dest->text().toStdString();
	string host = ui.lineEdit_dest->text().toStdString();

	if (ui.radioButton_tcp->isChecked())
	{
		protocol = TCP;
	}
	else if (ui.radioButton_udp->isChecked())
	{
		protocol = UDP;
	}
	else
	{
		displayError("Protocol selection failed, please try again.");
		ui.pushButton_start->setEnabled(true);
		ui.pushButton_stop->setEnabled(false);
		return;
	}

	if (mMode == CLIENT_MODE)
	{

		if (ui.radioButton_file->isChecked())
		{
			QString filename = ui.label_filename->text();
			if (filename == "C:\\" || filename == "")
			{
				displayError("Please select a file");
			}
			mClientAdapter->InitWithFile(host, port, protocol, ui.label_filename->text().toStdString(), packetSize);
		}
		else if (ui.radioButton_text->isChecked())
		{
			string msg = ui.plainTextEdit_message->toPlainText().toStdString();
			mClientAdapter->InitWithMsg(host, port, protocol, msg, packetSize, packetCount);
		}
		else
		{
			displayError("Input type error.");
			ui.pushButton_start->setEnabled(true);
			ui.pushButton_stop->setEnabled(false);
		}
	}
	else if (mMode == SERVER_MODE)
	{
		mServerAdapter->Init(host, port, protocol, mOutputFileName.toStdString());
	}
	else
	{
		displayError("Please select an operation mode.");
		ui.pushButton_start->setEnabled(true);
		ui.pushButton_stop->setEnabled(false);
		return;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			stop
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void stop ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggerd when the user clicks stop. This funciton will stop that adapter that relates
-- to the current running mode.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::stop()
{
	if (mMode == CLIENT_MODE)
	{
		mClientAdapter->StopRunning();
	}
	if (mMode == SERVER_MODE)
	{
		mServerAdapter->StopRunning();
	}
	toggleStartButton();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			displayError
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void displayError (QString error)
--							QString error: The error message.
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggered when an error occurs in either of the adpaters. This funciton will take the
-- error and display it in a critical QMessageBox.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::displayError(QString error)
{
	QMessageBox::critical(this, "Error", error);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			toggleStartButton
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void toggleStartButton ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggered when either of the adapters finishes their task. This function is purely
-- for the GUI.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::toggleStartButton()
{
	ui.pushButton_start->setEnabled(true);
	ui.pushButton_stop->setEnabled(false);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			startLogging
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void startLogging ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggerd when either of the adapters starts running. This initiates all things
-- required for recording stats for the current running mode.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::startLogging()
{
	mBytesSent = 0;
	mStartTime = clock();
	ui.label_time->setText("Running...");
	ui.label_data_transfered->setText(LABEL_XFER + QString::number(mBytesSent / 1000) + "KB");
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			stopLogging
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void stopLogging ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggered when either of the adapters stops running. This stops all things that are
-- used for recording stats.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::stopLogging()
{
	clock_t stopTime = clock();
	double totalTime = (stopTime - mStartTime) / (double)CLOCKS_PER_SEC;

	if (totalTime > 0)
	{
		ui.label_time->setText(LABEL_TIME + QString::number(totalTime) + "s");
	}
	else
	{
		ui.label_time->setText(LABEL_TIME + " N\A");
	}

	toggleStartButton();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: 			updateBytesLabel
--
-- DATE: 				Feb 5, 2018
--
-- DESIGNER: 			Benny Wang
--
-- PROGRAMMER: 			Benny Wang
--
-- INTERFACE: 			void updateBytesLabel ()
--
-- RETURNS: 			Void.
--
-- NOTES:
-- This is a Qt slot. This slot is triggerd when either of the adapter sends or reads data. This adds it to the current
-- running count and displays it to the user.
----------------------------------------------------------------------------------------------------------------------*/
void TransportLayerAnalyser::updateBytesLabel(int bytes)
{
	mBytesSent += bytes;
	ui.label_data_transfered->setText(LABEL_XFER + QString::number(mBytesSent / 1000) + "KB");
}
