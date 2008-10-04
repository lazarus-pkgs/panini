/* GLwindow.cpp  for freepvQt 09Sep2008 TKS

*/
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

/* Display a (presumed) QTVR file
*/
bool GLwindow::QTVR_file( char * name ){
	QTVRDecoder dec;
	bool ok = dec.parseHeaders( name );
	if( !ok ){
		qCritical("Not QTVR: %s", name );
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
	else qCritical("Not panorama: %s", name );		
	return ok;
}

bool GLwindow::rect_file( char * name ){
	return false;
}

bool GLwindow::fish_file( char * name ){
	return false;
}

bool GLwindow::cyli_file( char * name ){
	return false;
}

bool GLwindow::equi_file( char * name ){
	return false;
}

bool GLwindow::hemi_files( char ** names ){
	return false;
}


/* Display a presumed cubic set of 6 image files
  The file names must be in standard cube face order:
  front, right, back, left, top, bottom
*/
bool GLwindow::cube_files( char ** names ){
	bool ok = true;
	pvpic->setType( pvQtPic::cub );
	for( int i = 0; ok && i < 6; i++ ){
		ok = pvpic->setFaceImage( pvQtPic::PicFace(i), QString(names[i]) );
	}	
	if( ok ) glview->showPic( pvpic );
	else qCritical("Not cubic panorama");		
	return ok;		
}

/* handle command line (before GUI shows)
  if there is one argument try to load it as a QTVR
  if 6, try to load them as a cube.  Otherwise the
  first argument should be a type name, followed by
  the proper number of file names.
*/
bool GLwindow::commandLine( int argc, char ** argv ){
	if( argc == 2 ){  // assume it is a QTVR file name
		return QTVR_file( argv[1] );
	} else if( argc == 7 ){
		return cube_files( argv + 1 );
	} else {
		int n = typed_files( argc - 1, argv + 1 );
		if( n < 0 ) {	// not recognized
			return true;
		} else return n != 0;
	}
  // don't complain if it is something else
	return true;
}

// commandline type names and expected file counts
  static struct typn { char * typ; int nfi; } typn[] = {
    { "PTsc", 1 }, 	// PanoTools-style project file
    { "qtvr", 1 }, 	// pictures...
    { "rect", 1 }, 
    { "fish", 1 }, 
    { "cyli", 1 }, 
    { "equi", 1 }, 
    { "hemi", 2 }, 
    { "cube", 6 }
  };

// check commandline for typename, file... format.
// return -1 if argv[0] is not a type name,
// else 0 if file load failed, 1 if ok 
int GLwindow::typed_files( int argc, char ** argv ){
	int i, n = sizeof(typn) / sizeof(struct typn);
	char * t = argv[0];
	for( i = 0; i < n; i++ ){
		if(!strcmp( t, typn[i].typ )) break;
	}
	if( i == n) return -1;
	n = typn[i].nfi;	// expected files
	if( argc < n + 1 ) {
		qCritical("'%s' needs %d files, %d supplied", t, n, argc - 1);
		return 0;
	}
	bool ok = false;
	switch( i ){
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		qCritical("%s -- to be implemented", t);
		break;
	case 1:
		ok = QTVR_file( argv[1] );
		break;
	case 7:
		ok = cube_files( argv + 1 );
		break;
	}
	return ok ? 1 : 0;
}

// display empty cube
void GLwindow::newPicture(){
	pvpic->setType( pvQtPic::cub );
	glview->showPic( pvpic );
}
