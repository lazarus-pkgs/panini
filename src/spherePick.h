#ifndef __SPHEREPICK_H__
#define __SPHEREPICK_H__

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

bool mouse2sphere( int mx, int my, double sph[3] );
bool sphere2screen( double sph[3], double & sx, double & sy);

#endif // __SPHEREPICK_H__
