#include "AppMain.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	AppMain *w = AppMain::getInstance();
	w->show();
	return a.exec();
}
