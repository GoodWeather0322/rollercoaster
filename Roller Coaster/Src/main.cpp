#include "AppMain.h"
#include <chrono>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	srand(time(NULL));
	AppMain *w = AppMain::getInstance();
	w->show();
	return a.exec();
}
