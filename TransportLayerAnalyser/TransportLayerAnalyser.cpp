#include "TransportLayerAnalyser.h"

#include <QDebug>

TransportLayerAnalyser::TransportLayerAnalyser(QWidget *parent)
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

	connect(ui.actionClient, &QAction::triggered, this, &TransportLayerAnalyser::actionToggle);
	connect(ui.actionServer, &QAction::triggered, this, &TransportLayerAnalyser::actionToggle);

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

TransportLayerAnalyser::~TransportLayerAnalyser()
{
	mClientAdapter->StopRunning();
	mClientAdapter->terminate();

	if (mServerAdapter != nullptr)
	{
		mServerAdapter->terminate();
	}
	delete mClientAdapter;
	delete mServerAdapter;
}

void TransportLayerAnalyser::setClientMode()
{
	ui.actionClient->setChecked(true);
	ui.actionServer->setChecked(false);
	ui.groupBox_packet->setEnabled(true);
	ui.groupBox_data->setEnabled(true);
	setWindowTitle(TITLE + " - Client Mode");
	mMode = CLIENT_MODE;
	ui.menuBar->setStyleSheet("background-color : blue;");
}

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

void TransportLayerAnalyser::actionToggle(bool checked)
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

void TransportLayerAnalyser::selectFile()
{
	QString filename = QFileDialog::getOpenFileName(this, "Select File", "C:/");
	ui.label_filename->setText(filename);
}

void TransportLayerAnalyser::selectOutputFolder()
{
	mOutputFileName = QFileDialog::getExistingDirectory(this, "Open Folder", "C:/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	mOutputFileName += "/output.txt";
	qDebug() << "Output Folder:" << mOutputFileName;
}

void TransportLayerAnalyser::start()
{
	ui.pushButton_start->setEnabled(false);
	ui.pushButton_stop->setEnabled(true);

	// Get settings
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

	// Start thread based on mode
	if (mMode == CLIENT_MODE)
	{
		string msg = ui.plainTextEdit_message->toPlainText().toStdString();
		if (ui.radioButton_file->isChecked())
		{
			mClientAdapter->InitWithFile(host, port, protocol, ui.label_filename->text().toStdString(), packetSize);
		}
		else if (ui.radioButton_text->isChecked())
		{
			mClientAdapter->InitWithMsg(host, port, protocol, msg, packetSize, packetCount);
		}
		else
		{
			//displayError("Input type error.");
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

void TransportLayerAnalyser::displayError(QString error)
{
	QMessageBox::critical(this, "Error", error);
}

void TransportLayerAnalyser::toggleStartButton()
{
	ui.pushButton_start->setEnabled(true);
	ui.pushButton_stop->setEnabled(false);
}

void TransportLayerAnalyser::startLogging()
{
	mBytesSent = 0;
	mStartTime = clock();
	ui.label_time->setText("Running...");
	ui.label_data_transfered->setText(LABEL_XFER + QString::number(mBytesSent / 1000) + "KB");
}

void TransportLayerAnalyser::stopLogging()
{
	clock_t stopTime = clock();
	double totalTime = (stopTime - mStartTime) / (double)CLOCKS_PER_SEC;

	ui.label_time->setText(LABEL_TIME + QString::number(totalTime) + "s");

	toggleStartButton();
}

void TransportLayerAnalyser::updateBytesLabel(int bytes)
{
	mBytesSent += bytes;
	ui.label_data_transfered->setText(LABEL_XFER + QString::number(mBytesSent / 1000) + "KB");
}
