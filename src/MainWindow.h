/* MainWindow.h  for freepvQt 09Sep2008
 * Copyright (C) 2008 Thomas K Sharpless
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *

  Handles menu and keyboard input only,
  inner widgets handle graphical input.
  slot showStatus() puts message in status bar.

  NOTE GLwindow is an "insulating layer" for GLview,
  whose main function is to relay signals.  This is
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
	void showTitle( QString msg );
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
	
	void newPicture( const char * pictype );

	void about_pvQt();

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
	
	void on_actionQTVR_triggered();
	void on_actionRectilinear_triggered();
	void on_actionFisheye_triggered();
	void on_actionCylindrical_triggered();
	void on_actionEquirectangular_triggered();
	void on_actionCube_faces_triggered();
	void on_actionPT_script_triggered();

	void on_actionAbout_pvQt_triggered();	
};

#endif //ndef MAINWINDOW_H
