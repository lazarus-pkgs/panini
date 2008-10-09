/* GLwindow.cpp  for freepvQt 09Sep2008 TKS

*/
#include <QtCore>
#include "GLwindow.h"
#include "picTypeDialog.h"
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
	connect( parent, SIGNAL(super_fish()),
		     glview, SLOT(super_fish()));
		     
  if(ok) ok = 
	connect( glview, SIGNAL(reportView( QString )),
			 parent, SLOT(showStatus( QString )) );
			 
  if(ok) ok = 
	connect( parent, SIGNAL(newPicture( const char * )),	////TEST
			 this, SLOT(newPicture( const char * )) );
  if(!ok) {
	  qFatal("GLwindow setup failed");
  }

}

// relay window resize to the GL widget
void GLwindow::resizeEvent( QResizeEvent * ev ){
		glview->resize( size() );
}

// get new  picture of a specified type
void GLwindow::newPicture( const char * type ){
	choosePictureFiles( type );
}

/* Display a (presumed) QTVR file
*/
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

bool GLwindow::rect_file( QStringList names ){
	return false;
}

bool GLwindow::fish_file( QStringList names ){
	return false;
}

bool GLwindow::cyli_file( QStringList names ){
	return false;
}

/* Display a presumed equirectangular file
  show an empty image if no or too many files given
*/
bool GLwindow::equi_file( QStringList files ){
	bool ok = pvpic->setType( pvQtPic::eqr );
	int c = files.count();
	if(ok && c == 1) ok = pvpic->setFaceImage( pvQtPic::front, files[0] );
	if( ok ) glview->showPic( pvpic );
	return ok;
}

bool GLwindow::hemi_files( QStringList names ){
	return false;
}


/* Display a presumed cubic set of 1 to 6 image files
  The file names must sort in standard cube face order:
  front, right, back, left, top, bottom.
  Shows an empty image if no or too many files given
*/
bool GLwindow::cube_files( QStringList names ){
	bool ok = pvpic->setType( pvQtPic::cub );
	int n = names.count();
	if( n > 0 && n <= 6 ){
		names.sort();
		for( int i = 0; ok && i < n; i++ ){
			ok = pvpic->setFaceImage( pvQtPic::PicFace(i), names[i] );
		}	
	}
	if( ok ) glview->showPic( pvpic );
	else qCritical("Not cubic panorama");		
	return ok;		
}


/** Supported Picture Types
	are designated by ASCII names
**/


/* ask user for the picture type of a set of files
   returns a legal type name or a null pointer.
*/
const char * GLwindow::askPicType( QStringList files ){
	picTypeDialog ptd( this );
	int nf = files.count();
	if( nf < 1 ) return 0;
	if( nf == 1 ) ptd.setNameLabel( files[0] );
	else ptd.setNameLabel( files[0] + ",..." );
	ptd.setPicTypes( pictypes.picTypeDescrs() );

	if( !ptd.exec() ) return 0;
	return pictypes.picTypeName( ptd.chosenType() );
}

/* handle command line (before GUI shows)
*/
bool GLwindow::commandLine( int argc, char ** argv ){
	if( argc < 2 ) return true; // no command
  // see if 1st arg is a picture type name
	int it = pictypes.picTypeIndex( argv[1] );
  // make list of file names
	QStringList sl;	// file name list
	for( int i = (it < 0 ? 1 : 2); i < argc; i++ ){
		sl << QString( argv[i] );
	}
  // dispatch
	if( sl.count() > 0 ){
		if( it >= 0 ) return loadTypedFiles( argv[1], sl );
		else return loadPictureFiles( sl );
	}
	return choosePictureFiles( argv[1] );
}

/*  try to load a picture given type and 1 or more files
   if no files given, show an empty image for type other
   thand QTVR or project
*/
bool GLwindow::loadTypedFiles( const char * tnm, QStringList fnm ){
	int n = pictypes.picTypeCount( tnm );
	if( n == 0 ) return false;	// no such pic type
	int c = fnm.count();
	if( c != n ) {
		qWarning("'%s' wants %d files, %d supplied", tnm, n, c);
	}
	bool ok = false;
	switch( pictypes.picTypeIndex( tnm ) ){
	case 0:
	case 2:
	case 3:
	case 4:
	case 6:
		qCritical("%s -- to be implemented", tnm );
		break;
	case 1:
		if( c > 0 ) ok = QTVR_file( fnm[0] );
		break;
	case 5:
		ok = equi_file( fnm );
		break;
	case 7:
		ok = cube_files( fnm );
		break;
	}
	return ok;
}

/* try to load picture from a set of files.
  Try to guess pic type from file name and size;
  ask user to confirm pic type of plain image file.
  return true it file was loaded, else false.
*/
bool GLwindow::loadPictureFiles( QStringList names ){
	QFileInfo fi( names[0] );
	if( !fi.isReadable() ) return false;
	QString ext = fi.suffix();
	if( ext == "mov" ) return QTVR_file( names[0] );
	if( ext == "pts" || ext == "pto" || ext == "pro"
	  ){
		qCritical("PTscript -- to be implemented");
		return false;  // project_file( name );
	}
  // must be image files, ask for type
    return loadTypedFiles( askPicType( names ), names );
}

/* get a picture with a file selector dialog.

  Optional argument is the expected picture type name,
  which will set the dialog's filetype filter.  The
  default when no picture type is given it to accept
  any files and try to deduce the pic type, possibly
  with the help of another dialog.

  Special case: if user cancels but typename is valid,
  pass an empty image list to loadTypedFiles -- this 
  allows displaying "empty" images

*/
bool GLwindow::choosePictureFiles( const char * ptnm ){
	QFileDialog fd( this );
	fd.setAcceptMode( QFileDialog::AcceptOpen );
	fd.setFileMode( QFileDialog::ExistingFiles );
	fd.setViewMode( QFileDialog::List );
	int it = pictypes.picTypeIndex( ptnm );
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
	if( it < 0 && files.count() > 0 ){
		ptnm = askPicType( files );
	    it = pictypes.picTypeIndex( ptnm );
	}
	if( it < 0 ) return false;
	return loadTypedFiles( ptnm, files );
}
