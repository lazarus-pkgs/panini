/*
 * GLwindow.cpp  for pvQt 09Sep2008 TKS
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

 An invisible "glue" widget to encapsulate a pvQtView widget
 Handles most user interaction other than mouse view control;
 in particular selecting files and setting picture type and fov.

 The policy on fovs is to accept whatever the user enters, even
 if it exceeds the displayable limits or gives non-square pixels
 (however the user has to check a box to enable the latter).
 It is up to pvQtPic to fit the image into a displayable
 face.
*/

#include <QtCore>
#include <QFileDialog>
#include "GLwindow.h"
#include "pvQtView.h"
#include "pvQt_QTVR.h"
#include "MainWindow.h"

GLwindow::GLwindow (QWidget * parent )
    : QWidget(parent)
{
    glview = new pvQtView(this);
    pvpic = new pvQtPic();
    aboutbox = new pvQtAbout( parent );
    ipt = -1;
    for(int i = 0; i < NpictureTypes; i++ ){
        lastTurn[i] = 0;
        lastRoll[i] = lastPitch[i] = lastYaw[i] = 0;
        lastFOV[i] = pictypes.maxFov( i );
    }

    ovlyImg = 0;

    ok = (glview != 0 && pvpic != 0 );

    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_pan, glview, &pvQtView::step_pan);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_tilt, glview, &pvQtView::step_tilt);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_zoom, glview, &pvQtView::step_zoom);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_roll, glview, &pvQtView::step_roll);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_dist, glview, &pvQtView::step_dist);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_hfov, glview, &pvQtView::step_hfov);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_vfov, glview, &pvQtView::step_vfov);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_iproj, glview, &pvQtView::step_iproj);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::home_eyeXY, glview, &pvQtView::home_eyeXY);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::home_view, glview, &pvQtView::home_view);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::reset_view, glview, &pvQtView::reset_view);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::set_view, glview, &pvQtView::set_view);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::super_wide, glview, &pvQtView::super_fish);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::turn90, this, &GLwindow::turn90);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::reset_turn, this, &GLwindow::reset_turn);
    if(ok)
        ok = connect( &turndialog, &TurnDialog::newTurn, glview, &pvQtView::setTurn);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::save_as, this, &GLwindow::save_as);
    if(ok)
        ok = connect( glview, &pvQtView::reportTurn, this, &GLwindow::reportTurn);
    if(ok)
        ok = connect( glview, &pvQtView::reportTurn, &turndialog, &TurnDialog::setTurn);
    if(ok)
        ok = connect( glview, &pvQtView::reportView, (MainWindow*)parent, &MainWindow::showStatus);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::newPicture, this, &GLwindow::newPicture);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::about_pvQt, this, &GLwindow::about_pvQt);
    if(ok)
        ok = connect( this, &GLwindow::showTitle, (MainWindow*)parent, &MainWindow::showTitle);
    if(ok)
        ok = connect( &ptd, &picTypeDialog::picTypeSelected, this, &GLwindow::picTypeChanged);
    if(ok)
        ok = connect( &ptd, &picTypeDialog::hFovChanged, this, &GLwindow::hFovChanged);
    if(ok)
        ok = connect( &ptd, &picTypeDialog::vFovChanged, this, &GLwindow::vFovChanged);
    if(ok)
        ok = connect( glview, &pvQtView::OGLerror, this, &GLwindow::OGLerror);
    if(ok)
        ok = connect( this, &GLwindow::showProj, (MainWindow*)parent, &MainWindow::showProj);
    if(ok)
        ok = connect( this, &GLwindow::showFov, (MainWindow*)parent, &MainWindow::showFov);
    if(ok)
        ok = connect( this, &GLwindow::showSurface, (MainWindow*)parent, &MainWindow::showSurface);
    if(ok)
        ok = connect( glview, &pvQtView::reportSurface, (MainWindow*)parent, &MainWindow::showSurface);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::set_surface, this, &GLwindow::set_surface);
    if(ok)
        ok = connect( glview, &pvQtView::reportProj, this, &GLwindow::showProj);
    if(ok)
        ok = connect( glview, &pvQtView::reportFov, this, &GLwindow::showFov);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::overlayCtl, this, &GLwindow::overlayCtl);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::recenterMode, glview, &pvQtView::recenterMode);
    if(ok)
        ok = connect( glview, &pvQtView::reportRecenter, (MainWindow*)parent, &MainWindow::showRecenter);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_eyex, glview, &pvQtView::step_eyex);
    if(ok)
        ok = connect( (MainWindow*)parent, &MainWindow::step_eyey, glview, &pvQtView::step_eyey);

    if(!ok) {
        qFatal("GLwindow setup failed");
    }
    // enable image file dropping
    setAcceptDrops( true );
}

void GLwindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void GLwindow::dropEvent(QDropEvent *event)
{
    // only accept copy action

    // get the urls
    QList<QUrl> urls(event->mimeData()->urls());
    int n = urls.count();
    // list exisiting local files
    QStringList paths;
    for( int i = 0; i < n; i++ ){
        QString path = urls[i].toLocalFile();
        QFile tf( path );
        if( tf.exists()) paths += path;
    }
    n = paths.count();
    if( n > 0 ) {
        bool ok;
        if( pvpic->Type() == pvQtPic::cub ) {
            if( n > 1 ){
                ok = loadTypedFiles( "cube", paths );
            } else { // add or replace one cube face
                QPoint pnt = event->pos();
                pvQtPic::PicFace pf = glview->pickFace( pnt );
                ok = pvpic->setFaceImage( pf, paths[0] );
                if( ok ){
                    glview->newFace( pf );
                    reportPic( ok, 1, paths );
                }
                else {
                    ok = loadPictureFiles( paths );
                }
            }
        } else {
            ok = loadPictureFiles( paths );
        }

        if( ok ) {
            event->acceptProposedAction();
        }
    }
}

/*
 *  Relay an OpenGL error report to main
 */
void GLwindow::OGLerror( QString errmsg){
    QString msg = "  Panini  " + errmsg;
    emit showTitle( msg );
}

/*
 * Pop the image orientation dialog
  disable Yaw and Pitch for non-cubic images
  restore previous values on cancel
*/
void GLwindow::turn90( int t ){
    int twas;
    double rwas, pwas, ywas;

    turndialog.getTurn( twas, rwas, pwas, ywas );

    bool iscub = pvpic->Type() ==  pvQtPic::cub;

    turndialog.enablePitch( iscub );
    turndialog.enableYaw( iscub );

    if( !turndialog.exec() ){
        turndialog.setTurn( twas, rwas, pwas, ywas );
        glview->setTurn( twas, rwas, pwas, ywas );
    }
}

void GLwindow::reset_turn(){
    glview->setTurn( 0,0,0,0 );
}

void GLwindow::setCubeLimit( int lim ){
    glview->setCubeLimit( lim );
    glview->picChanged();
    reportPic();  // refresh size display
}

/*
 * Record turn angle changes
 */
void GLwindow::reportTurn( int turn, double roll, double pitch, double yaw ){
    if( ipt >= 0 ){
        lastTurn[ipt] = turn;
        lastRoll[ipt] = roll;
        lastPitch[ipt] = pitch;
        lastYaw[ipt] = yaw;
    }
}

/*
 * Pop the About box
 */
void GLwindow::about_pvQt(){
    QString msg =  tr("Built with Qt ") + QString(QT_VERSION_STR) + QString("\n\n");
    msg += tr("System OpenGL:\n\n");
    msg += tr("Version: ") + glview->OpenGLVersion() + QString("\n");
    msg += tr("Vendor: ") + glview->OpenGLVendor() + QString("\n");
    msg += tr("Video: ") + glview->OpenGLHardware() + QString("\n");
    msg += tr("Limits: ") + glview->OpenGLLimits() + QString("\n");
    aboutbox->setInfo( msg );
    aboutbox->show();
}

