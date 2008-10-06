/* MainWindow.h  for freepvQt 09Sep2008

  Handles menu and keyboard input only,
  inner widgets handle graphical input.
  slot showStatus() puts message in status bar.

  NOTE GLwindow is an "insulating layer" for GLview,
  whose only function is to connect signals.  This is
  required by the Qt architecture.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
class GLwindow;
class QActionGroup;
class QErrorMessage;

class MainWindow :
	public QMainWindow,
	public Ui_MainWindow 
{
	Q_OBJECT
public:
	MainWindow(QWidget * parent = 0);
  // send command line to glwindow
	bool postArgs( int argc, char **argv );
public slots:
	void showStatus( QString msg );
signals:
	void step_pan( int d );
	void step_tilt( int d );
	void step_zoom( int d );
	void step_roll( int d );
	void step_dist( int d );
	void home_view();
	void reset_view();
	void super_fish();
	void full_frame();
	
	void newPicture();
protected:
	void closeEvent( QCloseEvent * ev );
	QErrorMessage * errorMsgHandler;
private:
	GLwindow * glwindow;
	QString imgFnm;
	QString projectFnm;
private slots:
	void verify(int i);
	void panLft();
	void panRgt();
	void tiltUp();
	void tiltDwn();
	void zoomIn();
	void zoomOut();
	void rollLeft();
	void rollRight();
	void eyeIn();
	void eyeOut();
	void homeView();
	void resetView();
	void superFish();
	void fullFrame();
	
	void loadImage();
	
};

#endif //ndef MAINWINDOW_H
