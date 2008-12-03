/* pvQtView.cpp for pvQt  08Sep2008 TKS
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
 */

#include "pvQtView.h"

#include <QtOpenGL/QtOpenGL>
#ifdef __APPLE__
   #include "glext.h"
   #include "glu.h"
#else
   #include <GL/glext.h>
   #include <GL/glu.h>
#endif 
#include <cmath>

#define max( a, b ) (a > b ? a : b )
#define min( a, b ) (a > b ? b : a )
#define KLIP( x, l, u )  ((x)<(l)?(l):(x)>(u)?(u):(x))

#ifndef Pi
#define Pi 3.141592654
#define DEG(r) ( 180.0 * (r) / Pi )
#define RAD(d) ( Pi * (d) / 180.0 )
#endif

/**** maximum projection angle at eye ****/ 
#define MAXPROJFOV  140


/* C'tor for pvQtView 
  specifies a custom OGL context format, not because we need it 
  but in hope of working around a bug on Mac OSX that prevents
  proper cube map display when the default format is used.
*/
 pvQtView::pvQtView(QWidget *parent)
	 : QGLWidget( QGLFormat( QGL::AlphaChannel ),
				  parent)
 {
 // set up mouse control timer
	mTimer.setInterval( 50 );
	mTimer.setSingleShot( true );
	connect( &mTimer, SIGNAL(timeout()),
		     this, SLOT( mTimeout()) );

 // post unusable OGL capabilities
 	OGLv20 = OGLisOK = false;
	picType = pvQtPic::nil;
 	picok = false;
 	errmsg = tr("no picture");
	thePic = 0;
	theScreen = 0;
	textgt = 0;
	texname = 0;
	textgt = 0;


  // create the sphere tables
	pqs = new quadsphere( 50 );

     Width = Height = 400;
	 minpan = -180; maxpan = 180;
	 mintilt = -180; maxtilt = 180;
 	 turnAngle = 0;
     initView();
 }

 pvQtView::~pvQtView()
 {
     makeCurrent();
     glDeleteLists(theScreen, 1);
 }
 
/*  query and post OpenGl capabilities;
    return false if they are insufficient
    NOTE this code fails if run in c'tor,
    but must be run to enable display
*/
bool pvQtView::OpenGLOK()
{	
  // assume the worst
	texPwr2 = true;
	cubeMap = false;
	vertBuf = false;
	errmsg = tr("no error");

/* probe OGL capabilities...
  Use Qt's version flags to test for standard features
  then if necessary check the OGL extensions string
  I'd like to use gluCheckExtension for that, but it
  doesn't seem to be implemented on all systems.  And
  Apple doesn't seem to implement even the extensions
  string according to OGL specs, so we have to rely on 
  the Apple "OpenGL Extensions Guide", which gives the
  OSX version at which each extension is supported. 

  The minimum feasible OpenGL version is 1.2
*/
	unsigned int vf = QGLFormat::openGLVersionFlags();
	if( vf & QGLFormat::OpenGL_Version_1_2 ){  
		if( vf & QGLFormat::OpenGL_Version_1_3 ){
			cubeMap = true;
			if( vf & QGLFormat::OpenGL_Version_1_5 ){
				vertBuf = true;
				if( vf & QGLFormat::OpenGL_Version_2_0 ) texPwr2 = false;
			}
		}
	// check the GL_EXTENSIONS string
	// NOTE Apple says this works on OSX too
		QString glext((const char *)glGetString(GL_EXTENSIONS));
		if(texPwr2 )
			texPwr2 = ! (
				glext.contains(QString("GL_ARB_texture_non_power_of_two"))
			 || glext.contains(QString("GL_EXT_texture_non_power_of_two"))
			);
		if(!cubeMap)
			cubeMap = glext.contains(QString("GL_ARB_texture_cube_map"));
		if( !vertBuf ) 
			vertBuf = glext.contains(QString("GL_ARB_vertex_buffer_object"));
	}
  // nominal OGL texture dimension limits
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &max2d );
	glGetIntegerv( GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxcube );

  // operating controls
	OGLisOK = cubeMap;
	QS_BUF = vertBuf;

	if( !OGLisOK ){
		picok = false;
		errmsg = tr("OpenGL insufficient");
	} 

	return OGLisOK;
}

