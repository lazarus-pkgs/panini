/* MainWindow.h  for freepvQt 09Sep2008
 * Copyright (C) 2008 Thomas K Sharpless
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
class QSettings;
class CubeLimit_dialog;
#include "pvQtMouseModes.h"

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
    void showProj( QString name );
    void showFov( QSizeF fovs );
    void showSurface( int surf );
    void showRecenter( bool );
signals:
    void step_pan( int d );
    void step_tilt( int d );
    void step_zoom( int d );
    void step_roll( int d );
    void step_dist( int d );
    void step_hfov( int d );
    void step_vfov( int d );
    void step_iproj( int d );
    void save_as();
    void home_view();
    void home_eyeXY();
    void reset_view();
    void reset_turn();
    void super_wide();
    void set_view( int v );
    void turn90( int d );
    void set_surface( int surf );
    void step_eyex(int);
    void step_eyey(int);

    void newPicture( const char * pictype );

    void about_pvQt();
    void overlayCtl( int c );
    void recenterMode( bool ckd );

protected:
    virtual void resizeEvent( QResizeEvent * ev );
    virtual void closeEvent( QCloseEvent * ev );
    QErrorMessage * errorMsgHandler;
private:
    GLwindow * glwindow;
    QString imgFnm;
    QString projectFnm;
    pvQtMouseModes * pmm;
  // actions not created with Qt Designer
    QAction * actionToggleSurface;
    QAction * actionNext_iProj;
  // persistent settings
    QSettings * pqs;
  // Mac cube limit dialog
    CubeLimit_dialog * pcld;

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
    void on_actionPanini_proj_triggered();
    void on_actionLinear_proj_triggered();
    void on_actionOrtho_proj_triggered();
    void on_actionSuper_wide_triggered();
    void on_action90_deg_CW_triggered();

    void on_actionQTVR_triggered();
    void on_actionRectilinear_triggered();
    void on_actionFisheye_triggered();
    void on_actionSpherical_triggered();
    void on_actionCylindrical_triggered();
    void on_actionStereographic_triggered();
    void on_actionMercator_triggered();
    void on_actionEquirectangular_triggered();
    void on_actionCube_faces_triggered();
    void on_actionPT_script_triggered();

    void on_actionAbout_pvQt_triggered();
    void on_actionMouse_modes_triggered();

    void on_actionSave_as_triggered();
    void on_actionHFovUp_triggered();
    void on_actionHFovDn_triggered();
    void on_actionVFovUp_triggered();
    void on_actionVFovDn_triggered();
    void on_actionHome_Eye_X_Y_triggered();
    void on_actionNone_wire_model_triggered();
    void on_actionNext_iProj_triggered();
    void on_actionToggleSurface_triggered( bool ckd );
    void on_actionReset_turn_triggered();
    void on_actionCube_limit_triggered();
// overlay menu
    void on_actionShow_Hide_triggered();
    void on_actionLoad_overlay_triggered();
    void on_actionRemove_triggered();
    void on_actionFade_triggered();

    void on_actionRecenter_mode_triggered( bool checked );
    void on_actionEye_right_triggered();
    void on_actionEye_left_triggered();
    void on_actionEye_up_triggered();
    void on_actionEye_down_triggered();
};

#endif //ndef MAINWINDOW_H
