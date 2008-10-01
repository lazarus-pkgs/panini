#include "spherePick.h"
#include <QtOpenGL/QtOpenGL>
#include <GL/glut.h>
#include <cmath>

/*  Map screen coordinates <=> points on the panosphere
	according to current OpenGL display transformations.
	
	Screen coordinates are measured in pixels from the
	upper left.  [Strictly, the origin is at the center 
	of the upper left pixel, so the range of real screen 
	coordinates is from -1/2 to dimension + 1/2.]
	
	Panosphere points are unit 3-vectors.
	
	Return false with all zero result on any OpenGL error 
	or if view vector length is too close to zero.
*/

bool mouse2sphere( int mx, int my, double sph[3] )
{
    glLoadIdentity();
 
    GLdouble modelMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
    GLdouble projMatrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
  // convert to lower left origin and real values
    double sx = (double) mx  + 0.5,
           sy = (double)( viewport[3] - my ) + 0.5;
  // get near and far points on view ray
    sph[0] = sph[1] = sph[2] = 0;
    GLdouble pos0[3], pos1[3];
    GLint r;
    r = gluUnProject( sx, sy, 0.0,
         modelMatrix, projMatrix, viewport,
         &pos0[0], &pos0[1], &pos0[2]
    );
    if( r == GLU_FALSE ) return false;
    r = gluUnProject( sx, sy, 1.0,
         modelMatrix, projMatrix, viewport,
         &pos0[0], &pos0[1], &pos0[2]
    );
    if( r == GLU_FALSE ) return false;
  // vector in view direction
    pos1[0] -= pos0[0];
    pos1[1] -= pos0[1];
    pos1[2] -= pos0[2];
  // normalize giving result
    double s = pos1[0] * pos1[0]
    		 + pos1[1] * pos1[1]
    		 + pos1[2] * pos1[2];
    if( fabs(s) < 1.e-8 ) return false;
    else s = 1.0 / sqrt(s);
    sph[0] = s * pos1[0];
    sph[1] = s * pos1[1];
    sph[2] = s * pos1[2];
    
    return true;
}

bool sphere2screen( double sph[3], double & sx, double & sy  )
{
    glLoadIdentity();
 
    GLdouble modelMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
    GLdouble projMatrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

	sx = 0; sy = 0;
	GLdouble pos0[3];
    GLint r;
    r = gluProject( sph[0], sph[1], sph[2],
         modelMatrix, projMatrix, viewport,
         &pos0[0], &pos0[1], &pos0[2]
    );
    if( r == GLU_FALSE ) return false;
    
    sx = pos0[0];
    sy = (double)viewport[3] - pos0[1];
    return true;
}
