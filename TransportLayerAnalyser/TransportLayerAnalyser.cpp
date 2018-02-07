#include "TransportLayerAnalyser.h"

#include <QDebug>

TransportLayerAnalyser::TransportLayerAnalyser(QWidget *parent)
	: QMainWindow(parent)
	, mOutputFileName("C:/")
	, mMode("")
	, mNetworkManager(nullptr)
	, mFileManager(new FileManager())
	, mStatsManager(new FileManager("", "stats.txt"))
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

	setClientMode();
}

TransportLayerAnalyser::~TransportLayerAnalyser()
{
	delete mNetworkManager;
	delete mFileManager;
}

void TransportLayerAnalyser::setClientMode()
{
	ui.actionClient->setChecked(true);
	ui.actionServer->setChecked(false);
	ui.groupBox_packet->setEnabled(true);
	ui.groupBox_data->setEnabled(true);
	ui.lineEdit_dest->setEnabled(true);
	setWindowTitle(TITLE + " - Client Mode");
	mMode = "client";
	ui.menuBar->setStyleSheet("background-color : blue;");
}

void TransportLayerAnalyser::setServerMode()
{
	ui.actionClient->setChecked(false);
	ui.actionServer->setChecked(true);
	ui.groupBox_packet->setEnabled(false);
	ui.groupBox_data->setEnabled(false);
	ui.lineEdit_dest->setEnabled(false);
	setWindowTitle(TITLE + " - Server Mode");
	mMode = "server";
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
	QString filename = QFileDialog::getOpenFileName(this, "Select File", "C:/", "Plain Text (*.txt, *.md)");
	ui.label_filename->setText(filename);
}

void TransportLayerAnalyser::selectOutputFolder()
{
	mOutputFileName = QFileDialog::getExistingDirectory(this, "Open Folder", "C:/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	mOutputFileName += "output.txt";
	mFileManager->SetOutFile(mOutputFileName.toStdString());
}

void TransportLayerAnalyser::start()
{
	// Get settings
	int packetSize = ui.lineEdit_packet_size->text().toInt();
	int packetCount = ui.lineEdit_packet_count->text().toInt();
	int port = ui.lineEdit_port->text().toInt();
	string protocol;
	string dest = ui.lineEdit_dest->text().toStdString();

	if (ui.radioButton_tcp->isChecked())
	{
		protocol = "tcp";
	}
	else if (ui.radioButton_udp->isChecked())
	{
		protocol = "udp";
	}
	else
	{
		QMessageBox::critical(this, "Error", "Protocol selection failed please try again.");
		return;
	}

	// Create new NetworkManager
	delete mNetworkManager;
	mNetworkManager = new NetworkManager(protocol, dest, port);

	// Connect NetworkManager and start in server or client mode
	if (mNetworkManager->Connect() == -1)
	{
		QMessageBox::critical(this, "Error", QString::fromStdString(mNetworkManager->ErrorMessage()));
	}
	else
	{
		if (mMode == "client")
		{
			startClient(packetSize, packetCount);
		}
		else if (mMode == "server")
		{
			startServer();
		}
		else
		{
			QMessageBox::critical(this, "Error", "Please select a operation mode.");
		}
		mNetworkManager->Disconnect();
	}
	
}

void TransportLayerAnalyser::stop()
{

}

void TransportLayerAnalyser::startClient(const int packetSize, const int packetCount)
{
	int bytesSent;
	string msg = ui.plainTextEdit_message->toPlainText().toStdString();

	for (int i = 0; i < packetCount; i++)
	{
		bytesSent = mNetworkManager->Send(msg, packetSize);

		if (bytesSent == -1)
		{
			QMessageBox::critical(this, "Error", QString::fromStdString(mNetworkManager->ErrorMessage()));
			break;
		}
	}
}

void TransportLayerAnalyser::startServer()
{
	string msg;
	while ((msg = mNetworkManager->Read()) != "")
	{
		qDebug() << "Msg:" << QString::fromStdString(msg);
	}
	qDebug() << "Server mode exitting";
}