// relay window resize to the GL widget
void GLwindow::resizeEvent( QResizeEvent * ev ){
    glview->resize( size() );
}

// get new  picture of a specified type
// or show wire frame if type unknown
void GLwindow::newPicture( const char * type ){
    picFov = QSizeF(0,0);
    ipt = pictypes.picTypeIndex( type );
    if( ipt >= 0 ) choosePictureFiles( type );
    else {
        pvpic->setType( pvQtPic::nil );
        glview->showPic( 0 );
    }
}


/**
 * Picture Display Routines
    load a new picture into pvQtPic then pass it to pvQtView
    If the type has a variable fov, send picFov before image(s)
**/

/*
 *   load a picture given type and 0 or more file names
    return false if type is invalid or unsupported, otherwise
    the result of trying to display the files.
    All image file types are loaded here; project and qtvr
    files are handled by subroutines.

    06 Jan 09: select panocylinder for non cubic sources
*/
bool GLwindow::loadTypedFiles( const char * tnm, QStringList fnm ){
    errmsg = tr("(no image file)");
    ipt = pictypes.picTypeIndex( tnm );

    if( ipt < 0 ) {
        return false;	// no such pic type
    }

    picType = pictypes.PicType( ipt );
    int n = pictypes.picTypeCount( ipt );
    int c = fnm.count();
    bool ok = false, loaded = false;

    if( !strcmp( tnm, "proj" )) {
        qCritical("%s -- to be implemented", tnm );
    } else if( !strcmp( tnm, "qtvr" )){
        if( c > 0 ) {
            ok = loaded = QTVR_file( fnm[0] );
            if(!ok) {
                errmsg = tr("QTVR load failed");
            }
        }
    } else {
        ok = pvpic->setType( picType );
    }

    if( ok ) {
        errmsg = "";
        if( !loaded ){	// load image file types
            if( c > 0 ){
                pvpic->setImageFOV( picFov );
                lastFOV[ipt] = picFov;
            }

            if( c > 1 ) {
                fnm.sort();
            }

            for(int i = 0; i < c; i++ ){
                pvpic->setFaceImage( pvQtPic::PicFace(i), fnm[i] );
            }
        }

        // select appropriate default panosurface
        ////	set_surface( picType == pvQtPic::cub ? 0 : 1 );

        // display the image
        glview->showPic( pvpic );
        ok = glview->picOK( errmsg );	// get any OGL error
        glview->setTurn( lastTurn[ipt], lastRoll[ipt], lastPitch[ipt], lastYaw[ipt] );

        // display the file name & size (also post loaddir)
        reportPic( ok, c, fnm );
    }
    return ok;
}

/*
 * put image name & size or error message in window title
   c is the number of files just loaded (0 if no current
   file, -1 to refresh )
   if c > 0 saves the first file name and its directory name;
   replays those if c < 0
*/
void GLwindow::reportPic( bool ok, int c, QStringList fnm ) {
    if( ok ){
        if( c >= 0 ) loadcount = c;
        if( c > 0 ){
            // save the directory name...
            QFileInfo fi( fnm[0] );
            loaddir = fi.absolutePath();
            // display plain file name
            loadname = fi.fileName();
        } else if (c == 0){
            loadname = tr("(no image file)");
        }
        QString msg = "  Panini  ";
        msg += loadname;
        msg += QString("  ") + QString(" at ");
        double m = pvpic->PictureSize();
        msg += QString().setNum( m, 'f', 2 ) + QString(" Mpixels");
        emit showTitle( msg );
    } else {
        QString msg = "  Panini  error: " + errmsg;
        emit showTitle( msg );
    }
}


/*
 *  Load a QTVR file
 */
