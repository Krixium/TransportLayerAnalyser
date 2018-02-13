#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TransportLayerAnalyser.h"

#include <ctime>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QString>

#include "global.h"
#include "ClientAdapter.h"
#include "ServerAdapter.h"

const QString TITLE = "TransportLayerAnalyser";

const QString LABEL_TIME = "Time: ";
const QString LABEL_XFER = "Data Transfered: ";

class TransportLayerAnalyser : public QMainWindow
{
	Q_OBJECT

public:
	TransportLayerAnalyser(QWidget *parent = Q_NULLPTR);
	~TransportLayerAnalyser();

private:
	Ui::TransportLayerAnalyserClass ui;

	int mMode;
	int mBytesSent;
	QString mOutputFileName;
	clock_t mStartTime;

	ClientAdapter * mClientAdapter;
	ServerAdapter * mServerAdapter;

	void setClientMode();
	void setServerMode();
	void toggleStartButton();

private slots:
	void actionToggle(bool checked);

	void selectFile();
	void selectOutputFolder();

	void start();
	void stop();
	void updateBytesLabel(int bytes);
	void displayError(QString error);

	void startLogging();
	void stopLogging();

signals:
	void progress(int progress);
};
