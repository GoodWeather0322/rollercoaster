#include "AppMain.h"
#include <chrono>
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	srand(time(NULL));
	AppMain *w = AppMain::getInstance();
	w->player = new QMediaPlayer;
	w->player->setMedia(QUrl(QUrl::fromLocalFile("nyan.wav")));
	w->player->setVolume(50);
	w->player->play();

	w->show();
	return a.exec();
}
