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
	connect( parent, SIGNAL(newPicture()),	////TEST
			 this, SLOT(newPicture()) );
  if(!ok) {
	  qFatal("GLwindow setup failed");
  }

}

// relay window resize to the GL widget
void GLwindow::resizeEvent( QResizeEvent * ev ){
		glview->resize( size() );
}

// display an empty image
void GLwindow::newPicture(){
	pvpic->setType( pvQtPic::cub );
	glview->showPic( pvpic );
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

bool GLwindow::rect_file( QString name ){
	return false;
}

bool GLwindow::fish_file( QString name ){
	return false;
}

bool GLwindow::cyli_file( QString name ){
	return false;
}

bool GLwindow::equi_file( QString name ){
	bool ok = pvpic->setType( pvQtPic::eqr );
	if(ok) ok = pvpic->setFaceImage( pvQtPic::front, name );
	if( ok ) glview->showPic( pvpic );
	return ok;
}

bool GLwindow::hemi_files( QStringList names ){
	return false;
}


/* Display a presumed cubic set of 6 image files
  The file names must be in standard cube face order:
  front, right, back, left, top, bottom
*/
bool GLwindow::cube_files( QStringList names ){
	bool ok = pvpic->setType( pvQtPic::cub );
	for( int i = 0; ok && i < 6; i++ ){
		ok = pvpic->setFaceImage( pvQtPic::PicFace(i), names[i] );
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
  if the only argument is a picture type name, run
  the file selector dialog for that type; otherwise
  treat it as a file name.
  a filestry to load it as a QTVR
  if 6, try to load them as a cube.  Otherwise the
  first argument should be a type name, followed by
  the proper number of file names.
*/
bool GLwindow::commandLine( int argc, char ** argv ){
	if( argc < 2 ) return choosePictureFiles( 0 );  // no command
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
   if type is not valid, ask user
*/
bool GLwindow::loadTypedFiles( const char * tnm, QStringList fnm ){
	int n = pictypes.picTypeCount( tnm );
	if( n == 0 ) return false;	// no such pic type
	if( fnm.count() < n ) {
		qCritical("'%s' needs %d files, %d supplied", tnm, n, fnm.count());
		return 0;
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
		ok = QTVR_file( fnm[0] );
		break;
	case 5:
		ok = equi_file( fnm[0] );
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
	if( fd.exec() ){
		if( it < 0 ) ptnm = askPicType(fd.selectedFiles());
		return loadTypedFiles( ptnm, fd.selectedFiles() );
	}
	return false;
}
