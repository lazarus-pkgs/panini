/* GLview.cpp for freepvQt  08Sep2008 TKS
*/
#include "GLview.h"
#include <QtOpenGL/QtOpenGL>
#include <GL/glut.h>
#include <cmath>

#ifndef Pi
#define Pi 3.141592654
#define DEG(r) ( 180.0 * (r) / Pi )
#define RAD(d) ( Pi * (d) / 180.0 )
#endif

#define scaleFOV( V, r ) ( 4 * DEG( atan( (r) * tan( RAD( 0.25 * (V) )))))



 GLview::GLview(QWidget *parent)
     : QGLWidget(parent)
 {
     theSphere = 0;
     Width = Height = 400;
     initView();

     trolltechGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
     trolltechPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
 }

 GLview::~GLview()
 {
     makeCurrent();
     glDeleteLists(theSphere, 1);
 }

 QSize GLview::minimumSizeHint() const
 {
     return QSize(50, 50);
 }

 QSize GLview::sizeHint() const
 {
     return QSize(400, 400);
 }
 
 void GLview::initView()
 {
     ipan = 0; panstep = 32;
     itilt = 0; tiltstep = 32;
     ispin = 0; spinstep = 32;
     minFOV = 15;  maxFOV = 270;
     vFOV =  60.0;
     izoom = (int)(vFOV * 16); zoomstep = 5 * 16;
     panAngle = tiltAngle = spinAngle = 0;
}

void GLview::set_view( int i )
{
  switch(i){
  case 0:
	initView();
	updateGL();
	showview();
	break;
  }
}

 void GLview::setPan(int angle)
 {
     panAngle = normalizeAngle(angle, -180, 180);
     if (angle != ipan) {
         ipan = angle;
         updateGL();
		 showview();
     }
 }

 void GLview::setTilt(int angle)
 {
     tiltAngle = normalizeAngle(angle, -90, 90);
     if (angle != itilt) {
         itilt = angle;
         updateGL();
		 showview();
     }
 }

 void GLview::setSpin(int angle)
 {
     spinAngle = normalizeAngle(angle, -180, 180);
     if (angle != ispin) {
         ispin = angle;
         updateGL();
		 showview();
     }
 }

 void GLview::setZoom(int angle)
 {
 	double a = normalizeAngle(angle, minFOV, maxFOV);
	if (angle != izoom) {
         izoom = angle;
         setFOV(a);
         updateGL();
		 showview();
     }
 }

 double GLview::normalizeAngle(int &iangle, double lwr, double upr)
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
 		while ( a <= 0.0 ) a += 360.0;
		while ( a > 360.0 ) a -= 360.0;		
	}
	if( a < lwr ) a = lwr;
	else if( a > upr ) a = upr;
	iangle = (int)( 16 * a );
	return a;

 }

 void GLview::initializeGL()
 {
     qglClearColor(trolltechPurple.dark());
     theSphere = makeSphere();
     glShadeModel(GL_FLAT);
     glEnable(GL_DEPTH_TEST);
     glEnable(GL_CULL_FACE);
     
     
 }

 void GLview::paintGL()
 {
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 // eye at (-1,0,0) looking up the X axis gives stereographic projection
     glLoadIdentity();
     gluLookAt( -1, 0, 0, 1, 0, 0, 0, 0, 1);
 // Treat rotations as Euler angles
     glRotated( spinAngle, 1, 0, 0 );     
     glRotated( tiltAngle, 0, 1, 0 );
     glRotated( panAngle, 0, 0, 1 );
     glCallList(theSphere);
 }

 void GLview::resizeGL(int width, int height)
 {
// Viewport fills window.
	glViewport(0, 0, width, height);
// Adjust FOV to keep zoom constant
	double r = (double) height / (double) Height;
	double f = scaleFOV( vFOV, r );
	Width = width; Height = height;
 	setFOV( f );
 	showview();
 }
 
 void GLview::setFOV( double newfov ){
 /* Set angular size of the clipping window
   reads Width, Height
   posts vFOV = newfov, clipped legal, hFOV
   sets clipping plane distances assuming the viewed
   surface is the unit sphere
*/
	if( Height < 1 ) return;
	if(newfov == 0 ) newfov = vFOV;
	if( newfov < minFOV ) newfov = minFOV;
	else if( newfov > maxFOV) newfov = maxFOV;
	
	vFOV = newfov;
	double ar = (double)Width / (double)Height;
	hFOV = scaleFOV( vFOV, ar );
	
	Znear = 0.1; Zfar = 2.5;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(0.5 * vFOV, ar, Znear, Zfar);
	glMatrixMode(GL_MODELVIEW);

}

  GLuint GLview::makeSphere()
 {
	GLuint list = glGenLists(1);
	GLUquadricObj *qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_LINE);
	gluQuadricNormals(qobj, GLU_NONE);
	glNewList(list, GL_COMPILE);
		gluSphere(qobj, 1.0, 30, 30);
	glEndList();
	return list;
 }

 void GLview::mousePressEvent(QMouseEvent *event)
 {
     lastPos = event->pos();
 }

 void GLview::mouseMoveEvent(QMouseEvent *event)
 {
     int dx = event->x() - lastPos.x();
     int dy = event->y() - lastPos.y();


     lastPos = event->pos();
 }


 void GLview::step_pan( int dp ){
	 setPan( ipan - dp * panstep  );
 }

 void GLview::step_tilt( int dp ){
	 setTilt( itilt + dp * tiltstep );
 }

 void GLview::step_zoom( int dp ){
	 setZoom( izoom - dp * zoomstep );
 }
 void GLview::step_roll( int dp ){
	 setSpin( ispin - dp * spinstep );
 }

 void GLview::showview(){
	 QString s;
	 s.sprintf("Pan %.1f Tilt %.1f Spin %.1f  FOV %.1f x %.1f",
		 panAngle, tiltAngle, spinAngle, hFOV, vFOV );
	 emit reportView( s );
 }
 