/**  GUI Interactions  **/

 QSize pvQtView::minimumSizeHint() const
 {
     return QSize(50, 50);
 }

 QSize pvQtView::sizeHint() const
 {
     return QSize(400, 400);
 }
 
 /**  Mouse control  **/

void pvQtView::mouseDoubleClickEvent( QMouseEvent * pme ){
  if( mk == Qt::ShiftModifier ){ 
	  step_iproj( 1 );
  }
}

void pvQtView::mousePressEvent( QMouseEvent * pme ){
	mx1 = mx0 = pme->x();
	my1 = my0 = pme->y();
	mb = pme->buttons();
	mk = pme->modifiers();
	mTimer.start();
}

void pvQtView::mouseMoveEvent( QMouseEvent * pme ){
	mx1 = pme->x();
	my1 = pme->y();
	mb = pme->buttons();
	mk = pme->modifiers();
}

void pvQtView::mouseReleaseEvent( QMouseEvent * pme ){
	mx1 = mx0 = pme->x();
	my1 = my0 = pme->y();
	mb = pme->buttons();
	mk = pme->modifiers();
	if( mb == 0 ) mTimer.stop();
}

void pvQtView::mTimeout(){

	if( mb == 0 ) return;

  // wait til previous draw done?
	makeCurrent();
//	glFinish();	

	int dx = (mx1 - mx0) / 3,
		dy = (my0 - my1) / 3;

	if( mb == Qt::LeftButton ){
		if( mk == Qt::NoModifier ){
		// adjust yaw & pitch
			ipan += dx;
			panAngle = normalizeAngle( ipan, 1, minpan, maxpan);
			itilt += dy;
			tiltAngle = normalizeAngle( itilt, 1, mintilt, maxtilt);
		} else if( mk == Qt::ShiftModifier ){
		// adjust texture magnification
			ihfov = KLIP( ihfov - dx, 0, 4000);
			ivfov = KLIP( ivfov - dy, 0, 4000);
			setTexMag( QSizeF( 1.0 + 0.00225 * ihfov, 1.0 + 0.0025 * ivfov ));
		} else if( mk == Qt::ControlModifier ){
		}
	} else if ( mb & Qt::RightButton ){
		if( mk == Qt::ControlModifier ){
		} else {
			if( mb & Qt::LeftButton ){
			// if both buttons x adjusts eye
				idist -= dx;
				setDist( idist );
			}
			// Y adjusts zoom
			izoom -= dy;
			setFOV( normalizeAngle( izoom, 1, minFOV, maxFOV ) );
		}
	}

	updateGL();
	showview();

	mTimer.start();
}


 double pvQtView::normalizeAngle(int &iangle, int istep, double lwr, double upr)
 {
 /* iangle is in degrees * 16.  Return value in degrees.
    Reduces angle to a principal value in (-180:180] or [0:360) depending 
    on sign of lwr, then clips this to the limits [lwr:upr].  Sets
    iangle to the nearest whole multiple of istep, so that the step points
	will not shift due to clipping
 */
 	if( lwr >= upr){
 		iangle = 0;
 		return 0;
	}
 	double a = iangle / 16.0;
 	if( lwr < 0 ){
 		while ( a <= -180.0 ) a += 360.0;
		while ( a > 180.0 ) a -= 360.0;
	} else {
 		while ( a < 0.0 ) a += 360.0;
		while ( a >= 360.0 ) a -= 360.0;		
	}
// clip a legal
	if( a < lwr ) a = lwr;
	else if( a > upr ) a = upr;
// post 'detented' iangle
	iangle = (int)( 16 * a );
	int r = iangle % istep;
	if( r < 0 ) iangle -= istep + r;
	else if( r > 0 ) iangle += istep - r;

	return a;

 }


 // "detented" keyboard driven view controls

 void pvQtView::step_pan( int dp ){
	 setPan( ipan + dp * panstep  );
 }

 void pvQtView::step_tilt( int dp ){
	 setTilt( itilt + dp * tiltstep );
 }

 void pvQtView::step_zoom( int dp ){
	 setZoom( izoom - dp * zoomstep ); // note zoom = 1/FOV
 }

 void pvQtView::step_roll( int dp ){
	 setSpin( ispin + dp * spinstep );
 }

 void pvQtView::step_dist( int dp ){
	 idist += dp * diststep;
	 setDist( idist );
	 updateGL();
	 showview();
 }

 void pvQtView::step_hfov( int dp ){
	ihfov = KLIP( ihfov - 10 * dp, 0, 4000);
	setTexMag( QSizeF( 1.0 + 0.00225 * ihfov, 1.0 + 0.0025 * ivfov ));
	updateGL();
 }

 void pvQtView::step_vfov( int dp ){
	ivfov = KLIP( ivfov - 10 * dp, 0, 4000);
	setTexMag( QSizeF( 1.0 + 0.00225 * ihfov, 1.0 + 0.0025 * ivfov ));
	updateGL();
 }

 void pvQtView::step_iproj( int dp ){
  // cycle thru 2D projection types
	int i = curr_ipt;
	if( i < Nprojections && thePic != 0 ){
		if( dp > 0 ){
			if( ++i >= Nprojections ) i = 0;
		} else {
			if( --i < 0 ) i = Nprojections - 1;
		}
		pvQtPic::PicType pt = pictypes.PicType( i );
		curr_fovs = thePic->changeFovType( picType, thePic->PictureFOV(), pt );
		curr_pt = pt;
		curr_ipt = i;
		makeSphere( theScreen );		// set & display proj
		setTexMag( QSizeF(xtexmag, ytexmag));	// display fovs
		updateGL();
	 }
 }

  /**  preset views  **/
  
 void pvQtView::reset_view(){
	setPicType( picType );
 	initView();
	makeSphere( theScreen );
 	updateGL();
 	showview();
}

 void pvQtView::home_view(){
	panAngle = tiltAngle = spinAngle = 0;
	ipan = itilt = ispin = 0;
	updateGL();
	showview();
 }
 
 void pvQtView::full_frame(){
	setDist( 1000 );
	setZoom( int(16 * maxFOV) );
	updateGL();
	showview();
 }
 
 void pvQtView::super_fish(){
	setDist( 1087 );
	setZoom( int(16 * maxFOV) );	// maximize FOV
	updateGL();
	showview();
 }

 // incremental turn picture
 void pvQtView::turn90( int d ){
	 turnAbs( turnAngle + d * 90.0);
 }

