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
   #include <GL/glut.h>
#endif 
#include <cmath>

#ifndef Pi
#define Pi 3.141592654
#define DEG(r) ( 180.0 * (r) / Pi )
#define RAD(d) ( Pi * (d) / 180.0 )
#endif

/**** maximum projection angle at eye ****/ 
#define MAXPROJFOV  137.5


 pvQtView::pvQtView(QWidget *parent)
     : QGLWidget(parent)
 {
 // post unusable OGL capabilities
 	OGLv20 = OGLv14 = false;
 	 picok = false;
 	 errmsg = tr("no picture");
	 thePic = 0;
	 theScreen = 0;
	 textgt = 0;
 // set viewmatrix to rgt handed identity
	 for(int i = 0; i < 16; i++ ){
		 viewmatrix[i] = 0.0;
	 }
	 viewmatrix[0] = viewmatrix[5] = 1.0;
	 viewmatrix[10] = viewmatrix[15] = 1.0;

     Width = Height = 400;
	 minpan = -180; maxpan = 180;
	 mintilt = -90; maxtilt = 90;
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
	errmsg = tr("no error");
	QString glver((const char *)glGetString(GL_VERSION));
	QStringList vns = glver.split(QChar('.'));
	
	int vn0 = vns[0].toInt();
	OGLv20 = vn0 >= 2;
	if( !OGLv20 && vns.count() > 1 ){
		OGLv14 = vn0 == 1 && vns[1].toInt() >= 4;
	} else OGLv14 = OGLv20;

	QString glext((const char *)glGetString(GL_EXTENSIONS));
	texPwr2 = !glext.contains(QString("GL_ARB_texture_non_power_of_two"));
	
	if( !OGLv14 ){
		picok = false;
		errmsg = tr("OpenGL insufficient");
	} 
	return OGLv14;
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
 
 void pvQtView::mousePressEvent(QMouseEvent *event)
 {
     lastPos = event->pos();
 }

 void pvQtView::mouseMoveEvent(QMouseEvent *event)
 {
     int dx = event->x() - lastPos.x();
     int dy = event->y() - lastPos.y();


     lastPos = event->pos();
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

  /**  preset views  **/
  
 void pvQtView::reset_view(){
 	initView();
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
	setDist( 100 );
	setZoom( int(16 * maxFOV) );
	updateGL();
	showview();
 }
 
 void pvQtView::super_fish(){
	setDist( 110 );
	setZoom( int(16 * maxFOV) );	// maximize FOV
	updateGL();
	showview();
 }

 /** report current view **/
 
 void pvQtView::showview(){
	 QString s;
	 s.sprintf("Yaw %.1f  Pitch %.1f  Roll %.1f  Eye %.1f  vFOV %.1f",
		 panAngle, tiltAngle, spinAngle, eyeDistance, vFOV );
	 emit reportView( s );
 }

/**  API call to display a picture  **/
bool pvQtView::showPic( pvQtPic * pic )
 {
 	errmsg = tr("no error");
 	picok = setupPic( pic );
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
	 minDist = 0.0; maxDist = 1.1;	// fixed limits
	 idist = 0; diststep = 10;	// % of radius

     minFOV = 10.0;  maxFOV = MAXPROJFOV;
     zoomstep = 40;	// 2.5 degrees in FOV
	 izoom = 90 * 16;
	 setFOV(90);

     panstep = tiltstep = spinstep = 32;	// 2 degrees
	 home_view();	// zero rotation
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
	double d = id / 100.0;
	if( d < minDist ) d = minDist;
	if( d > maxDist ) d = maxDist;
	idist = int( 100 * d );
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

/* Set picture type-specific OpenGL options
   and class variables.
   Disables OGL options that might have been
   set for other pic types.
   pic type 0 clears and disables all.
*/
 void pvQtView::setPicType( pvQtPic::PicType pt )
 {
	makeCurrent();	// get OGL's attention
	if(textgt) glDisable( textgt ); textgt = 0;
    glDisable( GL_TEXTURE_GEN_S );
    glDisable( GL_TEXTURE_GEN_T );
    glDisable( GL_TEXTURE_GEN_R );
    
	
	textgt = GL_TEXTURE_2D;
	viewmatrix[0] = 1.0;	// right handed...
	glFrontFace( GL_CW );
	switch( pt ){
	default:
		pt = pvQtPic::nil;
	case pvQtPic::nil:		// No picture
		break;
	case pvQtPic::sph:		// A spherical image up to 180 degrees diameter
		viewmatrix[0] = -1.0;	// left handed...
		glFrontFace( GL_CCW );
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		break;
	case pvQtPic::cyl:		// A cylindrical panorama up to 360 x 135 degrees
		break;
	case pvQtPic::eqr:		// An equirectangular panorama up to 360 x 180 degrees
		break;
	case pvQtPic::rec:		// A rectilinear image up to 90x90 degrees
	case pvQtPic::cub:		// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
		textgt = GL_TEXTURE_CUBE_MAP;
		glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGenf( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glEnable( GL_TEXTURE_GEN_R );
		break;
	}

	picType = pt;
 }

/* One-time setup of the OpenGL environment
*/
 void pvQtView::initializeGL()
 {
  // abort if the OpenGL version is insufficient
 	if( !OpenGLOK() ) return;

	qglClearColor(Qt::black);
	glShadeModel(GL_SMOOTH);
////	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

 // create 2D and cube texture objects
	glGenTextures( 2, texIDs );
	glBindTexture(GL_TEXTURE_2D, texIDs[0] );
	glBindTexture(GL_TEXTURE_CUBE_MAP, texIDs[1] );
  // constant texture mapping parameters...
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // create projection screen displaylist
	theScreen = glGenLists(1);
  // make spherical screen
	makeSphere( theScreen );

  // clear the picture type specific state
	 setPicType( pvQtPic::nil );
   
 }
 
 /* check for (and reset) OpenGL errors;
   if no error return true, else
   post description to errmsg and return false;
 */
bool pvQtView::OGLok(){
	GLenum c = glGetError();
	if( c == GL_NO_ERROR ) return true;
	errmsg = QString("OpenGL error: ")
			+ QString( (const char *)gluErrorString( c ) );
	return false;
}

 void pvQtView::paintGL()
 {
 // abort if the OpenGL version is insufficient
	if( !OGLv14 ) return;
 	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if( textgt ) glEnable( textgt );

 // eye position rotates, screen does not
	glMatrixMode(GL_PROJECTION);
    glLoadMatrixd( viewmatrix );
	Znear = 0.05; Zfar = 7;
	gluPerspective( wFOV, portAR, 
				    Znear,  Zfar);
/* set view according to picture type
   rotations are Euler angles; orientation & sense set here
*/
	switch( picType ){
	default:
	case pvQtPic::nil:
	case pvQtPic::cyl:
	case pvQtPic::eqr:		// look +Y, X rgt, Z up
		gluLookAt( 0, -eyeDistance, 0,
				   0, 1, 0,
				   0, 0, 1 );
		glRotated( 180 - spinAngle, 0, 1, 0 );     
		glRotated( tiltAngle, 1, 0, 0 );
		glRotated( 180 - panAngle, 0, 0, 1 );
		break;
	case pvQtPic::sph:
		gluLookAt( 0, 0, -eyeDistance,
				   0, 0, 1,
				   0, -1, 0 );
		glRotated( 180 + spinAngle, 0, 0, 1 );     
		glRotated( tiltAngle, 1, 0, 0 );
		glRotated( 180 - panAngle, 0, 1, 0 );
		break;
	case pvQtPic::rec:
	case pvQtPic::cub:		// look +Z, X lft, Y up
		gluLookAt( 0, 0, -eyeDistance,
				   0, 0, 1,
				   0, 1, 0 );
		glRotated( 180 - spinAngle, 0, 0, 1 );     
		glRotated( -tiltAngle, 1, 0, 0 );
		glRotated( 180 - panAngle, 0, 1, 0 );
		break;
	}

	glMatrixMode(GL_MODELVIEW);

	glCallList(theScreen);
	
	picok = OGLok();	// post any OGL error
 }

 void pvQtView::resizeGL(int width, int height)
 {
 // abort if the OpenGL version is insufficient
 	if( !OGLv14 ) return;

// Viewport fills window.
	glViewport(0, 0, width, height);

	Width = width; Height = height;
	portAR = (double)Width / (double)Height;

 	showview();  // displays settings
 }
 

void pvQtView::makeSphere( GLuint list )
 {
  // abort if the OpenGL version is insufficient
 	if( !OGLv14 ) return;

	GLUquadricObj *qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, textgt ? GLU_FILL : GLU_LINE);
	gluQuadricNormals(qobj, GLU_SMOOTH);
	if( picType == pvQtPic::eqr ) gluQuadricTexture(qobj, GL_TRUE );

	glNewList(list, GL_COMPILE);
		gluSphere(qobj, 1.0, 72, 70);
	glEndList();
 }


/* set up screen and texture maps for a picture
  pass pic == 0 to just clear all picture state
  
  fails if it can't negotiate a feasible texture size
  and format with the pvQtPic
*/
bool pvQtView::setupPic( pvQtPic * pic )
{
// texture cube faces in the order pvQtPic uses
	static GLenum cubefaces[6] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // front
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,	// right
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,	// back
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // left
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,	// top
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	// bottom
	};
/* Reset picture type specific state
*/	
	thePic = pic;
	if( pic ) picType = pic->Type();
	else picType = pvQtPic::nil;
	
 // abort if the OpenGL version is insufficient
 	if( !OGLv14 ){
 		errmsg = tr("Insufficient OpenGL version");
		return false;
	}
/*  Choose a display strategy based on image type and FOV,
	set the texture image FOV, then set a feasible size.
	
  The strategies for variable fov pictures are
	rec: embed in front cube face (90x90 deg)
	sph: embed in front hemisphere face (180x180)
	cyl: embed in full cylinder (360 x vfov)
	eqr: embed in full equi (360 x 180)
  NOTE With proper texture coordinates, there would be
  no need to use padded texture images.

  The max feasible face size is found by probing OGL
  with a proxy texture image whose aspect ratio is fixed
  by the face fovs.  If OGL requires power-of-2 textures,
  the test face dims are powers of 2.

*/  
	int tw, th;
	switch( picType ){
	case pvQtPic::nil:
		errmsg = tr("s/w error: empty pvQtPic");
		return false;
		break;
	case pvQtPic::rec:
		pic->setFaceFOV(QSizeF( 90, 90 ));
		tw = th = 256;
		break;
	case pvQtPic::sph:
		pic->setFaceFOV(QSizeF( 360, 360 ));
		tw = th = 512;
		break;
	case pvQtPic::cyl:
		pic->setFaceFOV(QSizeF( 360, 135 ));
		tw = 512; th = 256;
		break;
	case pvQtPic::eqr:
		pic->setFaceFOV(QSizeF( 360, 180 ));
		tw = 512; th = 256;
		break;
	case pvQtPic::cub:
		pic->setFaceFOV(QSizeF( 90, 90 ));
		tw = th = 256;
		break;
	}
  // set up OGL for the picture type
	setPicType( picType );

  /* find max feasible texture size 
    starting with tw,th, increase proportionally until infeasible, 
	or both picture dimensions would be > source image dimensions;
	then back off one step.
  */
	GLenum proxy = textgt == GL_TEXTURE_2D ? 
			GL_PROXY_TEXTURE_2D : GL_PROXY_TEXTURE_CUBE_MAP;
	int tww = 0, thw = 0;	// last feasible size
	for(;;){
		GLint v;
		glTexImage2D( proxy, 0, GL_RGB, tw, th, 0, 
			GL_RGB, GL_UNSIGNED_BYTE, 0 );
		glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_WIDTH, &v );
		if( v == 0 ){	// infeasible size...
			if( tww == 0 || thw == 0 ){
				errmsg = tr("no feasible picture size!");
				return false;
			}
			tw = tww; th = thw;	// use last feasible size
			pic->setFaceSize( QSize( tw, th ) );
			break;
		} else {		// feasible size...
		  // post face size, check picture size
			pic->setFaceSize( QSize( tw, th ));
			QSize pdd = pic->PictureSize() - pic->ImageSize();
			if( !pdd.isEmpty() ){
			  // picture >= image: fit face to image if possible
				if( !texPwr2 ) pic->fitFaceToImage();
				break;
			}
		  // increase size and try again
			tww = tw; 	thw = th;	// post last feasible size
			if( texPwr2 ){
				tw += tw; th += th;
			} else {
				tw = int( tw * 1.2599 ); // (cube root of 2)
				th = int( th * 1.2599 );
			}
		}
	}

  // set pan and tilt limits according to picture fov
	QSizeF pv = pic->PictureFOV();
	double d = 0.5 * pv.width();
	minpan = -d; maxpan = d;
	d = 0.5 * pv.height();
	mintilt = -d; maxtilt = d;

/* Load texture images */

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // QImage row alignment

	if( picType == pvQtPic::cub 
	 || picType == pvQtPic::rec ){
		for(int i = 0; i < 6; i++){
			QImage * p = pic->FaceImage(pvQtPic::PicFace(i));
			if( p ){
				glTexImage2D( cubefaces[i], 0, GL_RGB,
					p->width(), p->height(), 0,
					GL_RGB, GL_UNSIGNED_BYTE,
					p->bits() );
				delete p;
			}
		}
	} else {
		QImage * p = pic->FaceImage(pvQtPic::PicFace(0));
		if( p ){
			glTexImage2D( textgt, 0, GL_RGB,
				p->width(), p->height(), 0, 
				GL_RGB, GL_UNSIGNED_BYTE,
				p->bits() );
			delete p;
		}
	}

  // rebuild the display list
	makeSphere( theScreen );
	
  // show user the view parameters
	showview();

  // repaint the view
	updateGL();
	
// check for (?asychronous?) OpenGL error
	return picok;

}

void pvQtView::updatePic()
{
	setupPic( thePic );
}
