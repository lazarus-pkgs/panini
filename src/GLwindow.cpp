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

}
#if 0
// Handle signals from MainWindow
void GLwindow::step_pan( int dir ){
	glview->step_pan( dir );
}

void GLwindow::step_tilt( int dir ){
	glview->step_tilt( dir );
}

void GLwindow::step_zoom( int dir ){
	glview->step_zoom( dir );
}
void GLwindow::step_roll( int dir ){
	glview->step_roll( dir );
}

void GLwindow::set_view( int i ){
	glview->set_view( i );
}
#endif

void GLwindow::resizeEvent( QResizeEvent * ev ){
	// must resize the GLview when we get resized
		glview->resize( size() );
}