// absolute turn picture
void pvQtView::turnAbs( double deg ){
	 while( deg > 180 ) deg -= 360;
	 while( deg < -180 ) deg += 360;
	 if( deg != turnAngle ){
		turnAngle = deg;
		updateGL();
		showview();
		emit( reportTurn( turnAngle ));
	 }
}


 /** report current view **/
 
 void pvQtView::showview(){
	 QString s;
	 s.sprintf("Yaw %.1f  Pitch %.1f  Roll %.1f  Eye %.2f  vFOV %.1f",
		 panAngle, tiltAngle, spinAngle, eyeDistance, vFOV );
	 emit reportView( s );
 }

/**  API call to display a picture  **/
bool pvQtView::showPic( pvQtPic * pic )
 {
 	setupPic( pic );
 	return picok;
 }

void pvQtView::picChanged( )
{
	updatePic();
}

/**  Field-of-View Policy

	User can adjust 'zoom' and the distance of the eye point 
	from the center of the spherical projection screen, as well
	as the size of the display window.  Zoom sets the viewing
	magnification at the center of the window.  Eye position
	changes the viewing projection without affecting central
	magnification.  When the window is resized, 
	
	Eye distance changes the viewing projection by moving the 
	actual eye point away from the sphere center: 
		angle at eye = angle on sphere / (eye distance + 1) 
	where eye distance is measured in sphere radii.  The OGL
	projection angle, wFOV, is the eye angle corresponding
	to vFOV, the vertical field of view on the sphere.

	The working parameters are display height in pixels, Height;
	eyeDistance, in sphere radii; and vFOV.  The zoom control
	adjusts vFOV directly. The working limits on vFOV depend
	on Height and eyeDistance, so vFOV may change when one of
	those changes.

	As a practical matter the maximum view angle at the eye is
	limited to MAXPROJFOV, so that is the largest vFOV allowed
	when the eye is at sphere center (distance 0). As the eye
	moves out, more of the sphere becomes visible, up to a maximum
	of about 315 degrees at distance 1.08.  Beyond that point the
	visible area of the sphere shrinks again.

**/

