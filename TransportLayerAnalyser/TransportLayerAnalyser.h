#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TransportLayerAnalyser.h"

#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QString>

#include "FileManager.h"
#include "NetworkManager.h"

const QString TITLE = "TransportLayerAnalyser";
const QString RCV_DUMP_FILE = "recv.txt";
const QString STAT_LOG = "stat-log.txt";

class TransportLayerAnalyser : public QMainWindow
{
	Q_OBJECT

public:
	TransportLayerAnalyser(QWidget *parent = Q_NULLPTR);
	~TransportLayerAnalyser();

private:
	Ui::TransportLayerAnalyserClass ui;

	string mMode;
	QString mOutputFileName;

	FileManager * mFileManager;
	NetworkManager * mNetworkManager;

	void setClientMode();
	void setServerMode();

	void startClient();
	void startServer();

private slots:
	void actionToggle(bool checked);

	void selectFile();
	void selectOutputFolder();

	void start();
	void stop();
};
