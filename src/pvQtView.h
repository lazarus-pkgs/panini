/* pvQtView.h for pvQt 11 Sept 2008
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

 pvQtView is an OpenGL display widget that shows images projected on
 a 3D spherical or cylindrical screen.

 This class is the only one in pvQt that issues OpenGL calls.
 
 App should call OpenGLOK() before using this widget, and terminate 
 with error if it returns false (as nothing can be displayed).

*/
#ifndef PVQTVIEW_H
#define PVQTVIEW_H

#include <QtOpenGL/QGLWidget>
#include "pvQtPic.h"
#include "quadsphere.h"

class pvQtView : public QGLWidget
{
	Q_OBJECT
public:
     pvQtView(QWidget *parent = 0);
     ~pvQtView();

     QSize minimumSizeHint() const;
     QSize sizeHint() const;
     
/**  OpenGL attributes  **/

	bool OpenGLOK();	// false if OGL version too low
	
	QString OpenGLVersion(){
		return QString( (const char *)glGetString(GL_VERSION) );
	}
	QString OpenGLVendor(){
		return QString( (const char *)glGetString(GL_VENDOR) );
	}
	QString OpenGLHardware(){
		return QString( (const char *)glGetString(GL_RENDERER) );
	}
	QString OpenGLFeatures(){
		return QString("texPwr2 %1, cubeMap %2, vBuffer %3")
			.arg(texPwr2).arg(cubeMap).arg(QS_BUF);
	}

  /* Display a picture
	pic = 0 resets to base screen display.  Otherwise
	*pic must be valid and undisturbed until showPic is next 
	called (caller can then delete pic if appropriate).
	returns sucess or failure, with errmsg updated.
  */
	 bool showPic( pvQtPic * pic );
  /* check whether picture displayed OK, get message if not
    The error flag is reset when a new picture is loaded.
  */
	 bool picOK( QString & errMsg ){
	 	 errMsg = errmsg; 
	 	 return picok;
	 }
	 
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
  // keypress steps -- dp should normally be +/- 1
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
	 void turn90( int d) ;	// turn picture
  // update display of current picture
	 void picChanged();

 signals:
	 void reportView( QString msg );
	 void OGLerror( QString msg );

 protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     void mousePressEvent(QMouseEvent *pme );
     void mouseMoveEvent(QMouseEvent *pme );
	 void mouseReleaseEvent( QMouseEvent *pme );
private slots:
	 void mTimeout();
 private:
 // GUI support
     double normalizeAngle(int &iangle, int istep, double lwr, double upr);
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
	 double minpan, maxpan, mintilt,maxtilt;  //image limits
	 double turnAngle;		// 90 deg picture rotation

     int ipan, panstep;
     int itilt, tiltstep;
     int ispin, spinstep;
     int izoom, zoomstep;
	 int idist, diststep;

	int mx0, my0, mx1, my1;	// mouse coordinates
	Qt::MouseButtons mb;
	QTimer mTimer;


  // display support
	 void setPicType( pvQtPic::PicType pt );
	 bool setupPic( pvQtPic * pic );
	 void updatePic();
	 pvQtPic  * thePic;
	 pvQtPic::PicType	picType;
  // initial view matrix (rgt or left hand)
	 GLdouble viewmatrix[16];
  // display lists for screens
     void makeSphere( GLuint list );
	 GLuint theScreen;	// current screen list
  // textures
	 GLenum textgt;	// current target (2D or cube)
	 GLuint texIDs[2];	// 0: 2D, 1: cube object
	 GLuint theTex;	// current object
	 GLuint boundtex[6];	// textures converted from QImages
  // OpenGL capabilities
	bool OGLisOK;	// is at least version 1.4
	bool OGLv20;	// is at least version 2.0
	bool texPwr2;	// needs power-of-2 texture dimensions
	bool cubeMap;	// has cube mapping (rq'd)
  // status 
	bool paintok;	// most recent OGL error status
  	bool picok;		// sticky OGL error flag
  	QString errmsg;	// sticky OGL error message
  	bool OGLok();	// check & post OGL errors
  // tabulated sphere points and texture coordinates
	quadsphere  * pqs;
	quadsphere::projection proj;
	bool QS_BUF;	// put quadsphere data in OGL buffers
};

#endif //ndef PVQTVIEW_H