void pvQtView::initView()
 {
 /* set the intial view parameters and the
	default user step increments 
*/
	eyeDistance = 0.0; // spherical projection
	minDist = 0.0; maxDist = 1.087;	// fixed limits
	idist = 0; diststep = 10;	// % of radius

	minFOV = 10.0;  maxFOV = MAXPROJFOV;
 	zoomstep = 40;	// 2.5 degrees in FOV
	izoom = 90 * 16;
	setFOV(90);

	panstep = tiltstep = spinstep = 32;	// 2 degrees
 	panAngle = tiltAngle = spinAngle = 0;
	ipan = itilt = ispin = 0;
	ihfov = ivfov = 0;

}

void pvQtView::setFOV( double newvfov ){
 /*   If newfov is zero or omitted, uses current vFOV.
  sets
	vFOV = newvfov (clipped legal) = view height as an
		angle at the center of the unit sphere
	wFOV = half view height as an angle at the eye point.
*/
	if(newvfov == 0 ) newvfov = vFOV;
	if( newvfov < minFOV ) newvfov = minFOV;
	else if( newvfov > maxFOV) newvfov = maxFOV;
	
	vFOV = newvfov;
	wFOV = vFOV / (eyeDistance + 1);		
}

void pvQtView::setDist( int id ){
/*  set distance of eye from sphere center,
	Post new vFOV limits.
	Then update the screen.
*/
	double d = id / 1000.0;
	if( d < minDist ) d = minDist;
	if( d > maxDist ) d = maxDist;
	idist = int( 1000 * d );
	eyeDistance = d;

	double m = MAXPROJFOV * (d > 1 ? 2 : d + 1 );
	if( m < minFOV ) m = minFOV;
	maxFOV = m;
	setFOV( );
}

 void pvQtView::setPan(int angle)
 {
     panAngle = normalizeAngle(angle, panstep, minpan, maxpan);
     if (angle != ipan) {
        ipan = angle;
        updateGL();
	showview();
     }
 }

 void pvQtView::setTilt(int angle)
 {
     tiltAngle = normalizeAngle(angle, tiltstep, mintilt, maxtilt);
     if (angle != itilt) {
        itilt = angle;
        updateGL();
	showview();
     }
 }

 void pvQtView::setSpin(int angle)
 {
     spinAngle = normalizeAngle(angle, spinstep, -180, 180);
     if (angle != ispin) {
		ispin = angle;
		updateGL();
		showview();
     }
 }

 void pvQtView::setZoom(int angle)
 {
 	double a = normalizeAngle(angle, zoomstep, minFOV, maxFOV);
	if (angle != izoom) {
		izoom = angle;
        setFOV(a);
        updateGL();
		showview();
     }
 }

 /* check for (and reset) async OpenGL error;
   if picok is currently true, post any error to it
   and to errmsg and emit an error signal; otherwise 
   leave them as-is.
   But in any case return current OGL error status.
 */
bool pvQtView::OGLok( const char * label ){
	GLenum c = glGetError();
	if( c == GL_NO_ERROR ) return true;
	if( picok ){
		picok = false;
		errmsg = QString("%1 OGL error: ").arg(label)
			+ QString( (const char *)gluErrorString( c ) );
		emit OGLerror( errmsg );
	}
	return false;
}

