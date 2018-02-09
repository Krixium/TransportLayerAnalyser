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
	QString mOutputFileName;

	ClientAdapter * mClientAdapter;
	ServerAdapter * mServerAdapter;

	void setClientMode();
	void setServerMode();

private slots:
	void actionToggle(bool checked);

	void selectFile();
	void selectOutputFolder();

	void start();

public slots:
	void stop();
	void displayError(QString error);

signals:
	void progress(int progress);
};
