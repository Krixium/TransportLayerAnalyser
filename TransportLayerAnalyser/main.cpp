#include "TransportLayerAnalyser.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TransportLayerAnalyser w;
	w.show();
	return a.exec();
}
