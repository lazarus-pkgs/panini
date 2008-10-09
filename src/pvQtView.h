/* pvQtView.h 

*/
#ifndef PVQTVIEW_H
#define PVQTVIEW_H

#include <QtOpenGL/QGLWidget>
#include "pvQtPic.h"

class pvQtView : public QGLWidget
{
	Q_OBJECT
public:
     pvQtView(QWidget *parent = 0);
     ~pvQtView();

     QSize minimumSizeHint() const;
     QSize sizeHint() const;

 public slots:
/* Angles passed from/to GUI are integers in 16ths of a degree,
   so they can be reliably checked for equality.  The zoom angle 
   is the fov of the shorter window axis
*/ 
     void setPan(int iangle);	// unit = 1/16 degree
     void setTilt(int iangle);
     void setSpin(int iangle);
     void setZoom( int iangle );
	 void setDist( int idist );  // unit = 1% of radius
  // dp should normally be +/- 1
	 void step_pan( int dp );
	 void step_tilt( int dp );
	 void step_zoom( int dp );
	 void step_roll( int dp );
	 void step_dist( int dp );
  // preset views
  	 void reset_view();	// reinit all params
	 void home_view();	// zero view angles
	 void full_frame();	// stereographic, min zoom
	 void super_fish();	// circular superwide
	 

  /* set the picture to be displayed
	pic = 0 resets to base screen display.  Otherwise
	*pic must be valid until after showPic is next called
	(caller can then delete pic if appropriate).
  */
	 void showPic( pvQtPic * pic );
  // update display of current picture
	 void picChanged();

 signals:
	 void reportView( QString msg );

 protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);

 private:
 // GUI support
     double normalizeAngle(int &iangle, double lwr, double upr);
	 int iAngle( double angle );	// real to integer coded angle
	 void showview();
	 void setFOV( double fov = 0 );
	 void initView();
  // current view parameters
	 double hFOV, vFOV;	// angular size at sphere center (deg)
	 double minFOV, maxFOV; // limits on vFOV
	 double wFOV;  // angular vFOV at eye point (deg)
 	 double eyeDistance;	// of eye from origin, in sphere radii
	 double maxDist, minDist;

	 int Width, Height;		// screen pixel dimensions
	 double portAR;			// width/height
	 double Znear, Zfar;	// clipping plane distances from eye
	 double panAngle, tiltAngle, spinAngle; // degrees

     int ipan, panstep;
     int itilt, tiltstep;
     int ispin, spinstep;
     int izoom, zoomstep;
	 int idist, diststep;
     QPoint lastPos;

  // display support
	 void setPicType( pvQtPic::PicType pt );
	 void setupPic( pvQtPic * pic );
	 void updatePic();
	 pvQtPic  * thePic;
	 pvQtPic::PicType	picType;
  // display lists for screens
     void makeSphere( GLuint list );
	 GLuint theScreen;	// current screen list
  // textures
	 GLenum textgt;	// current target (2D or cube)
	 GLuint texIDs[2];	// 0: 2D, 1: cube object
	 GLuint theTex;	// current object


};

#endif //ndef PVQTVIEW_H