/* One-time setup of the OpenGL environment
  Determines the biggest feasible texture sizes
*/
 void pvQtView::initializeGL()
 {
  // debug check for quadsphere setup error
 	const char * erm = pqs->errMsg();
	if( erm != 0 ){
		qFatal("quadsphere: %s", erm );
	}

  // abort if the OpenGL version is insufficient
 	if( !OpenGLOK() ) return;

	qglClearColor(Qt::black);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);

  // create texture objects
	glGenTextures( 2, texnms );
	glBindTexture( GL_TEXTURE_2D, texnms[0] );
	glBindTexture( GL_TEXTURE_CUBE_MAP, texnms[1] );

  // find the largest feasible textures
	maxTex2Dsqr = maxTexSize( GL_PROXY_TEXTURE_2D, max2d, max2d );
	maxTex2Drec = maxTexSize( GL_PROXY_TEXTURE_2D, max2d, max2d / 2 );
	maxTexCube = maxTexSize( GL_PROXY_TEXTURE_CUBE_MAP, maxcube, maxcube );

  // constant texture mapping parameters...
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  // for cube maps...
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // 2d maps...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // create a displaylist
	theScreen = glGenLists(1);

  // make wireframe panosphere
	makeSphere( theScreen );

}

/* find the largest feasible texture dimensions
  proportional to and not larger than a given pair
  using a proxy test
*/
 QSize pvQtView::maxTexSize( GLenum proxy, int tw, int th ){
	for(;;){
		GLint v;
		glTexImage2D( proxy, 0, GL_RGBA, tw, th, 0, 
			GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_WIDTH, &v );
		if( v == tw ){	// feasible size...
			break;
		} else {	// infeasible
		  // decrease size and try again
			if( texPwr2 ){
				tw /= 2; th /= 2;
			} else {
				tw = int( 0.8 * tw ); 
				th = int( 0.8 * th );
			}
			if( max( tw, th ) < 64 ) break;	// sanity?
		}
	}
	return QSize( tw, th );
}

 
/* Set picture type-specific OpenGL options
   and class variables.
   Disables OGL options that might have been
   set for other pic types.
   pic type 0 clears and disables all.
*/
 void pvQtView::setPicType( pvQtPic::PicType pt )
 {
	picType = pt;	// official
	ipicType = pictypes.picTypeIndex( picType );

	minpan = -180; maxpan = 180;
	mintilt = -180; maxtilt = 180;
	xtexmag = ytexmag = 1.0;

	curr_pt = picType;
	curr_ipt = ipicType;
	if( curr_pt > 0 ){
		curr_fovs = thePic->PictureFOV();
		setTexMag( thePic->getTexScale() );
	} else curr_fovs = QSizeF(0, 0);

	makeCurrent();	// get OGL's attention
	if(textgt){
		glDisable( textgt ); 
		texname = 0;
		textgt = 0;
	}

    glDisable( GL_TEXTURE_GEN_S );
    glDisable( GL_TEXTURE_GEN_T );
    glDisable( GL_TEXTURE_GEN_R );
    
	glFrontFace( GL_CCW );
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	if( picType == pvQtPic::cub ){
	/* for cube map, generate texture coords from normals */
		textgt = GL_TEXTURE_CUBE_MAP;
		texname = texnms[1];
		glEnableClientState(GL_NORMAL_ARRAY);
		glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGenf( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glEnable( GL_TEXTURE_GEN_R );
	} else if( picType != pvQtPic::nil ) {
	/* for 2D maps, use quadsphere texture coordinates */
		textgt = GL_TEXTURE_2D;
		texname = texnms[0];
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	 // black border color for 2D textures
		float bord[4] = { 0, 0, 0, 1 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bord);
	} else {
	/* no picture, show wireframe */
	}
 }

/* set working texture coordinate magnifications,
  and report the corresponding apparent FOV
*/
void pvQtView::setTexMag( QSizeF mags ){
	double xmag = mags.width(),
		   ymag = mags.height();
	if( xmag < 1.0 ) xmag = 1.0;
	else if( xmag > 10.0 ) xmag = 10.0;
	if( ymag < 1.0 ) ymag = 1.0;
	else if( ymag > 10.0 ) xmag = 10.0;
	xtexmag = xmag;
	ytexmag = ymag;
  // report the apparent image FOV
	if( thePic ){
		QSizeF rats(1.0 / xtexmag, 1.0 / ytexmag);
		curr_fovs = thePic->picScale2Fov( curr_pt, thePic->PictureFOV(), rats );
		emit reportFov( curr_fovs );
	}
}

 /**  Display a Frame  **/

 void pvQtView::paintGL()
 {
 // abort if the OpenGL version is insufficient
	if( !OGLisOK ) return;
 	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if( textgt ){	//there is a picture...
		glBindTexture( textgt, texname );
		glEnable( textgt );
	}
  // 2D texture border mode
	if( textgt ==  GL_TEXTURE_2D ){
		GLuint sclamp, tclamp;
		sclamp = tclamp = GL_CLAMP_TO_BORDER;
		if( curr_fovs.width() >= 360 ) sclamp = GL_CLAMP_TO_EDGE;
		if( curr_fovs.height() >= 180 ) tclamp = GL_CLAMP_TO_EDGE;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sclamp);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tclamp);
	}

