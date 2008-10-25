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
	pvpic = new pvQtPic( pvQtPic::cub );

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
  if(!ok) {
	  qFatal("GLwindow setup failed");
  }

}

// pop the About box
void GLwindow::about_pvQt(){
	QString msg = tr("About your OpenGL implentation:\n");
	msg += tr("Version: ") + glview->OpenGLVersion() + QString("\n");
	msg += tr("Vendor: ") + glview->OpenGLVendor() + QString("\n");
	msg += tr("Video: ") + glview->OpenGLHardware() + QString("\n");
	aboutbox.setInfo( msg );
	aboutbox.show();
}

// relay window resize to the GL widget
void GLwindow::resizeEvent( QResizeEvent * ev ){
		glview->resize( size() );
}

// get new  picture of a specified type
void GLwindow::newPicture( const char * type ){
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
	int n = pictypes.picTypeCount( tnm );
	if( n == 0 ) return false;	// no such pic type
	int c = fnm.count();
	bool ok = false, shown = false;
	switch( pictypes.picTypeIndex( tnm ) ){
	case 0:	// proj
	case 4:	// cyli
		qCritical("%s -- to be implemented", tnm );
		return false;
		break;
	case 1:	// qtvr
		if( c == 1 ) shown = ok = QTVR_file( fnm[0] );
		break;
	case 2:	// rect
		ok = pvpic->setType( pvQtPic::rec );
		break;
	case 3:	// fish
		ok = pvpic->setType( pvQtPic::sph );;
		break;
	case 5:	// equi
		ok = pvpic->setType( pvQtPic::eqr );;
		break;
	case 6:	// cube
		ok = pvpic->setType( pvQtPic::cub );
		break;
	}
	if( ok && !shown ){	  // load and show image file types
		if( c > 1 ) fnm.sort();
		if( pvpic->setImageFOV( picFov ) ){
			int nok = 0;
			for(int i = 0; i < c; i++ ){
				if( pvpic->setFaceImage( pvQtPic::PicFace(i), fnm[i] ) ) ++nok;
			}
			ok = shown = glview->showPic( pvpic );
		} else ok = false;
	}
  // put image info in window title
	if( ok ){
		QString msg = "  pvQt  ";
		if( c > 0 ){
			msg += QFileInfo(fnm[0]).fileName();
		} else 	msg += tr("(no image file)");

		msg += QString("  ") + QString(tnm) + QString(" at ");
		QSize pd = pvpic->ImageSize();
		double m = pvpic->NumImages();
		m *= pd.width(); m *= pd.height(); m *= 1.0e-6;
		msg += QString().setNum( m, 'f', 2 ) + QString(" Mpixels");
		emit showTitle( msg );
	} else {
		QString vmsg;
		glview->picOK( vmsg ); 
		QString msg = "  pvQt  ERROR: " + vmsg;
		emit showTitle( msg );
		
	}

	return ok;
}

// Display a QTVR file
bool GLwindow::QTVR_file( QString name ){
	QTVRDecoder dec;
	bool ok = dec.parseHeaders( name.toUtf8().data() );
	if( !ok ){
		qCritical("Not QTVR: %s", name.toUtf8().data() );
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
	
	if( ok ) glview->showPic( pvpic );
	else qCritical("Not panorama: %s", name.toUtf8().data() );		
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
									QSizeF & thefov,
									const char * ptype ){
	thefov = QSizeF(0,0); // failure size
	int nf = files.count();
	if( nf < 1 ) return 0;
// fail if not a readable image file
	QImageReader ir( files[0] );
	if( !ir.canRead() ){
		qCritical("Can't read image: %s", (const char *)files[0].toUtf8());
		return 0;
	}
// get and show image size
	QSize dims = ir.size();
	ptd.setDims( dims );
// show file name
	if( nf == 1 ) ptd.setNameLabel( files[0] );
	else ptd.setNameLabel( files[0] + ",..." );
// get localized picture type descriptions
	QStringList desc = pictypes.picTypeDescrs();
  // remove the project and qtvr picture types
	desc.removeFirst();
	desc.removeFirst();
  // post the rest to the type selector box
	ptd.setPicTypes( desc );
// lock type selection if ptype is valid
  	if( ptype ){
  		int it = pictypes.picTypeIndex( ptype );
  		if( it < 2 ) return 0;
  		ptd.selectPicType( it - 2, true );
 	}
// run the dialog
	if( !ptd.exec() ) return 0;
// return chosen fov and type name
 	thefov = ptd.getFOV();
	return pictypes.picTypeName( ptd.chosenType() + 2 );
}
// Slot: post FOV limits & default = max when pic type changes
void GLwindow::picTypeChanged( int t ){
	ptd.setMinFOV( pictypes.minFov( t + 2 ) );
	ptd.setMaxFOV( pictypes.maxFov( t + 2 ) );
	ptd.setFOV( pictypes.maxFov( t + 2 ) );
}

/* handle command line (before GUI shows)
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
  // see if 1st arg is a picture type name
	int it = pictypes.picTypeIndex( argv[1] );
  // make list of file names
	QStringList sl;	// file name list
	for( int i = (it < 0 ? 1 : 2); i < argc; i++ ){
		sl << QString::fromLocal8Bit( argv[i] );
	}
  // dispatch
  	bool ok;
	if( sl.count() > 0 ){
		if( it >= 0 ) ok = loadTypedFiles( argv[1], sl );
		else ok = loadPictureFiles( sl );
	}
	else ok = choosePictureFiles( argv[1] );
	
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
  // extensions that imply picture type...
	if( ext == "mov" ) return QTVR_file( names[0] );
	if( ext == "pts" || ext == "pto" || ext == "pro"
	  ){
		qCritical("PTscript -- to be implemented");
		return false;  // project_file( name );
	}
  // if more thatn one file, assume cube faces
	if( names.count() > 1 ){
		picFov = QSizeF( 90, 0 );
		return loadTypedFiles( "cube", names );
	}
  // must be an image file, ask for picture type and fov
    return loadTypedFiles( askPicType( names, picFov ), names );
}

/* get a picture with a file selector dialog.

  Optional argument is a picture type name, which 
  sets the dialog's filetype filter.  If no valid
  picture type is given, accept any files and ask
  the user (unless the filename implies pic type).
  
  If the passed type is valid, but allows a variable 
  FOV, we have to ask the user for the FOV anyhow.

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
	if( it == 0 ){
		filter = tr("Stitcher scripts (*.pts, *.pto)");
	} else if( it == 1 ){
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
				picFov = QSizeF(90,0);
				ptnm = "cube";
			} else {
			// no type, ask user for type and FOV
				ptnm = askPicType( files, picFov );
			}
		    it = pictypes.picTypeIndex( ptnm );
	    } else {
		// post default (= max) angular size for this pic type
			picFov = pictypes.maxFov( it );
		// if variable, ask user for FOV only
	  		if( pictypes.minFov( it ) != picFov ){
	  			askPicType( files, picFov, ptnm );  			
  			}
    	}
	} 
  // fail if still no valid type
	if( it < 0 ) return false;
  // else try to show the picture
	return loadTypedFiles( ptnm, files );
}
