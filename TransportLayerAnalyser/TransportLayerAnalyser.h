#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TransportLayerAnalyser.h"

#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QString>

const QString TITLE = "TransportLayerAnalyser";
const QString RCV_DUMP_FILE = "recv.txt";
const QString STAT_LOG = "stat-log.txt";

class TransportLayerAnalyser : public QMainWindow
{
	Q_OBJECT

public:
	TransportLayerAnalyser(QWidget *parent = Q_NULLPTR);

private:
	Ui::TransportLayerAnalyserClass ui;

	QString mFilename;
	QString mOutputFolder;

	void setClientMode();
	void setServerMode();

private slots:
	void actionToggle(bool checked);

	void selectFile();
	void selectOutputFolder();
};