bool GLwindow::QTVR_file( QString name ){
    QTVRDecoder dec;
    bool ok = dec.parseHeaders( name.toUtf8().data() );

    if( !ok ){
        qCritical("QTVR parse: %s", dec.getError() );
        return false;
    }
    if( dec.getType() == PANO_CUBIC ){
        pvpic->setType( pvQtPic::cub );
        picFov = QSizeF( 90, 90 );
        pvpic->setImageFOV( picFov );
        for( int i = 0; ok && i < 6; i++ ){
            QImage * pim = dec.getImage( i );
            ok = pvpic->setFaceImage( pvQtPic::PicFace(i), pim );
        }
    } else if( dec.getType() == PANO_CYLINDRICAL ){
        pvpic->setType( pvQtPic::cyl );
        QImage * pim = dec.getImage( 0 );

        // compute vFov assuming hFov = 360
        picFov = pvpic->adjustFov(  pvQtPic::cyl, QSizeF( 360 , 0 ), pim->size() );
        pvpic->setImageFOV( picFov );
        ok = pvpic->setFaceImage( pvQtPic::PicFace(0), pim );
    } else {
        ok = false;
    }

    return ok;
}

/*
 * ask user for the picture type and/or angular size
    of one or more image files.

    If ptype points to a valid type name, then that
    type is locked in the selector and only the FOV
    can be changed.

    The range of legal fovs depends on currently selected
    type, image dimensions, and the "unlock aspect ratio"
    checkbox (implemented by xxxChanged slots below).

   returns
    a type name, with a legal angular size in picFov,
    or a null pointer with picFov = (0,0)

    initial type and fov same as last image loaded, if
    valid.
*/
const char * GLwindow::askPicType(  QStringList files,
                                    const char * ptype ){
    picFov = QSizeF(0,0);
    int nf = files.count();

    if( nf < 1 ) {
        return 0;
    }

    // fail if not a readable image file
    QImageReader ir( files[0] );
    if( !ir.canRead() ){
        qCritical("Can't read image: %s", (const char *)files[0].toUtf8());
        return 0;
    }

    // get and show image size
    picDim = ir.size();
    ptd.setDims( picDim );
    // default lock aspect ratio
    ptd.setFreeFovs( false );
    // show file name
    if( nf == 1 ) {
        ptd.setNameLabel( files[0] );
    }
    else {
        ptd.setNameLabel( files[0] + ",..." );
    }

    // get localized picture type descriptions
    QStringList desc = pictypes.picTypeDescrs();

    // remove the project and qtvr picture types
    desc.removeLast();
    desc.removeLast();
    // post the rest to the type selector box
    ptd.setPicTypes( desc );

    // lock type selection if ptype is valid
    if( ipt < 0 ) {
        ipt = 0;
    }

    if( ptype ) {
        ipt = pictypes.picTypeIndex( ptype );

        if( ipt < 0 ) {
            return 0;
        }

        ptd.selectPicType( ipt, true );
    } else {
        ptd.selectPicType( ipt, false );
    }

    // make sure initial fov is set
    picTypeChanged( ipt );

    // run the dialog
    if( !ptd.exec() ){
        picFov = QSizeF(0,0);
        return 0;
    }

    // return chosen fov and type name
    //	picFov = ptd.getFOV();
    //	ipt = ptd.chosenType();
    return pictypes.picTypeName( ipt );
}

/*
 * Slots: update pictype dialog on type or FOV change
  type change: post new fov and limits.
  fov change: rescale the other fov resp. type,
  unless "unlock aspect ratio" is checked.

  If user checks "freeFovs" we accept any input fov up
  to 360 x 360.  Otherwise they are limited to the
  displayable fovs published by pictypes, and the Fovs
  are locked together via the dimensions.
*/
void GLwindow::picTypeChanged( int t ){
    ipt = t;
    picType = pictypes.PicType( ipt );
    picFov = lastFOV[ipt];

    if( !ptd.freeFovs() ){
        picFov = pvpic->adjustFov( picType, picFov, picDim );
    }

    ptd.setMaxFOV( pictypes.absMaxFov( ipt ) );
    ptd.setMinFOV( pictypes.minFov( ipt ) );
    ptd.setFOV( picFov );
}

