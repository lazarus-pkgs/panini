/* quadsphere.cpp	for pvQt 14 Nov 2008 (3rd version)

  A sphere tessellation with texture coordinates,
  line and quad incices, in arrays usable by OpenGl

  Note the OGL coordinate system is left handed:
	X right, Y up, Z toward eye
  PvQt puts the picture center at +Z, and loads
  Y-reversed images, so these texture coordinates 
  have X left, Y down (Z toward).
 *
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

#include	"quadsphere.h"
#include	<cmath>
#include	<cstdio>

#ifndef Pi
#define Pi 3.1415926535897932384626433832795
#define DEG2RAD( x ) (( x ) * Pi / 180.0)
#define RAD2DEG( x ) (( x ) * 180.0 / Pi)
#endif 

static inline double dotp( double a[3], double b[3] ){
	return a[0] * b[0] + a[1] * b[1] + a[2]  * b[2];
}

/* split the great circle between 2 unit vectors into divs
  intervals, storing results in a general array (divs + 1 
  results in all,  including the end points).  Results and
  stride are in float 3-vectors.
	
*/
static void slerp( int divs, double v0[3], double v1[3],
				  float * pf, int stride )
{
	if( divs < 1 ) return;
	double om  = dotp( v0, v1 ); // cos(angle between ends)
	double som = sqrt( 1.0 - om * om );	// sin ditto
	om = asin( som );		// the angle
	double s = 1.0 / divs ;	// the t step
	for( int i = 0; i <= divs; i++ ){
		double t = i * s;
		double a = sin(om*(1.0 - t)) / som,
			   b = sin(om*t) / som;
		pf[0] = float(v0[0] * a + v1[0] * b);
		pf[1] = float(v0[1] * a + v1[1] * b);
		pf[2] = float(v0[2] * a + v1[2] * b);	
		pf += 3 * stride;
	}
}

// debug error messages can be put here
static char msg[80];


/*  Abandoning a long hard struggle to put together a
  usable sphere from the mimimum number of data points,
  I've decided in this third version to keep it simple,
  stupid.  So here there are 6 full cube faces, 3 of
  which are split up the middle to span the wraparound
  line.  The redundant edges add 12 * (divs + 1) points,
  but make it possible to draw the sphere with code I
  can write.

  The faces are stored in unform blocks as follows:
	0:	+Z (front)		3:	-Z (back)
	1:	+y (top)		4:	-Y (bottom)
	2:	+X (left)		5:	-X (right)
  followed by the duplicate points that split faces
  1, 3, and 4.  The vertices and texture coordinate
  sets are stored separately in this format.

  The uniform face layout makes it easy to index the 
  vertices and texture coordinates to create lines
  and quads.

  The faces are oriented as pvQt sees them -- that is, 
  from the inside. Since the OGL coordinate system is 
  left handed, the +Z face has +X left and +Y up, and 
  so forth.  This makes all quad indices run CCW.

*/

