/* GLwindow.cpp  for pvQt 09Sep2008 TKS
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

 An invisible "glue" widget to encapsulate a pvQtView widget

*/
#include <QtCore>
#include "GLwindow.h"
#include "pvQtView.h"
#include "pvQt_QTVR.h"

GLwindow::GLwindow (QWidget * parent )
: QWidget(parent)
{
	glview = new pvQtView(this);
	pvpic = new pvQtPic( );

	for(int i = 0; i < NpictureTypes; i++ ){
		lastTurn[i] = 0;
		lastFOV[i] = pictypes.maxFov( i );
	}

  ok = (glview != 0 && pvpic != 0 );

  if(ok) ok = 
	connect( parent, SIGNAL(step_pan( int )),
		     glview, SLOT(step_pan( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(step_tilt( int )),
		     glview, SLOT(step_tilt( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(step_zoom( int )),
		     glview, SLOT(step_zoom( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(step_roll( int )),
		     glview, SLOT(step_roll( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(step_dist( int )),
		     glview, SLOT(step_dist( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(step_hfov( int )),
		     glview, SLOT(step_hfov( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(step_vfov( int )),
		     glview, SLOT(step_vfov( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(step_iproj( int )),
		     glview, SLOT(step_iproj( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(home_view()),
		     glview, SLOT(home_view()));
  if(ok) ok = 
	connect( parent, SIGNAL(reset_view()),
		     glview, SLOT(reset_view()));
  if(ok) ok = 
	connect( parent, SIGNAL(full_frame()),
		     glview, SLOT(full_frame()));
  if(ok) ok = 
	connect( parent, SIGNAL(super_wide()),
		     glview, SLOT(super_fish()));
  if(ok) ok = 
	connect( parent, SIGNAL(turn90( int )),
		     glview, SLOT(turn90( int )));
  if(ok) ok = 
	connect( parent, SIGNAL(save_as()),
		     this, SLOT(save_as()));
  if(ok) ok = 
	connect( glview, SIGNAL(reportTurn(double)),
		     this, SLOT(reportTurn(double)));
  if(ok) ok = 
	connect( glview, SIGNAL(reportView( QString )),
			 parent, SLOT(showStatus( QString )) );
  if(ok) ok = 
	connect( parent, SIGNAL(newPicture( const char * )),
			 this, SLOT(newPicture( const char * )) );
  if(ok) ok = 
	connect( parent, SIGNAL(about_pvQt()),
			 this, SLOT(about_pvQt()) );
  if(ok) ok = 
	connect( this, SIGNAL(showTitle(QString)),
			 parent, SLOT(showTitle(QString)) );
  if(ok) ok = 
	connect( &ptd, SIGNAL(picTypeSelected( int )),
			 this, SLOT(picTypeChanged( int )) );
  if(ok) ok = 
	connect( &ptd, SIGNAL(hFovChanged( double )),
			 this, SLOT(hFovChanged( double )) );
  if(ok) ok = 
	connect( &ptd, SIGNAL(vFovChanged( double )),
			 this, SLOT(vFovChanged( double )) );
  if(ok) ok = 
	connect( glview, SIGNAL(OGLerror( QString )),
			 this, SLOT(OGLerror( QString )) );
  if(ok) ok = 
	connect( this, SIGNAL(showProj( QString )),
			 parent, SLOT(showProj( QString )) );
  if(ok) ok = 
	connect( this, SIGNAL(showFov(QSizeF)),
			 parent, SLOT(showFov(QSizeF)) );
  if(ok) ok = 
	connect( glview, SIGNAL(reportProj( QString )),
			 this, SIGNAL(showProj( QString )) );
  if(ok) ok = 
	connect( glview, SIGNAL(reportFov(QSizeF)),
			 this, SIGNAL(showFov(QSizeF)) );
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
	if( n > 0 ){
		bool ok;
		if( pvpic->Type() == pvQtPic::cub ){
			if( n > 1 ){
				ok = loadTypedFiles( "cube", paths );
			} else { // add or replace one cube face
				QPoint pnt = event->pos();
				pvQtPic::PicFace pf = glview->pickFace( pnt );
				pvpic->setImageFOV(QSizeF( 90, 90 ));
				ok = pvpic->setFaceImage( pf, paths[0] );
				if( ok ){
					if( pvpic->NumImages() == 1 ) glview->showPic( pvpic );
					else glview->newFace( pf );
				}
				else ok = loadPictureFiles( paths );
			}
		} else {
			ok = loadPictureFiles( paths );
		}
		if( ok ) event->acceptProposedAction();
	}
}

// Relay an OpenGL error report to main
void GLwindow::OGLerror( QString errmsg){
		QString msg = "  pvQt  " + errmsg;
		emit showTitle( msg );
}

// Record turn angle changes
void GLwindow::reportTurn( double deg ){
	if( ipt >= 0 ) lastTurn[ipt] = deg;
}


// pop the About box
void GLwindow::about_pvQt(){
	QString msg = tr("About your OpenGL implementation:\n");
	msg += tr("Version: ") + glview->OpenGLVersion() + QString("\n");
	msg += tr("Vendor: ") + glview->OpenGLVendor() + QString("\n");
	msg += tr("Video: ") + glview->OpenGLHardware() + QString("\n");
	msg += tr("Limits: ") + glview->OpenGLLimits();
	aboutbox.setInfo( msg );
	aboutbox.show();
}

// relay window resize to the GL widget
void GLwindow::resizeEvent( QResizeEvent * ev ){
		glview->resize( size() );
}

// get new  picture of a specified type
void GLwindow::newPicture( const char * type ){
	picFov = QSizeF(0,0);
	choosePictureFiles( type );
}


/**  Picture Display Routines
	load a new picture into pvQtPic then pass it to pvQtView
	If the type has a variable fov, send picFov before image(s)
**/

/*  load a picture given type and 0 or more file names
	return false if type is invalid or unsupported, otherwise 
	the result of trying to display the files.
	All image file types are loaded here; project and qtvr
	files are handled by subroutines.
*/
bool GLwindow::loadTypedFiles( const char * tnm, QStringList fnm ){
	QString errmsg("(no image file)");
	ipt = pictypes.picTypeIndex( tnm );
	if( ipt < 0 ) return false;	// no such pic type
	picType = pictypes.PicType( ipt );
	int n = pictypes.picTypeCount( ipt );
	int c = fnm.count();
	bool ok = false, loaded = false;

	if( !strcmp( tnm, "proj" )){
		qCritical("%s -- to be implemented", tnm );
	} else if( !strcmp( tnm, "qtvr" )){
		if( c > 0 ){
			ok = loaded = QTVR_file( fnm[0] );
			if(!ok) errmsg = tr("QTVR load failed");
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
			if( c > 1 ) fnm.sort();
			for(int i = 0; i < c; i++ ){
				pvpic->setFaceImage( pvQtPic::PicFace(i), fnm[i] );
			}
		}
	  // display the image
		glview->showPic( pvpic );
		ok = glview->picOK( errmsg );	// get any OGL error
		glview->turnAbs( lastTurn[ipt] );
	}
  // put image name & size or error message in window title
	if( ok ){
		QString msg = "  pvQt  ";
		if( c > 0 ){
			msg += QFileInfo(fnm[0]).fileName();
		} else 	msg += tr("(no image file)");
		msg += QString("  ") + QString(" at ");
		QSize pd = pvpic->PictureSize();
		double m = pvpic->NumImages();
		m *= pd.width(); m *= pd.height(); m *= 1.0e-6;
		msg += QString().setNum( m, 'f', 2 ) + QString(" Mpixels");
		emit showTitle( msg );
	} else {
		QString msg = "  pvQt  error: " + errmsg;
		emit showTitle( msg );
	}

	return ok;
}

// Load a QTVR file
bool GLwindow::QTVR_file( QString name ){
	QTVRDecoder dec;
	bool ok = dec.parseHeaders( name.toUtf8().data() );
	if( !ok ){
		qCritical("QTVR parse: %s", dec.getError() );
		return false;
	}
	if( dec.getType() == PANO_CUBIC ){
		pvpic->setType( pvQtPic::cub );
		for( int i = 0; ok && i < 6; i++ ){
			QImage * pim = dec.getImage( i );
			ok = pvpic->setFaceImage( pvQtPic::PicFace(i), pim );
		}	
	} else if( dec.getType() == PANO_CYLINDRICAL ){
		pvpic->setType( pvQtPic::cyl );
		QImage * pim = dec.getImage( 0 );
		ok = pvpic->setFaceImage( pvQtPic::PicFace(0), pim );
	} else ok = false; 

	return ok;
}

/* ask user for the picture type and/or angular size
 	of one or more image files. 
 	 
	The choices for type don't include project or
	qtvr, and the range of allowed fovs depends 
	on currently selected type (implemented by slot 
	picTypeChanged ).
	
	Note the dialog sets the larger of the 2 axial fovs. 
	The other fov is computed from the image dimensions
	and picture type.
	
	If ptype points to a valid type name, then that
	type is locked in the selector and only the FOV
	can be changed.
   
   returns
	a type name and a legal angular size,
	or a null pointer with size = 0,0
*/
const char * GLwindow::askPicType(  QStringList files, 
									const char * ptype ){
	int nf = files.count();
	if( nf < 1 ) return 0;
// fail if not a readable image file
	QImageReader ir( files[0] );
	if( !ir.canRead() ){
		qCritical("Can't read image: %s", (const char *)files[0].toUtf8());
		return 0;
	}
// get and show image size
	picDim = ir.size();
	ptd.setDims( picDim );
// show file name
	if( nf == 1 ) ptd.setNameLabel( files[0] );
	else ptd.setNameLabel( files[0] + ",..." );
// get localized picture type descriptions
	QStringList desc = pictypes.picTypeDescrs();
  // remove the project and qtvr picture types
	desc.removeLast();
	desc.removeLast();
  // post the rest to the type selector box
	ptd.setPicTypes( desc );
// lock type selection if ptype is valid
  	if( ptype ){
  		ipt = pictypes.picTypeIndex( ptype );
  		if( ipt < 0 ) return 0;
  		ptd.selectPicType( ipt, true );
	} else {
		ipt = 0;
		ptd.selectPicType( 0, false );
	}
	picTypeChanged( ipt );
// run the dialog
	if( !ptd.exec() ) return 0;
// return chosen fov and type name
 	picFov = ptd.getFOV();
	ipt = ptd.chosenType();
	return pictypes.picTypeName( ipt );
}
/* Slots: update pictype dialog on type or FOV change
  type change: post new fov and limits.  
  fov change: rescale the other fov resp. type
*/
void GLwindow::picTypeChanged( int t ){
	ipt = t;
	picType = pictypes.PicType( ipt );
	picFov = pvpic->adjustFov( picType, lastFOV[ipt], picDim );
	ptd.setMaxFOV( pictypes.maxFov( ipt ) );
	ptd.setMinFOV( pictypes.minFov( ipt ) );
	ptd.setFOV( picFov );
}

void GLwindow::hFovChanged( double h ){
	if( fabs(h - picFov.width()) < 0.05 ) return;
//	picFov = ptd.getFOV();
	if( ipt < 0 ) picFov.setWidth( h );
	else {
		picFov = pvpic->changeFovAxis( picType, picFov, h, 0 );
	}
	ptd.setFOV( picFov );
}

void GLwindow::vFovChanged( double v ){
	if( fabs(v - picFov.height()) < 0.05 ) return;
//	picFov = ptd.getFOV();
	if( ipt < 0 ) picFov.setHeight( v );
	else {
		picFov = pvpic->changeFovAxis( picType, picFov, v, 1 );
	}
	ptd.setFOV( picFov );
}


/* handle command line
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
//		qCritical("Your video card does not support pvQt");
		qCritical("OpenGL version: %s", (const char *)glGetString(GL_VERSION));
	}

	if( argc < 2 ) return true; // no command

	QStringList sl;			// empty file name list
	picFov = QSizeF(0,0);	// null fov

  // see if 1st arg is a picture type name
	int it = pictypes.picTypeIndex( argv[1] );
	int ifile = (it < 0 ? 1 : 2 );
  // if picture type, load default fov
	if( it >= 0 ) picFov = pictypes.maxFov( it );
  // if it is a plain image type, see if 2nd is FOV
  // ( if so, put full fov in picFov)
	if( it >= 0  && it < Nprojections ){
	  // display empty frame if there is no 2nd arg
		if( argc < 3 ) return loadTypedFiles( argv[1], sl );
	  // is it an fov?
		double f = atof( argv[2] );
		if( f > 5 && f <= 360 ){
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
		if( it >= 0 ) ok = loadTypedFiles( argv[1], sl );
		else ok = loadPictureFiles( sl );
	}
	else if( it >= 0 ) ok = choosePictureFiles( argv[1] );
	
	return true;
}

/* try to load picture from a set of files.
  Try to guess pic type from file name and size;
  ask user to confirm pic type of plain image file.
  return true if a pic was loaded, else false.
*/
bool GLwindow::loadPictureFiles( QStringList names ){
	int n = names.count();
	if( n < 1 ) return false;
	QFileInfo fi( names[0] );
	QString ext = fi.suffix();
  // extensions that imply picture QTVR_filetype...
	if( ext == "mov" ) return loadTypedFiles( "qtvr", names );
	if( ext == "pts" || ext == "pto" || ext == "pro"
	  ){
		qCritical("PTscript -- to be implemented");
		return false;  // project_file( name );
	}
  // if more thatn one file, assume cube faces
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
  // If 2:1 aspect ratio assume eqr
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

/* get a picture with a file selector dialog.

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
	QFileDialog fd( this );
	fd.setAcceptMode( QFileDialog::AcceptOpen );
	fd.setFileMode( QFileDialog::ExistingFiles );
	fd.setViewMode( QFileDialog::List );
  // get pic type index (-1 if invalid)
	int it = pictypes.picTypeIndex( ptnm );
  // set dialog title
	QString title = tr("pvQt -- Select Image Files");
	if( it >= 0 ) title += QString(": ") + QString( ptnm );
	fd.setWindowTitle( title );
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
	fd.setNameFilter( filter );
	QStringList files;
	if( fd.exec()) files = fd.selectedFiles();
  // if there are files...
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
	} else if( it >= 0 ) picFov = pictypes.maxFov( it );
  // try to show the picture
	return loadTypedFiles( ptnm, files );
}

// save current view to an image file
void GLwindow::save_as() {
	QSize scrn = glview->screenSize();	// size as displayed
	double fac = 2.5;		// linear scale factor for size
	QString fnm = "testSaveView.jpg";

	QFileDialog fd( this );
	fd.setAcceptMode( QFileDialog::AcceptSave );
	fd.setFileMode( QFileDialog::AnyFile );
	fd.setViewMode( QFileDialog::List );
  // set dialog title
	QString title = tr("pvQt -- Save View As");
	fd.setWindowTitle( title );
  // contruct file extension filter
	QString filter(tr("Jpeg files (*.jpg)") );
	fd.setNameFilter( filter );
  // default suffix is "jpg"
	fd.setDefaultSuffix( QString("jpg"));
	QStringList files;
	if( fd.exec()){
		files = fd.selectedFiles();
		if( !glview->saveView( files[0], scrn * fac )){
			qCritical("saveView() failed");
		}
	}
}

