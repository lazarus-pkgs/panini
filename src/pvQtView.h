/*
 * pvQtView.h for pvQt 11 Sept 2008
 * Copyright (C) 2008 Thomas K Sharpless
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include "panosphere.h"
#include "panocylinder.h"

class pvQtView : public QGLWidget
{
    Q_OBJECT
public:
    pvQtView(QWidget *parent = 0);
    ~pvQtView();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    /**  OpenGL attributes  **/

    bool OpenGLOK(); // false if OGL version too low

    QString OpenGLVersion(){
        return QString( (const char *)glGetString(GL_VERSION) );
    }
    QString OpenGLVendor(){
        return QString( (const char *)glGetString(GL_VENDOR) );
    }
    QString OpenGLHardware(){
        return QString( (const char *)glGetString(GL_RENDERER) );
    }
    QString OpenGLLimits(){
        return QString("texPwr2 %1, texMax %2, cubeMax %3")
                .arg(texPwr2).arg(max2d).arg(maxcube);
    }

    /*
    Display an overlay image
    ovl = 0 stops overlay display
    Image format should be ARGB.  Size should be moderate.
    Vertical dimension will fill the viewport.
    */
    bool showOverlay( QImage * ovl );

    /*
    Display a picture
    pic = 0 resets to base screen display.  Otherwise
    *pic must be valid and undisturbed until showPic is next
    called (caller can then delete pic if appropriate).
    returns sucess or failure, with errmsg updated.
    */
    bool showPic( pvQtPic * pic );
    /*
    check whether picture displayed OK, get message if not
    The error flag is reset when a new picture is loaded.
    */
    bool picOK( QString & errMsg ){
        errMsg = errmsg;
        return picok;
    }

    /*
    Identify image face that contains a screen pixel
    pnt is in viewport mouse coordinates (ulc origin)
    returns front if pic type is not cubic
   */
    pvQtPic::PicFace pickFace( QPoint pnt );

    /*
    Save the current view to a file
    name is full pathname, with extension .jpg or .tif
    Default size is the current viewport size.  Custom sizes
    need not be the same shape as the viewport (but may
    not be supported on a given system -- if not, viewport
    size is used)
    Returns true if image was rendered and written OK.
    */
    bool saveView( QString name, QSize size = QSize());
    // overload to save possibly scaled-up copy of viewport
    // scale is clipped to [1.0:5.0]
    bool saveView( QString name, double scale = 1.0 );

    // get the current screen viewport size in pixels
    QSize screenSize(){ return QSize( Width, Height ); }

public slots:
    /*
    Angles passed from/to GUI are integers in 16ths of a degree,
    so they can be reliably checked for equality.  The zoom angle
    is the fov of the shorter window axis
    */
    void setPan(int iangle); // unit = 1/16 degree
    void setTilt(int iangle);
    void setSpin(int iangle);
    void setZoom( int iangle );
    void setDist( double dist );
    // keypress steps -- dp should normally be +/- 1
    void step_pan( int dp );
    void step_tilt( int dp );
    void step_zoom( int dp );
    void step_roll( int dp );
    void step_dist( int dp );
    void step_hfov( int dp );
    void step_vfov( int dp );
    void step_iproj( int dp );
    // preset views
    void reset_view(); // reinit all params
    void set_view( int v );  // 0: lin, 1: proto, 2: ortho
    void home_view(); // zero view angles
    void home_eyeXY(); // zero eyepoint shifts
    void super_fish(); // Ez = 1.07, min zoom
    // update display of current picture
    void picChanged(); // from scratch
    void newFace( pvQtPic::PicFace face ); // one cube face
    // select panosurface
    void setSurface( int surf );
    // orient image on panosurface turn(0:3)= 0,90,180,270 deg
    void setTurn( int turn, double roll, double pitch, double yaw );
    // Mac cube texture dimension limit
    void setCubeLimit( int lim );
    // V0.7
    void recenterMode( bool );
    void step_eyex( int );
    void step_eyey( int );