// set texture rotation and scaling
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	
	if(picType == pvQtPic::cub){
  // cube texture rotates around origin
		glRotated( 180, 0,1,0 );
		glRotated( 180, 0,0,1 );
		glRotated( -turnAngle, 0, 0, 1 );
	} else {
  // 2D textures rotate and scale around pic center
		glTranslated( 0.5, 0.5, 0 );
		glScaled( xtexmag, ytexmag, 1.0 );
		glRotated( -turnAngle, 0, 0, 1 );
		glTranslated( -0.5, -0.5, 0 );
	}

  // Set point of view
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Znear = 0.05; Zfar = 7;
	gluPerspective( wFOV, portAR, 
				    Znear,  Zfar);
	gluLookAt( 0, 0, -eyeDistance,
			   0, 0, 1,
			   0, 1, 0 );
	glRotated( tiltAngle, 1, 0, 0 );
	glRotated( panAngle, 0, 1, 0 );
	glRotated( -spinAngle, 0, 0, 1 );     


	glMatrixMode(GL_MODELVIEW);

  // Display the panosphere
	if( QS_BUF ) {	// use buffers
		glCallList(theScreen);				//// TODO: buffers
	} else glCallList(theScreen); // use display list
	
  // check for OGL error
	paintok = OGLok("paintGL");
 }

 void pvQtView::resizeGL(int width, int height)
 {
 // abort if the OpenGL version is insufficient
 	if( !OGLisOK ) return;

// Viewport fills window.
	glViewport(0, 0, width, height);

	Width = width; Height = height;
	portAR = (double)Width / (double)Height;

	updateGL();
 	showview();  // displays settings
 }
 

void pvQtView::makeSphere( GLuint list )
 {
  // abort if the OpenGL version is insufficient
 	if( !OGLisOK ) return;

	if( textgt ){	// there is an image
		glNewList(list, GL_COMPILE);
		  glVertexPointer( 3, GL_FLOAT, 0, pqs->vertices());
		  glNormalPointer( GL_FLOAT, 0, pqs->vertices());
		  glTexCoordPointer( 2, GL_FLOAT, 0, pqs->texCoords( curr_pt ));
		  glDrawElements(GL_QUADS, pqs->quadIndexCount(), 
			  GL_UNSIGNED_INT, pqs->quadIndices() );
		glEndList();
	 // report the projection used
		emit reportProj(QString( pictypes.picTypeName( curr_pt )));
	} else {
		glEnableClientState(GL_VERTEX_ARRAY);
		glNewList(list, GL_COMPILE);
		  glVertexPointer( 3, GL_FLOAT, 0, pqs->vertices());
		  glDrawElements(GL_LINES, pqs->lineIndexCount(), 
			  GL_UNSIGNED_INT, pqs->lineIndices());
		glEndList();
	 // report the projection used
		emit reportProj(QString("none"));
	}
 }


/* Load a picture
  pass pic == 0 to just clear all picture state
  
  fails if it can't negotiate a feasible texture size
  and format with the pvQtPic
*/
// texture cube faces in the order pvQtPic uses
static GLenum cubefaces[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // front
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,	// right
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,	// back
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // left
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,	// top
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	// bottom
};


