/* GLview.h for freepvQt 08 Sep2008 
  GLview is an OpenGL renderer for projected images

*/
#ifndef GLVIEW_H
#define GLVIEW_H

#include <QtOpenGL/QGLWidget>
#include "pvQt.h"

class GLview : public QGLWidget{
	Q_OBJECT
public:
     GLview(QWidget *parent = 0);
     ~GLview();

     QSize minimumSizeHint() const;
     QSize sizeHint() const;

 public slots:
/* Angles passed from & to GUI are integers in 16ths of a degree,
   so they can be reliably checked for equality.  The zoom angle 
   is the fov of the shorter window axis
*/ 
     void setPan(int iangle);
     void setTilt(int iangle);
     void setSpin(int iangle);
     void setZoom( int iangle );
  // dp should be +/- 1
	 void step_pan( int dp );
	 void step_tilt( int dp );
	 void step_zoom( int dp );
	 void step_roll( int dp );
	 void set_view( int i );

	 void newPicture();
	 void displayPic( pvQt & pic );

 signals:
	 void reportView( QString msg );

 protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);

 private:
	 pvQt  * thePic;
     GLuint makeSphere();
     double normalizeAngle(int &iangle, double lwr, double upr);
	 void showview();
	 void setFOV( double fov = 0 );
	 void initView();
  // current view parameters
	 double hFOV, vFOV;	// angular size, degrees
	 double minFOV, maxFOV; // limits on vFOV
	 int Width, Height;		// screen pixel dimensions
	 double Znear, Zfar;	// clipping plane distances from eye
	 double panAngle, tiltAngle, spinAngle; // degrees

     GLuint theSphere;
     int ipan, panstep;
     int itilt, tiltstep;
     int ispin, spinstep;
     int izoom, zoomstep;
     QPoint lastPos;
     QColor trolltechGreen;
     QColor trolltechPurple;
};

#endif //ndef GLVIEW_H