quadsphere::quadsphere( int divs ){

// no error
	errmsg = 0;	
// make sure divs is even, or 1 (cube only)
	divs = 2 * ((divs + 1) / 2);
	if( divs < 1 ) divs = 1;
/* allocate memory 
*/
	int dm1 = divs - 1;	// interior points per row
	int dp1 = divs + 1;	// total points per row
	int qpf = divs * divs;	// quads per face
	int ppf = dp1 * dp1;	// points per face

	vertpnts = 6 * ppf + 3 * dp1;	// total vertices
	linewrds = 24 * qpf;
	quadwrds = linewrds;
	nwords =  15 * vertpnts + linewrds + quadwrds;
	words = new float[ nwords ];
	if( words == 0 ){
		errmsg = "insufficient memory";
		return;	
	}
// set pointers to arrays
	verts = words;
	angls = verts + 3 * vertpnts;
	rects = angls + 2 * vertpnts;
	fishs = rects + 2 * vertpnts;
	cylis = fishs + 2 * vertpnts;
	equis = cylis + 2 * vertpnts;
	lineidx = (unsigned int *)(equis + 2 * vertpnts);
	quadidx = lineidx + linewrds;


/* Build Vertex arrays
	+Z face is computed from cube corners using slerp;
	rest are rotations of that by multiples of 90 degrees.
*/
  // cube corner coordinates for front face
	const double ccc = sqrt( 1.0 / 3.0 );
	double	vul[3] = { ccc, ccc, ccc },	// upper left
			vur[3] = { -ccc, ccc, ccc },	// upper rigt
			vll[3] = { ccc, -ccc, ccc },	// lower left
			vlr[3] = { -ccc, -ccc, ccc };	// lower right

  /* compute front face row-by row with a double slerp.
    The outer one, done here, interpolates row endpoints 
	at double precision, then slerp() computes the rows 
	at float precision.
	Note posts the full face including all edge points.
  */
	float * pd = verts;	// running output addr
	int nr = divs + 1;	// row length for 1st 4 faces
	unsigned int vcnt = 0;	// debug check
	double om  = dotp( vul, vll ); // cos(angle between ends)
	double som = sqrt( 1.0 - om * om );	// sin ditto
	om = asin( som );		// the angle
	double s = 1.0 / divs ;	// the t step
	for(int i = 0; i <= divs; i++ ){
		double v0[3], v1[3];
		double t = i * s;
		double a = sin(om*(1.0 - t)) / som,
			   b = sin(om*t) / som;
		v0[0] = vul[0] * a + vll[0] * b;
		v0[1] = vul[1] * a + vll[1] * b;
		v0[2] = vul[2] * a + vll[2] * b;	
		v1[0] = vur[0] * a + vlr[0] * b;
		v1[1] = vur[1] * a + vlr[1] * b;
		v1[2] = vur[2] * a + vlr[2] * b;	
		slerp( divs, v0, v1, pd, 1 ); // fill row
		pd += 3 * nr;
		vcnt += divs + 1;
	}

  /* remaining faces 
    Note this code depends on the symmetry of the
	faces around their center points.
  */
	unsigned int jf = 3 * ppf;	// words per face
	float * ps = verts;	// -> front face
	for( int i = 0; i < ppf; i++ ){
		register float * p = ps;

		p +=  jf;	// ->top
		p[0] = ps[0];	//  x = x
		p[1] = ps[2];	//  y = z
		p[2] = -ps[1];	//  z = -y
		p += jf;	// -> left
		p[0] = ps[2];	//  x = z
		p[1] = ps[1];	//  y = y
		p[2] = -ps[0];	//  z = -x
		p += jf;	// -> back
		p[0] = -ps[0];	//  x = -x
		p[1] = ps[1];	//  y = y
		p[2] = -ps[2];	//  z = -z
		p += jf;	// -> bottom
		p[0] = ps[0];	//  x = x
		p[1] = -ps[2];	//  y = -z
		p[2] = ps[1];	//  z = y
		p += jf;	// -> right
		p[0] = -ps[2];	//  x = -z
		p[1] = ps[1];	//  y = y
		p[2] = ps[0];	//  z = x

		ps += 3;
	}


  /* quad indices
	same pattern each face		0	3
								1	2
	generate one face then copy with offset
  */
	unsigned int * pq = quadidx;
	int r, c;
	for( r = 0; r < divs; r++ ){
		unsigned int k = r * dp1;	// 1st index of row
		for( int c = 0; c < divs; c++ ){
		// CCW quad
			*pq++ = k + c;
			*pq++ = dp1 + k + c;
			*pq++ = dp1 + k + c + 1;
			*pq++ = k + c + 1;
		}
	}

	unsigned int * qq = quadidx;
	for( r = 20 * qpf; r > 0; --r ){
		*pq++ = *qq++ + ppf;
	}
 
  /* line indices
	copy quad indices, replacing one index of each quad
	with a copy of another.  Only 2 edges of each quad 
	are drawn; the doubled index selects which ones.  The
	doubled corners are chosen so that all 12 cube edges
	get drawn: F, T, R : 0 => 2; K, L: 1 => 3; B: 3 => 1
  */
	qq = quadidx;
	pq = lineidx;
	// front
	for( r = qpf; r > 0; --r ){
		pq[0] = qq[0];
		pq[1] = qq[1];
		pq[2] = qq[0];
		pq[3] = qq[3];
		pq += 4;
		qq += 4;
	}
	// top
	for( r = qpf; r > 0; --r ){
		pq[0] = qq[0];
		pq[1] = qq[1];
		pq[2] = qq[0];
		pq[3] = qq[3];
		pq += 4;
		qq += 4;
	}
	// left
	for( r = qpf; r > 0; --r ){
		pq[0] = qq[0];
		pq[1] = qq[1];
		pq[2] = qq[2];
		pq[3] = qq[1];
		pq += 4;
		qq += 4;
	}
	// back
	for( r = qpf; r > 0; --r ){
		pq[0] = qq[0];
		pq[1] = qq[1];
		pq[2] = qq[2];
		pq[3] = qq[1];
		pq += 4;
		qq += 4;
	}
	// bottom
	for( r = qpf; r > 0; --r ){
		pq[0] = qq[0];
		pq[1] = qq[3];
		pq[2] = qq[2];
		pq[3] = qq[3];
		pq += 4;
		qq += 4;
	}
	// right
	for( r = qpf; r > 0; --r ){
		pq[0] = qq[0];
		pq[1] = qq[1];
		pq[2] = qq[0];
		pq[3] = qq[3];
		pq += 4;
		qq += 4;
	}
	

  /* Texture coordinates 
	are generated from the sphere points one by one.
	
	The valid range of TCs is [0:1]; invalid points
	get TCs just slightly outside that range.

	NOTE max rectilinear FOV is the max vertical
	view angle allowed by pvQtView
  */
// rectlinear fov params
	const double amaxrect = DEG2RAD(137.5 / 2.0);
	const double trect = tan( amaxrect );

const float ninv = -0.1, pinv = 1.1;
#define CLIP( x ) ( x < ninv ? ninv : x > pinv ? pinv : x )
#define INVAL( t ) ((t > 0) ? pinv : ninv )

	ps = verts;
	float * pr = rects,
		  * pf = fishs,
		  * pc = cylis,
		  * pe = equis,
		  * pa = angls;
  // loop over all vertices
	for( r = 6 * ppf; r > 0; --r ){
	  /* angles from the axes
	    xa is in [-Pi:Pi], ya and za in[0:Pi]
	  */
		double xa = -atan2(ps[0], ps[2]), // horiz from +Z
			   ya = acos(ps[1]), // vert from -Y
			   za = acos(ps[2]);  // radial from +Z
	  // direction for radial fns (0: invalid)
		s = sqrt(ps[0]*ps[0] + ps[1]*ps[1]);
		double sx = 0, sy = 0;
		if( s >= 1.0e-4 ){
			sx = -ps[0] / s;
			sy = -ps[1] / s;
		}

	  // rectilinear
		if( za > 0.45 * Pi ){
			pr[0] = INVAL( sx );
			pr[1] = INVAL( sy );
		} else {
		  s = 0.5 * tan( za ) / trect;
		  double x = CLIP( 0.5 + s * sx ), 
			     y = CLIP( 0.5 + s * sy );
		  pr[0] = float( x );
		  pr[1] = float( y );
		}

	  // fisheye
		if( sx == 0 && sy == 0 && za > 0.5 * Pi ){
			pf[0] = INVAL( xa );
			pf[1] = INVAL( ya - 0.5 * Pi );
		} else {
			s = 0.5 * sin( 0.5 * za );
			pf[0] = float(0.5 + s * sx );
			pf[1] = float(0.5 + s * sy );
		}

	  // equirectangular
		pe[0] = float( 0.5 + 0.5 * xa / Pi );
		pe[1] = float(ya / Pi);

	  // cylindrical
		s = ya - 0.5 * Pi;
		if( fabs( s ) > amaxrect ){
			pc[0] = INVAL( xa );
			pc[1] = INVAL( s );
		} else {
			pc[0] = pe[0];
			pc[1] = float(0.5 + 0.5 * tan(s) / trect );
		}


	  // equiangular sphere
		if( sx == 0 && sy == 0 && za > 0.5 * Pi ){
			pa[0] = INVAL( xa );
			pa[1] = INVAL( ya - 0.5 * Pi );
		} else {
			s = 0.5 * za / Pi;
			pa[0] = float(0.5 + s * sx );
			pa[1] = float(0.5 + s * sy );
		}

	  // next point
		ps += 3; 
		pr += 2; pf += 2; pc += 2; pe += 2; pa += 2;
	}


}

quadsphere::~quadsphere(){
	if( words ) delete[] words;
}

const float * quadsphere::texCoords( projection proj ){
	switch( proj ){
	default: return 0;
	case eqa: return angls;
	case rec: return rects;
	case sph: return fishs;
	case cyl: return cylis;
	case eqr: return equis;
	}
}

unsigned int quadsphere::texCoordOffset( projection proj ){
	const float * p = texCoords( proj );
	if( p == 0 ) p = angls;
	return (char *)p - (char *)verts;
}

