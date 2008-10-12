/* pvQtView.cpp for pvQt  08Sep2008 TKS
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
	 thePic = 0;
	 theScreen = 0;
	 textgt = 0;

     Width = Height = 400;
     initView();
 }

 pvQtView::~pvQtView()
 {
     makeCurrent();
     glDeleteLists(theScreen, 1);
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

 double pvQtView::normalizeAngle(int &iangle, double lwr, double upr)
 {
 /* iangle is in degrees * 16.  Return value in degrees.
    Reduces angle to a principal value in (-180:180] or [0:360) depending 
    on sign of lwr, then clips this to the limits [lwr:upr]
    The result is placed in iangle and returned as a double.
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
	if( a < lwr ) a = lwr;
	else if( a > upr ) a = upr;
	iangle = (int)( 16 * a );
	return a;

 }

 int pvQtView::iAngle( double deg ){
	 return int(16 * deg);
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
	setFOV( maxFOV );
	updateGL();
	showview();
 }
 
 void pvQtView::super_fish(){
	setDist( 110 );
	setFOV( maxFOV );
	updateGL();
	showview();
 }

 /** report current view **/
 
 void pvQtView::showview(){
	 QString s;
	 s.sprintf("Yaw %.1f  Pitch %.1f  Roll %.1f  Dist %.1f  vFOV %.1f",
		 panAngle, tiltAngle, spinAngle, eyeDistance, vFOV );
	 emit reportView( s );
 }

 void pvQtView::showPic( pvQtPic * pic )
 {
	if( pic != 0 && pic->Type() == pvQtPic::nil ) {
		qWarning("pvQtView::showPic: empty pvQtPic ignored" );
		pic = 0;
	}
	setupPic( pic );
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
	limited to 135 degrees, so that is the largest vFOV allowed
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

     minFOV = 5.0;  maxFOV = MAXPROJFOV;
     zoomstep = 5 * 16;	// 5 degrees in FOV
     setFOV( 90.0 );

     panstep = tiltstep = spinstep = 32;	// 2 degrees
	 home_view();	// zero rotation
}

void pvQtView::setFOV( double newvfov ){
 /* set
	vFOV = newvfov (clipped legal) = view height as an
		angle at the center of the unit sphere
	wFOV = half view height as an angle at the eye point.
  If newfov is zero or omitted, uses current vFOV.
  Note also sets constant view clipping distances.
*/
	if(newvfov == 0 ) newvfov = vFOV;
	if( newvfov < minFOV ) newvfov = minFOV;
	else if( newvfov > maxFOV) newvfov = maxFOV;
	
	vFOV = newvfov;
	izoom = iAngle( vFOV );	
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
     panAngle = normalizeAngle(angle, -180, 180);
     if (angle != ipan) {
         ipan = angle;
         updateGL();
		 showview();
     }
 }

 void pvQtView::setTilt(int angle)
 {
     tiltAngle = normalizeAngle(angle, -180, 180);
     if (angle != itilt) {
         itilt = angle;
         updateGL();
		 showview();
     }
 }

 void pvQtView::setSpin(int angle)
 {
     spinAngle = normalizeAngle(angle, -180, 180);
     if (angle != ispin) {
         ispin = angle;
         updateGL();
		 showview();
     }
 }

 void pvQtView::setZoom(int angle)
 {
 	double a = normalizeAngle(angle, minFOV, maxFOV);
 	angle = iAngle( a );
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

	switch( pt ){
	default:
		pt = pvQtPic::nil;
	case pvQtPic::nil:		// No picture
		break;
	case pvQtPic::rec:		// A rectilinear image up to 135 x 135 degrees
		break;
	case pvQtPic::sph:		// A spherical image up to 180 degrees diameter
		break;
	case pvQtPic::cyl:		// A cylindrical panorama up to 360 x 135 degrees
		break;
	case pvQtPic::eqr:		// An equirectangular panorama up to 360 x 180 degrees
		glFrontFace( GL_CW );
		textgt = GL_TEXTURE_2D;
		break;
	case pvQtPic::cub:		// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
		glFrontFace( GL_CW );
		textgt = GL_TEXTURE_CUBE_MAP;
		glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGenf( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glEnable( GL_TEXTURE_GEN_R );
		break;
	case pvQtPic::hem:		// A panorama with 1 or 2 hemispherical images
		break;
	}

	picType = pt;
 }

 void pvQtView::initializeGL()
 {
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
	makeSphere( theScreen );

  // clear the picture type specific state
	 setPicType( pvQtPic::nil );
   
 }

 void pvQtView::paintGL()
 {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if( textgt ) glEnable( textgt );

 // eye position rotates, screen does not
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	Znear = 0.05; Zfar = 7;
	gluPerspective( wFOV, portAR, 
				    Znear,  Zfar);
/* set view according to picture type
   rotations are Euler angles; orientation & sense set here
*/
	switch( picType ){
	default:
	case pvQtPic::nil:
	case pvQtPic::rec:
	case pvQtPic::sph:
	case pvQtPic::cyl:
	case pvQtPic::eqr:		// look +Y, X rgt, Z up
		gluLookAt( 0, -eyeDistance, 0,
				   0, 1, 0,
				   0, 0, 1 );
		glRotated( 180 - spinAngle, 0, 1, 0 );     
		glRotated( tiltAngle, 1, 0, 0 );
		glRotated( 180 - panAngle, 0, 0, 1 );
		break;
	case pvQtPic::cub:		// look +Z, X lft, Y up
		gluLookAt( 0, 0, -eyeDistance,
				   0, 0, 1,
				   0, 1, 0 );
		glRotated( 180 - spinAngle, 0, 0, 1 );     
		glRotated( tiltAngle, 1, 0, 0 );
		glRotated( 180 - panAngle, 0, 1, 0 );
		break;
	case pvQtPic::hem:		//
		break;
	}

	glMatrixMode(GL_MODELVIEW);

	glCallList(theScreen);
 }

 void pvQtView::resizeGL(int width, int height)
 {
// Viewport fills window.
	glViewport(0, 0, width, height);

	Width = width; Height = height;
	portAR = (double)Width / (double)Height;

 	showview();  // displays settings
 }
 

void pvQtView::makeSphere( GLuint list )
 {
	GLUquadricObj *qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, textgt ? GLU_FILL : GLU_LINE);
	gluQuadricNormals(qobj, GLU_SMOOTH);
	if( picType == pvQtPic::eqr ) gluQuadricTexture(qobj, GL_TRUE );

	glNewList(list, GL_COMPILE);
		gluSphere(qobj, 1.0, 72, 70);
	glEndList();
 }


/* set up screen and texture maps for a picture
  pass pic == 0 to clear all picture state
*/
void pvQtView::setupPic( pvQtPic * pic )
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
/* Reset picture type specific vars and OpenGL options
*/
	thePic = pic;
	if( pic ) picType = pic->Type();
	else picType = pvQtPic::nil;
	setPicType( picType );
	if( picType == pvQtPic::nil ) return;

/* Load texture images
*/
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // QImage row alignment

	if( picType == pvQtPic::cub ){
		theTex = texIDs[1];
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
		theTex = texIDs[0];
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

  // repaint the view
	updateGL();

  // display view parameters
	showview();
}

void pvQtView::updatePic()
{
	setupPic( thePic );
}