signals:
    void reportView( QString msg );
    void OGLerror( QString msg );
    void reportTurn( int turn, double roll, double pitch, double yaw );
    void reportFov( QSizeF fovs );
    void reportProj( QString name );
    void reportSurface( int surf );
    void reportRecenter( bool ); // when recenter changed internally
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *pme );
    void mouseMoveEvent(QMouseEvent *pme );
    void mouseReleaseEvent( QMouseEvent *pme );
    void mouseDoubleClickEvent( QMouseEvent *pme );
    void wheelEvent(QWheelEvent *event);

private slots:
    void mTimeout();
private:
    // GUI support
    double normalizeAngle(int &iangle, int istep, double lwr, double upr);
    int iAngle( double angle );	// real to integer coded angle
    void stepDangl( int dp, int stp );
    void showview();
    void setFOV( double fov = 0 );
    void initView();
    // current view parameters
    double hFOV, vFOV;	// angular size at sphere center (deg)
    double minFOV, maxFOV; // limits on vFOV
    double wFOV;  // vert angle at eye (deg) sets magnification
    double eyeDistance;	// of eye from origin, in sphere radii
    // working eye position, in radii
    double eyex, eyey, eyez;
    // framing shifts
    double framex, framey, // user controlled
    fcompx, fcompy,	// to compensate eye shifts
    framex0, framey0,
    fwf, fhf;

    int Width, Height; // screen pixel dimensions
    double portAR; // width/height
    double Znear, Zfar;	// clipping plane distances from eye
    double panAngle, tiltAngle, spinAngle; // degrees
    double minpan, maxpan, mintilt,maxtilt; //image limits
    // imge orientation on panosurface
    int turn90; // 0:3 90 deg steps
    double turnRoll; // fine -45:45 deg
    double turnPitch; // -90:90 deg
    double turnYaw;	// -180:180 deg

    int ipan, panstep;
    int itilt, tiltstep;
    int ispin, spinstep;
    int izoom, zoomstep;
    int idangl, danglstep;
    int ihangl, hanglstep;
    int ivangl, vanglstep;
    double hangle, vangle; // recenter eye dir
    int mx0, my0, mx1, my1; // mouse coordinates
    Qt::MouseButtons mb;
    Qt::KeyboardModifiers mk;
    QTimer mTimer;

    void setTexMag( double magx, double magy );

    // display support
    void setPicType( pvQtPic::PicType pt );
    bool setupPic( pvQtPic * pic );
    void updatePic();
    pvQtPic  * thePic;
    pvQtPic::PicType picType;
    int	ipicType; // index of picType
    // display lists for screens
    void makeSphere( GLuint list );
    GLuint theScreen; // current screen list
    // textures
    GLenum textgt; // current target (2D or cube)
    GLuint texname; // current texture object
    GLuint texnms[2]; // bound textures: 0: 2d, 1: cube
    // OpenGL capabilities
    bool OGLisOK; // is usable
    bool OGLv20; // is version 2.0 or better
    bool texPwr2; // needs power-of-2 texture dimensions
    bool cubeMap; // has cube mapping (rq'd)
    bool vertBuf; // has vertex buffers (opt)
    GLint maxcube, max2d; // OGL's texture dim limits
    QSize maxTex2Dsqr, // largest feasible texture dims
    maxTex2Drec,
    maxTexCube;
    QSize maxTexSize( GLenum proxy, int tw, int th );
    // status
    bool paintok; // most recent OGL error status
    bool picok; // sticky OGL error flag
    QString errmsg;	// sticky OGL error message
    bool OGLok(const char * label);	// check, post and signal OGL errors
    // tabulated sphere points and texture coordinates
    panosphere  * pqs;
    panocylinder * ppc;
    bool QS_BUF; // put quadsphere data in OGL buffers
    double xtexmag, ytexmag; // tex coord scale factors

    pictureTypes pictypes;
    QSizeF curr_fovs; // current
    pvQtPic::PicType curr_pt;
    int curr_ipt; // index of curr_pt
    QSizeF stdTexScale;

    int surface; // 0: sphere, 1: cylinder

    int MacCubeLimit;

    // pointer to overlay image
    QImage * povly;
    // for recenter mode
    bool recenter;
    void clipEyePosition();

};

#endif //ndef PVQTVIEW_H