bool pvQtView::setupPic( pvQtPic * pic )
{
 // abort if the OpenGL version is insufficient
 	if( !OGLisOK ){
 		errmsg = tr("Insufficient OpenGL facilities");
		return false;
	}
	
/* Reset picture type specific state
*/	
	thePic = pic;
	if( pic ) picType = pic->Type();
	else picType = pvQtPic::nil;
 	
  // set up OGL for the picture type
	setPicType( picType );

  // reinitialize viewing parameters
	initView();

  // reset error status
    picok = true;
 	errmsg = tr("no error");

/* limit face size to feasible texture size
*/

// select largest feasible texture size
	QSize maxdims;
	switch( picType ){
	case pvQtPic::nil:
		errmsg = tr("s/w error: empty pvQtPic");
		return false;
		break;
	case pvQtPic::rec:
	case pvQtPic::eqs:
	case pvQtPic::eqa:
	case pvQtPic::stg:
		maxdims = maxTex2Dsqr;
		break;
	case pvQtPic::cyl:
	case pvQtPic::eqr:
	case pvQtPic::mrc:
		maxdims = maxTex2Drec;
		break;
	case pvQtPic::cub:
		maxdims = maxTexCube;
		break;
	}

	thePic->fitFaceToImage( maxdims );

/**  Load texture images  **/
	makeCurrent();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // QImage row alignment

	if( picType == pvQtPic::cub ){
		for(int i = 0; i < 6; i++){
			QImage * p = thePic->FaceImage(pvQtPic::PicFace(i));
			if( p ){
				glTexImage2D( cubefaces[i], 0, GL_RGBA,
					p->width(), p->height(), 0,
					GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
					p->bits() );
				delete p;
				if( !OGLok("load cube") ) return false;
			}
		}
	} else {
		QImage * p = thePic->FaceImage(pvQtPic::PicFace(0));
		if( p ){
			glTexImage2D( textgt, 0, GL_RGBA,
				p->width(), p->height(), 0,
				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
				p->bits() );
			delete p;
			if( !OGLok("load 2D") ) return false;
		}
	}

  // reset the view and display
	reset_view();

// check for (?asychronous?) OpenGL error
	return picok;

}

void pvQtView::updatePic()
{
	setupPic( thePic );
}

/* pick the face that contains a mouse position
*/
pvQtPic::PicFace pvQtView::pickFace( QPoint pnt )
{
	if( curr_pt != pvQtPic::cub ) return pvQtPic::front;
  // select the default texture object
	GLuint svnm = texname;
	texname = 0;
	makeCurrent();
	glBindTexture( textgt, texname );
  // set required texture parameters
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//	glEnableClientState(GL_NORMAL_ARRAY);
	glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
	glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
	glTexGenf( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
	glEnable( GL_TEXTURE_GEN_R );
  // load coded cube face images
	unsigned int timg[4096];	// 64 x 64
	for(int i = 0; i < 6; i++ ){
		memset(timg, i, 4096 * 4 );
		glTexImage2D( cubefaces[i], 0, GL_RGBA,
					  64, 64, 0, GL_RGBA,
					  GL_UNSIGNED_BYTE, timg );
	}
  // render to an offscreen buffer
	GLenum buf = GL_BACK;
	glDrawBuffer( buf );
	paintGL();
  // read pixel at cursor position
	glReadBuffer( buf );
	GLint x = pnt.x(),
		  y = Height - pnt.y();
	glReadPixels( x, y, 1, 1, GL_RGBA,
				  GL_UNSIGNED_BYTE, timg );
  // restore image display texture
	texname = svnm;
	glBindTexture( textgt, texname );

  // result is pixel value
	return pvQtPic::PicFace( 255 - (timg[0] & 0xFF) );

}

/* reload a cube face texture image
*/
void pvQtView::newFace( pvQtPic::PicFace face )
{
	if( curr_pt != pvQtPic::cub ) return;

	makeCurrent();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // QImage row alignment
	QImage * p = thePic->FaceImage( face );
	if( p ){
		glTexImage2D( cubefaces[int(face)], 0, GL_RGBA,
			p->width(), p->height(), 0,
			GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
			p->bits() );
		delete p;
	}

	updateGL();
	showview();
	
}