void GLwindow::hFovChanged( double h ){
    if( fabs(h - picFov.width()) < 0.05 ) {
        return;
    }

    picFov = ptd.getFOV();
    if( ptd.freeFovs() ) {
        picFov.setWidth( h );
    }
    else {
        picFov = pvpic->changeFovAxis( picType, picFov, h, 0 );
    }
    ptd.setFOV( picFov );
}

void GLwindow::vFovChanged( double v ){
    if( fabs(v - picFov.height()) < 0.05 ) {
        return;
    }

    picFov = ptd.getFOV();
    if( ptd.freeFovs() ) {
        picFov.setHeight( v );
    }
    else {
        picFov = pvpic->changeFovAxis( picType, picFov, v, 1 );
    }

    ptd.setFOV( picFov );
}


/*
 * handle command line
  Always returns true, so program will continue
  even if the command line fails.

  Note commandline args are strings of 8 bit chars, in one of
   many possible encodings. The recommended way to convert them
   to Unicode is QString::fromLocal8Bit(); it is reported that
   this doesn't always work, but it will have to do.
   The picture type name is supposed to be UTF8 or ASCII, but
   there may be systems that won't let you enter them as such.
   If so we might have to use localized type names in a fully
   internationalized pvQt.
*/
bool GLwindow::commandLine( int argc, char ** argv ){
    if( !glview->OpenGLOK() ){
        qCritical("Your video driver does not support Panini");
        // qCritical("OpenGL version: %s", (const char *)glGetString(GL_VERSION));
        return false;
    }

    if( argc < 2 ) {
        // no command
        return true;
    }

    // empty file name list
    QStringList sl;
    // null fov
    picFov = QSizeF(0,0);

    // see if 1st arg is a picture type name
    int it = pictypes.picTypeIndex( argv[1] );
    int ifile = (it < 0 ? 1 : 2 );
    // if picture type, load default fov
    if( it >= 0 ) {
        picFov = pictypes.maxFov( it );
    }

    // if it is a plain image type, see if 2nd is FOV
    // ( if so, put full fov in picFov)

    if( it >= 0  && it < Nprojections ) {
        // display empty frame if there is no 2nd arg
        if( argc < 3 ) {
            return loadTypedFiles( argv[1], sl );
        }

        // is it an fov?
        double f = atof( argv[2] );
        if( f > 5 && f <= 360 ) {
            // yes
            ++ifile;
            // scale the default fov
            pvQtPic::PicType pt = pictypes.PicType( it );
            picFov = pvpic->changeFovAxis( pt, picFov, f );
        }
    }

    // list the file names (convert to Unicode)
    for( int i = ifile; i < argc; i++ ){
        sl << QString::fromLocal8Bit( argv[i] );
    }

    // dispatch
    bool ok;
    if( sl.count() > 0 ){
        if( it >= 0 ) {
            ok = loadTypedFiles( argv[1], sl );
        } else {
            ok = loadPictureFiles( sl );
        }
    }
    else if( it >= 0 ) {
        ok = choosePictureFiles( argv[1] );
    }

    return true;
}

/*
 * try to load picture from a set of files.
  Try to guess pic type from file name and size;
  ask user to confirm pic type of plain image file.
  return true if a pic was loaded, else false.
*/
bool GLwindow::loadPictureFiles( QStringList names ){
    int n = names.count();

    if( n < 1 ) {
        return false;
    }

    QFileInfo fi( names[0] );
    QString ext = fi.suffix();

    // extensions that imply picture QTVR_filetype...
    if( ext == "mov" ) {
        return loadTypedFiles( "qtvr", names );
    }

    if( ext == "pts" || ext == "pto" || ext == "pro" ){
        qCritical("PTscript -- to be implemented");
        return false;  // project_file( name );
    }

    // if more than one file, assume cube faces
    if( names.count() > 1 ){
        picFov = QSizeF( 90, 90 );
        return loadTypedFiles( "cube", names );
    }

    // fail if not a readable image file
    QImageReader ir( names[0] );
    if( !ir.canRead() ){
        qCritical("Can't read image: %s", (const char *)names[0].toUtf8());
        return 0;
    }

    // if 2:1 aspect ratio assume eqr
    picDim = ir.size();
    if( picDim.width() == 2 * picDim.height() ){
        ipt = pictypes.picTypeIndex("equi");
        picType = pictypes.PicType( ipt );
        picFov = pictypes.maxFov( ipt );
        return loadTypedFiles( "equi", names );
    }

    // ask for picture type and fov
    return loadTypedFiles( askPicType( names ), names );
}

