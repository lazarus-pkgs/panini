/* main.cpp for freepvQt 08Sep2008
*/

#include <QApplication>

#include "MainWindow.h"

int main(int argc, char **argv )
{
	QApplication app(argc, argv);
	MainWindow *window = new MainWindow;
	
	window->show();
	return app.exec();
}
