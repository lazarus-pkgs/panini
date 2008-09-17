/* GLwindow.cpp  for freepvQt 09Sep2008 TKS

*/
#include "GLwindow.h"

#include "GLview.h"

GLwindow::GLwindow (QWidget * parent )
: QWidget(parent)
{
	glview = new GLview(this);
  ok = (glview != 0 );

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
	connect( parent, SIGNAL(set_view( int )),
		     glview, SLOT(set_view( int )));
  if(ok) ok = 
	connect( glview, SIGNAL(reportView( QString )),
			 parent, SLOT(showStatus( QString )) );
  if(ok) ok = 
	connect( parent, SIGNAL(newPicture()),
			 glview, SLOT(newPicture()) );
  if(!ok) {
	  qFatal("GLwindow setup failed");
  }

}

// relay window resize to the GL widget
void GLwindow::resizeEvent( QResizeEvent * ev ){
		glview->resize( size() );
}