/*
 * get a picture with a file selector dialog.
  Optional argument is a picture type name, which
  sets the dialog's filetype filter.  If no valid
  picture type is given, accept any files and ask
  the user (unless the filename implies pic type).

  If the passed type is valid, but allows a variable
  FOV, and no valid picFov has ben posted, we have to
  ask the user for the FOV anyhow.

  Special case: if user cancels and typename is valid,
  pass an empty image list to loadTypedFiles -- this
  allows displaying "empty" images
*/
bool GLwindow::choosePictureFiles( const char * ptnm ){
    QStringList files;
    // get pic type index (-1 if invalid)
    int it = pictypes.picTypeIndex( ptnm );
    // dialog title
    QString title = tr(" Panini -- Select Source Image File(s)");

    if( it >= 0 ) {
        title += QString(": ") + QString( ptnm );
    }

    // contruct file extension filter
    QString filter(tr("All files (*.*)") );
    if( !strcmp( ptnm, "proj" ) ){
        filter = tr("Stitcher scripts (*.pts, *.pto)");
    } else if( !strcmp( ptnm, "qtvr" ) ){
        filter = tr("Quicktime panoramas (*.mov)");
    } else {
        filter = tr("Image files") + " (";
        QList<QByteArray> fmts(QImageReader::supportedImageFormats());
        foreach( QByteArray fmt, fmts ){
            filter += " *." + fmt;
        }
        filter += ")";
    }

    // use native Win or Mac file selectors, Qfd on Linux...
    files = QFileDialog::getOpenFileNames( this, title, loaddir, filter );

    // if there are files
    if( files.count() > 0 ){
        if( it < 0 ){
            // assume cubic if more than one file
            if( files.count() > 1 ){
                picFov = QSizeF(90,90);
                ptnm = "cube";
            } else {
                // no type, ask user for type and FOV
                ptnm = askPicType( files );
            }
            it = pictypes.picTypeIndex( ptnm );
        } else {
            // post default (= max) angular size for this pic type
            picFov = pictypes.maxFov( it );
            // if variable, ask user for FOV only
            if( pictypes.minFov( it ) != picFov ){
                ptnm = askPicType( files, ptnm );
            }
        }
    } else if( it >= 0 ) {
        picFov = pictypes.maxFov( it );
    }

    // try to show the picture
    return loadTypedFiles( ptnm, files );
}

/*
 * save current view to an image file
 */
void GLwindow::save_as() {
    double fac = 2.5;		// linear scale factor for size
    // dialog title
    QString title = tr(" Panini -- Save View As");
    // file extension filter
    QString filter(tr("Jpeg files (*.jpg *.jpeg)") );

    // default directory
    if( savedir.isEmpty() ) {
        savedir = loaddir;
    }

    // get file name
    QString fnm = QFileDialog::getSaveFileName( this, title, savedir, filter );

    if( !fnm.isEmpty() ){
        QFileInfo fi( fnm );
        // save directory...
        savedir = QFileInfo( fnm ).absolutePath();
        // add .jpg suffix if seems missing (Q&D)
        if( fi.suffix().length() != 3  ) {
            fnm += ".jpg";
        }
        // save file
        if( !glview->saveView( fnm, fac )){
            qCritical("saveView() failed");
        }
    }
}

/*
 * handle set surface requests
 */
