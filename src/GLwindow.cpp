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
	} 
	if( ok ) glview->showPic( pvpic );
	else qCritical("Not cubic panorama");		
	return ok;
}

/* Display a presumed cubic set of 6 image files
  The file names must be in standard cube face order:
  front, right, back, left, top, bottom
*/
bool GLwindow::CUBE_files( char ** names ){
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
  if 6, try to load them as a cube
*/
bool GLwindow::commandLine( int argc, char ** argv ){
	if( argc == 2 ){  // assume it is a QTVR file name
		return QTVR_file( argv[1] );
	} else if( argc == 7 ){
		return CUBE_files( argv + 1 );
	}
  // don't complain if it is something else
	return true;
}

// send TEST picture to display
void GLwindow::newPicture(){
////TEST//// with a cubic pano
#ifdef WIN32
	QString pfx = QString("/users/tommy/documents/");
#else
	QString pfx = QString("/home/tommy/");
#endif
#if 0
  // test with 6 cubic images
	static int sw = 0;
	pvpic->setType( pvQtPic::cub );
  
  if( sw > 0 )
	pvpic->setFaceImage( pvQtPic::front, 
			pfx + QString("pvQt/test/outside_000000.jpg") );
  if( sw > 1 )
	pvpic->setFaceImage( pvQtPic::right, 
			pfx + QString("pvQt/test/outside_000001.jpg") );
  if( sw > 2 )
	pvpic->setFaceImage( pvQtPic::back, 
			pfx + QString("pvQt/test/outside_000002.jpg") );
  if( sw > 3 )
	pvpic->setFaceImage( pvQtPic::left, 
			pfx + QString("pvQt/test/outside_000003.jpg") );
  if( sw > 4 )
	pvpic->setFaceImage( pvQtPic::top, 
			pfx + QString("pvQt/test/outside_000004.jpg") );
  if( sw > 5 )
	pvpic->setFaceImage( pvQtPic::bottom, 
			pfx + QString("pvQt/test/outside_000005.jpg") );

  sw = (sw + 1) % 7;
#else
	// test with a QTVR file
	QTVRDecoder dec;
	bool ok = dec.parseHeaders(
//		"/home/tommy/pvQt/test/OutsideSionHillCampus.mov"
		"/home/tommy/MiscProjects/freepv/testcases/good/MichelThoby_tiled_qtvr.mov"
// windows		"C:/users/tommy/documents/pvQt/test/OutsideSionHillCampus.mov"
	);
	if( !ok ){
		qCritical("QTVR parse: %s", dec.getError());
		return;
	}

	if( dec.getType() == PANO_CUBIC ){
		pvpic->setType( pvQtPic::cub );
		for( int i = 0; i < 6; i++ ){
			QImage * pim = dec.getImage( i );
			pvpic->setFaceImage( pvQtPic::PicFace(i), pim );
		}	
	} else qCritical("Not a cubic pano");
#endif
////END TEST//// 
	glview->showPic( pvpic );
}
