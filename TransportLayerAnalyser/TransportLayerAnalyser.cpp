#include "TransportLayerAnalyser.h"

#include <QDebug>

TransportLayerAnalyser::TransportLayerAnalyser(QWidget *parent)
	: QMainWindow(parent)
	, mFilename("")
	, mOutputFolder("C:/")
{
	ui.setupUi(this);

	connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui.actionSelect_File, &QAction::triggered, this, &TransportLayerAnalyser::selectFile);
	connect(ui.actionSelect_Ouput_Folder, &QAction::triggered, this, &TransportLayerAnalyser::selectOutputFolder);

	connect(ui.actionClient, &QAction::triggered, this, &TransportLayerAnalyser::actionToggle);
	connect(ui.actionServer, &QAction::triggered, this, &TransportLayerAnalyser::actionToggle);

	connect(ui.radioButton_text, &QRadioButton::toggled, ui.plainTextEdit_message, &QPlainTextEdit::setEnabled);
	connect(ui.radioButton_file, &QRadioButton::toggled, ui.label_filename, &QLabel::setEnabled);

	setClientMode();
}

void TransportLayerAnalyser::setClientMode()
{
	ui.actionClient->setChecked(true);
	ui.actionServer->setChecked(false);
	ui.groupBox_packet->setEnabled(true);
	ui.groupBox_data->setEnabled(true);
	setWindowTitle(TITLE + " - Client Mode");
}

void TransportLayerAnalyser::setServerMode()
{
	ui.actionClient->setChecked(false);
	ui.actionServer->setChecked(true);
	ui.groupBox_packet->setEnabled(false);
	ui.groupBox_data->setEnabled(false);
	setWindowTitle(TITLE + " - Server Mode");
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
	mFilename = QFileDialog::getOpenFileName(this, "Select File", "C:/", "Plain Text (*.txt, *.md)");
	ui.label_filename->setText(mFilename);
}

void TransportLayerAnalyser::selectOutputFolder()
{
	mOutputFolder = QFileDialog::getExistingDirectory(this, "Open Folder", "C:/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}