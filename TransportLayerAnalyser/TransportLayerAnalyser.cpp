#include "TransportLayerAnalyser.h"

#include <QDebug>

TransportLayerAnalyser::TransportLayerAnalyser(QWidget *parent)
	: QMainWindow(parent)
	, mOutputFileName("C:/")
	, mMode(CLIENT_MODE)
	, mClientAdapter(nullptr)
	, mServerAdapter(nullptr)
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
	stop();
	if (mClientAdapter != nullptr)
	{
		mClientAdapter->terminate();
	}
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
	// Get settings
	int packetSize = ui.lineEdit_packet_size->text().toInt();
	int packetCount = ui.lineEdit_packet_count->text().toInt();
	int port = ui.lineEdit_port->text().toInt();
	int protocol;
	string src = ui.lineEdit_dest->text().toStdString();

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
		QMessageBox::critical(this, "Error", "Protocol selection failed please try again.");
		return;
	}

	// Start thread based on mode
	if (mMode == CLIENT_MODE)
	{
		string msg = ui.plainTextEdit_message->toPlainText().toStdString();
		if (mClientAdapter != nullptr)
		{
			mClientAdapter->terminate();
		}
		mClientAdapter  = new ClientAdapter(src, port, protocol, msg, packetSize, packetCount, this);
		mClientAdapter->start();
	}
	else if (mMode == SERVER_MODE)
	{
		if (mServerAdapter != nullptr)
		{
			mClientAdapter->terminate();
		}
		// change so that it reset winsock stuff instead of making a new one
		mServerAdapter = new ServerAdapter(mOutputFileName.toStdString(), port, protocol, mOutputFileName.toStdString(), this);
		mServerAdapter->start();
	}
	else
	{
		QMessageBox::critical(this, "Error", "Please select a operation mode.");
		return;
	}
}

void TransportLayerAnalyser::stop()
{
	if (mClientAdapter != nullptr)
	{
		mClientAdapter->StopSending();
	}
	if (mServerAdapter != nullptr)
	{
		mServerAdapter->StopListening();
	}
}