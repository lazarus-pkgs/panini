/* main.cpp for freepvQt 08Sep2008
*/

#include <QApplication>

#include "MainWindow.h"

int main(int argc, char **argv )
{
	QApplication app(argc, argv);
	MainWindow *window = new MainWindow;
  // process commandline, abort if fails
	if( !window->postArgs(argc, argv) ) return 3;
  // ok, run the GUI...
	window->show();
	return app.exec();
}
