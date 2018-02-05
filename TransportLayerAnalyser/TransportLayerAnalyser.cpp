#include "TransportLayerAnalyser.h"

#include <QDebug>

TransportLayerAnalyser::TransportLayerAnalyser(QWidget *parent)
	: QMainWindow(parent)
	, mOutputFileName("C:/")
	, mMode("")
	, mNetworkManager(nullptr)
	, mFileManager(new FileManager())
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
	setWindowTitle(TITLE + " - Client Mode");
	mMode = "client";
}

void TransportLayerAnalyser::setServerMode()
{
	ui.actionClient->setChecked(false);
	ui.actionServer->setChecked(true);
	ui.groupBox_packet->setEnabled(false);
	ui.groupBox_data->setEnabled(false);
	setWindowTitle(TITLE + " - Server Mode");
	mMode = "server";
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
}

void TransportLayerAnalyser::start()
{
	if (mMode == "client")
	{
		startClient();
	} 
	else if (mMode == "server")
	{
		startServer();
	}
	else
	{
		QMessageBox::critical(this, "Error", "Please select a operation mode.");
	}
}

void TransportLayerAnalyser::stop()
{

}

void TransportLayerAnalyser::startClient()
{
	FileManager stats("", "stats.txt");

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
	}

	delete mNetworkManager;
	mNetworkManager = new NetworkManager(protocol, dest, port);

	if (ui.radioButton_text->isChecked())
	{
		string msg = ui.plainTextEdit_message->toPlainText().toStdString();
		if (msg.length() > packetSize)
		{
			QMessageBox::critical(this, "Error", "The message must be small enough to fit in the specificed packet size.");
			return;
		}

		mNetworkManager->Connect();
		for (int i = 0; i < packetCount; i++)
		{
			if (mNetworkManager->Send(msg, packetSize) == -1)
			{
				QMessageBox::critical(this, "Error", "An error was encoutered during sending");
				break;
			}
		}
		mNetworkManager->Disconnect();
	}
	else if (ui.radioButton_file->isChecked())
	{
		mFileManager->SetInFile(ui.label_filename->text().toStdString());

		mNetworkManager->Connect();
		for (int i = 0; i < packetCount; i++)
		{
			mNetworkManager->Send(mFileManager->Read(packetSize), packetSize);
		}
		mNetworkManager->Disconnect();
	}
	else
	{
		QMessageBox::critical(this, "Error", "The input methond was not specified or it encountered an error.");
	}
}

void TransportLayerAnalyser::startServer()
{

}