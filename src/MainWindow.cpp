/* MainWindow.cpp	for pvQt 08Sep2008
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
*/

#include <QtGui>

#include "MainWindow.h"
#include "GLwindow.h"

/* modal error message display
  Note QErrorMessage::qtHandler installs a non-modal handler, so
  you don't get to see the message before the program bombs 
*/
static void errMsgHandler( QtMsgType type, const char * msg ){
	QString s;
	bool die = false;
	switch( type ){
		case QtDebugMsg:
		  s = "Debug: ";
		case QtWarningMsg:
		  s += msg;
		  QMessageBox::warning( 0, "pvQt", s );
		break;
		case QtFatalMsg:
		  die = true;
		  s = "Fatal: ";
		case QtCriticalMsg:
		  s += msg;
		  QMessageBox::critical( 0, "pvQt", s );
		break;
	}
    if( die ) exit( 3 );
}

MainWindow::MainWindow( QWidget * parent)
{
// install modal error message handler
	qInstallMsgHandler( errMsgHandler );
	
	setupUi( this );
	resize( 800, 800 );
	pmm = new pvQtMouseModes( this );

bool ok = true;
if(ok) ok =
	connect(actionQuit, SIGNAL(triggered()), 
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
	connect(actionEye_In, SIGNAL(triggered()),
		    this, SLOT(eyeIn()) );
if(ok) ok =
	connect(actionEye_Out, SIGNAL(triggered()),
		    this, SLOT(eyeOut()) );
if(ok) ok =
	connect(action_Home, SIGNAL(triggered()),
		    this, SLOT(homeView()) );
if(ok) ok =
	connect(actionReset, SIGNAL(triggered()),
		    this, SLOT(resetView()) );

// Add permanent status bar fields
	statusbar->addPermanentWidget( vfovLabel );
	statusbar->addPermanentWidget( hfovLabel );
	statusbar->addPermanentWidget( projLabel );
	statusbar->addPermanentWidget( surfaceLabel );
	surfaceLabel->setText(tr("panosphere"));
	statusBar()->showMessage(tr("Ready"));

if(ok){
	glwindow = new GLwindow(this);
	ok = glwindow->isOK();
	setCentralWidget( glwindow );
}

  if( !ok ) qFatal("MainWindow setup failed");
 
// enable panosurface switch 
  actionPanosphere->setEnabled(true);
  actionPanocylinder->setEnabled(true);

}

/* handle command line argumnents
  called before GUI is activated
  if it returns false the run is aborted.
*/
bool MainWindow::postArgs( int argc, char **argv ){
	return glwindow->commandLine( argc, argv );
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

void MainWindow::showTitle( QString msg ){
	setWindowTitle( msg );
}

/* View menu handlers
*/

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

void MainWindow::eyeIn(){
	emit step_dist(-1);
}

void MainWindow::eyeOut(){
	emit step_dist(1);
}

void MainWindow::homeView(){
	emit home_view();
}
void MainWindow::resetView(){
	emit reset_view();
}

void MainWindow::on_actionFull_frame_triggered(){
	emit full_frame();
}

void MainWindow::on_actionSuper_wide_triggered(){
	emit super_wide();
}

void MainWindow::on_action90_deg_CW_triggered(){
	emit turn90( 1 );
}

void MainWindow::on_action90_deg_CCW_triggered(){
	emit turn90( -1 );
}

/** Picture Menu Handlers 
  named so connectSlotsByName() will find them
**/

void MainWindow::on_actionQTVR_triggered(){
	emit newPicture( "qtvr" );
}

void MainWindow::on_actionRectilinear_triggered(){
	emit newPicture( "rect" );
}

void MainWindow::on_actionFisheye_triggered(){
	emit newPicture( "fish" );
}

void MainWindow::on_actionSpherical_triggered(){
	emit newPicture( "sphr" );
}

void MainWindow::on_actionCylindrical_triggered(){
	emit newPicture( "cyli" );
}

void MainWindow::on_actionStereographic_triggered(){
	emit newPicture( "ster" );
}

void MainWindow::on_actionMercator_triggered(){
	emit newPicture( "merc" );
}

void MainWindow::on_actionEquirectangular_triggered(){
	emit newPicture( "equi" );
}

void MainWindow::on_actionCube_faces_triggered(){
	emit newPicture( "cube" );
}

void MainWindow::on_actionPT_script_triggered(){
	emit newPicture( "proj" );
}

/* Help menu handlers
*/
void MainWindow::on_actionAbout_pvQt_triggered(){
	emit about_pvQt();
}

void MainWindow::on_actionMouse_modes_triggered(){
	pmm->show();
}

/* Display projection and FOV 
*/
void MainWindow::showProj( QString name ){
	projLabel->setText(QString("iproj %1").arg(name));
}

void MainWindow::showFov( QSizeF f ){
	hfovLabel->setText(QString("hfov %1").arg(f.width(), 0, 'f', 1));
	vfovLabel->setText(QString("vfov %1").arg(f.height(), 0, 'f', 1));
}

void MainWindow::on_actionSave_as_triggered(){
	emit save_as();
}

void MainWindow::on_actionNext_iProj_triggered(){
	emit step_iproj( 1 );
}

void MainWindow::on_actionPrev_iProj_triggered(){
	emit step_iproj( -1 );
}

void MainWindow::on_actionHFovUp_triggered(){
	emit step_hfov( 1 );
}

void MainWindow::on_actionHFovDn_triggered(){
	emit step_hfov( -1 );
}

void MainWindow::on_actionVFovUp_triggered(){
	emit step_vfov( 1 );
}

void MainWindow::on_actionVFovDn_triggered(){
	emit step_vfov( -1 );
}

void MainWindow::on_actionPanosphere_triggered( bool ckd ){
	int surf = ckd? 0 : 1;
	showSurface( surf );
	emit set_surface( surf );
}

void MainWindow::on_actionPanocylinder_triggered( bool ckd ){
	on_actionPanosphere_triggered( !ckd );
}

void MainWindow::on_actionHome_Eye_X_Y_triggered(){
	emit home_eyeXY();
}

// Display panosurface.  0: sphere, 1: cylinder
// also sets menu checks
void MainWindow::showSurface( int surf ){
	bool s = surf == 0;
	surfaceLabel->setText( s? tr("panosphere") : tr("panocylinder") );
	actionPanosphere->setChecked( s );
	actionPanocylinder->setChecked( !s );
}
