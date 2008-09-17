/* MainWindow.cpp	for freepvQt 08Sep2008
*/

#include <QtGui>

#include "MainWindow.h"
#include "GLwindow.h"
#include "pvQt.h"

MainWindow::MainWindow( QWidget * parent)
{
	errorMsgHandler = QErrorMessage::qtHandler();	

	setupUi( this );

	qWarning("MainWindow c'tor");

bool ok = true;
if(ok) ok =
	connect(action_Quit, SIGNAL(triggered()), 
		    qApp, SLOT( quit()) );
if(ok) ok =
	connect(actionPan_Left, SIGNAL(triggered()), 
		    this, SLOT( panLft()) );
if(ok) ok =
	connect(actionPan_Right, SIGNAL(triggered()),
		    this, SLOT(panRgt()) );
if(ok) ok =
	connect(actionTilt_Up, SIGNAL(triggered()), 
		    this, SLOT(tiltUp()) );
if(ok) ok =
	connect(actionTilt_Down, SIGNAL(triggered()),
		    this, SLOT(tiltDwn()) );
if(ok) ok =
	connect(actionZoom_In, SIGNAL(triggered()), 
		    this, SLOT(zoomIn()) );
if(ok) ok =
	connect(actionZoom_Out, SIGNAL(triggered()),
		    this, SLOT(zoomOut()) );
if(ok) ok =
	connect(actionRoll_Right, SIGNAL(triggered()),
		    this, SLOT(rollRight()) );
if(ok) ok =
	connect(actionRoll_Left, SIGNAL(triggered()),
		    this, SLOT(rollLeft()) );
if(ok) ok =
	connect(action_ResetView, SIGNAL(triggered()),
		    this, SLOT(resetView()) );
if(ok) ok =
	connect(actionLoad_Image, SIGNAL(triggered()),
		    this, SLOT(loadImage()) );

if(ok){
	glwindow = new GLwindow(this);
	ok = glwindow->isOK();
	setCentralWidget( glwindow );
}
#ifdef _DEBUG
ok = false;
#endif

	if( ok ) statusBar()->showMessage(tr("Ready"));
	else  qFatal("MainWindow setup failed");


}

void MainWindow::closeEvent( QCloseEvent * ev ){
}

void MainWindow::showStatus( QString msg ){
	statusBar()->showMessage(msg);
}

void MainWindow::verify( int i ){
	QString s;
	showStatus( s.sprintf("verify: %d", i) );
}

void MainWindow::panLft(){
	emit step_pan(-1);
}
void MainWindow::panRgt(){
	emit step_pan(1);
}

void MainWindow::tiltDwn(){
	emit step_tilt(-1);
}
void MainWindow::tiltUp(){
	emit step_tilt(1);
}

void MainWindow::zoomOut(){
	emit step_zoom(-1);
}
void MainWindow::zoomIn(){
	emit step_zoom(1);
}
void MainWindow::rollLeft(){
	emit step_roll(-1);
}
void MainWindow::rollRight(){
	emit step_roll(1);
}

void MainWindow::resetView(){
	emit set_view(0);
}

void MainWindow::loadImage()
{
	emit newPicture();
}