void GLwindow::set_surface( int surf ){
    bool ok = pvpic->setSurface( surf );  // check
    glview->setSurface( pvpic->Surface());
    emit showSurface( pvpic->Surface());
}

/*
 * Overlay image control
*/
void GLwindow::overlayCtl( int c ){
    const int nfades = 6;
    static double alphas[nfades] = {
        1, 0.7, 0.5, 0.3, 1, 0.7
    };
    static int deltas[nfades] = {
        0, 0, 0, 0, 30, 30
    };

    switch( c ){
    default:
    case 0:
        glview->showOverlay( 0 );
        if( ovlyImg ) {
            delete ovlyImg;
        }
        ovlyImg = 0;
        ovlyVisible = false;
        break;
    //load image
    case 1:
        // delete any existing overlay
        if( ovlyImg ) {
            delete ovlyImg;
        }
        ovlyImg = 0;

        // maybe load a new one
        ovlyVisible = loadOverlayImage();
        ovlyFade = 2;
        setImgAlpha( ovlyImg, 1 - 0.25 * ovlyFade );
        glview->showOverlay( ovlyImg );
        break;
    // show/hide
    case 2:
        ovlyVisible = !ovlyVisible;
        glview->showOverlay( ovlyVisible ? ovlyImg : 0 );
        break;
    //fade
    case 3:
        if(++ovlyFade >=nfades) {
            ovlyFade = 0;
        }
        diceImgAlpha( ovlyImg, alphas[ovlyFade], deltas[ovlyFade] );
        glview->showOverlay( ovlyVisible ? ovlyImg : 0 );
        break;
    }
}

bool GLwindow::loadOverlayImage()
{
    // file type filter
    QString filter = tr("Image files") + " (";
    QList<QByteArray> fmts(QImageReader::supportedImageFormats());

    foreach( QByteArray fmt, fmts ){
        filter += " *." + fmt;
    }
    filter += ")";

    // file selector dialog
    QString fnm = QFileDialog::getOpenFileName( this, tr("Panini - Overlay Image"), loaddir, filter );

    /* if name is valid, load and prepare image
    convert format to ARGB if necessary, and scale
    height to a fraction of max screen height.
    */
    if( fnm.isEmpty() ) {
        return false;
    }

    QImage * pt, * ps;
    QImageReader ird( fnm );

    pt = new QImage( ird.read() );
    if( !pt->isNull() ){
        ps = new QImage( pt->scaledToHeight(640) );
        delete pt;
        if( ps->format() != QImage::Format_ARGB32 ){
            pt = new QImage( ps->convertToFormat(QImage::Format_ARGB32));
            delete ps;
            ps = pt;
        }
        ovlyImg = ps;
    }

    return true;
}

void GLwindow::setImgAlpha( QImage * pim, double alpha ){
    if( pim == 0 || pim->isNull() || pim->format() != QImage::Format_ARGB32 ) {
        return;
    }

    qint32 * pw = (qint32 *) pim->bits();
    qint32 m = (int( 255 * alpha ) & 255 ) << 24;

    for( int i = pim->width() * pim->height(); i > 0; i-- ){
        qint32 t = *pw & 0x00ffffff;
        *pw++ = t + m;
    }
}

void GLwindow::diceImgAlpha( QImage * pim, double alpha, int dw ){
    if( pim == 0 || pim->isNull() || pim->format() != QImage::Format_ARGB32 ) {
        return;
    }

    qint32 * pw = (qint32 *) pim->bits();
    qint32 m = (int( 255 * alpha ) & 255 ) << 24;
    int r = 0, c = 0, w = pim->width();

    for( int i = w * pim->height(); i > 0; i-- ){
        qint32 t = *pw & 0x00ffffff;
        int d = 1;
        if( dw ){  // dice...
            d = ((r + c) / dw) ^ ((r + w - c) / dw);
        }
        if( d & 1 ) t += m;
        *pw++ = t;
        if(++c >= w ){
            ++r;
            c = 0;
        }
    }
}
