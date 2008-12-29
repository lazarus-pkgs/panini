/* panocylinder.cpp	for pvQt 11 Dec 2008

  A cylinder tessellation with texture coordinates,
  line and quad incices, in arrays usable by OpenGl.

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

#include	"panocylinder.h"
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

// debug error messages can be put here
static char msg[80];

/* texcoord utilities
*/
const float ninv = -0.01, pinv = 1.01;
#define CLIP( x ) ( x < ninv ? ninv : x > pinv ? pinv : x )
#define INVAL( t ) (t > 0 ? pinv : ninv )
#define EVAL( t ) ( t < 0.5 ? t < 0 ? t : 0 : t > 1 ? t : 1 ) 


/* 
  The cylinder is divided into quads of equal angular
  size (as seen from the center) so the heights increase 
  away from equator.  The width is 360 degrees and the
  full height is 150 degrees.

  The equatorial plane is XZ, with +Z at the center
  and +X at left; axis is Y, with +Y up.

  Quads are oriented CCW as seen from the inside. 

  Each level is a circle of constant Y, and the XZ
  coordinates are same at all levels.  So a very
  compact representation is possible; but I use 
  a full array for simplicity.  The vertical edge
  points are duplicated.
*/

panocylinder::panocylinder( int divs ){

// no error
	errmsg = 0;	
// make sure divs is even and > 3
	divs = 2 * ((divs + 1) / 2);
	if( divs < 4 ) divs = 4;
/* 
  The vertex and TC arrays have divs + 1 columns,
  and the smallest odd number of rows >= 0.5 * divs,
  for avg square quads at a vFov of 150 degrees.
  There are (rows - 1) * (cols -1) quads; each of the 
  index arrays has 4 entries per quad.
*/
	int cols = divs + 1;
	int rows = int( 0.5 * divs ); 
	if( (rows & 1) == 0 ) ++rows;  // odd no. of rows
	int quads = (rows - 1) * (cols - 1);
	vertpnts = rows * cols;	// total vertices
	linewrds = 4 * quads;
	quadwrds = linewrds;
	nwords =  (3 + 2 * Nprojections) * vertpnts + linewrds + quadwrds;
	words = new float[ nwords ];
	if( words == 0 ){
		errmsg = "insufficient memory";
		return;	
	}

// set pointers to arrays
	verts = words;
	TCs = verts + 3 * vertpnts;
	lineidx = (unsigned int *)(TCs + Nprojections * 2 * vertpnts);
	quadidx = lineidx + linewrds;

// local TC pntrs
	float *	rects = (float *)texCoords("rect");
	float * fishs = (float *)texCoords("fish");
	float * cylis = (float *)texCoords("cyli");
	float * equis = (float *)texCoords("equi");
	float * sters = (float *)texCoords("ster");
	float * mercs = (float *)texCoords("merc");
	float * angls = (float *)texCoords("sphr");

/* Build Vertex array
  All rows have same X and Z coordinates
  Array origin is lower left corner; X
  points left, Y up when look along Z.
*/
	int row, col;
	int r2 = rows / 2;
	float * pv0 = verts + 3 * r2 * cols;
  // center row
	double hs = 2 * Pi / double(cols - 1);
	float * pv = pv0;
	for( col = 0; col < cols; col++ ){
		double a = col * hs - Pi;
		*pv++ = float(sin( a ));	// X
		*pv++ = 0;					// Y
		*pv++ = float(cos( a ));	// Z
	}
  // rest of rows
	double vs = 0.5 * DEG2RAD( 150 ) / double(r2);
	for( int r = 0; r < r2; r++){
		register double t = tan( r * vs );
		register float * pv = pv0;
		register float * pu = pv0 + 3 *( cols + r * cols );
		register float * pl = pv0 - 3 *( cols + r * cols );
		for( col = 0; col < cols; col++){
			*pu++ = *pv;
			*pl++ = *pv++;
			*pu++ = t;
			*pl++ = -t;
			++pv;
			*pu++ = *pv;
			*pl++ = *pv++;
		}
	}


/* quad indices		0	3
					1	2
*/
	unsigned int * pq = quadidx;
	int r, c;
	for( r = 0; r < rows - 1; r++ ){
		unsigned int k = r * cols;	// 1st index of row
		for( int c = 0; c < cols - 1; c++ ){
		// CCW quad
			*pq++ = k + c;
			*pq++ = cols + k + c;
			*pq++ = cols + k + c + 1;
			*pq++ = k + c + 1;
		}
	}

  /* line indices
	copy quad indices, replacing one index of each quad
	with a copy of another.  Only 2 edges of each quad 
	are drawn; the doubled index selects which ones. 
  */
	unsigned int * qq = quadidx;
	pq = lineidx;
	for( r = quads; r > 0; --r ){
		pq[0] = qq[0];
		pq[1] = qq[1];
		pq[2] = qq[0];
		pq[3] = qq[3];
		pq += 4;
		qq += 4;
	}

  /* Texture coordinates 
	are generated from the known angular spacing of
	the vertices.
	
	The valid range of TCs is [0:1]; invalid points
	get TCs just slightly outside that range.

	Max fovs are read from pictureTypes
  */
// set angular limts from fovs in pictureTypes
	QSizeF fovs;
	fovs = pictypes.maxFov( pictypes.picTypeIndex( "rect" ) );
	double amaxrect = DEG2RAD(0.5 * fovs.width());
	double cminrect = cos( amaxrect );
	double tmaxrect = tan( amaxrect );

	fovs = pictypes.maxFov( pictypes.picTypeIndex( "cyli" ) );
	double amaxcyli = DEG2RAD( 0.5 * fovs.height() );
	double tmaxcyli = tan( amaxcyli );

	fovs = pictypes.maxFov( pictypes.picTypeIndex( "sphr" ) );
	double amaxsphr = DEG2RAD( 0.5 * fovs.width() );


	fovs = pictypes.maxFov( pictypes.picTypeIndex( "fish" ) );
	double amaxfish = DEG2RAD( 0.5 * fovs.width() );

	fovs = pictypes.maxFov( pictypes.picTypeIndex( "merc" ) );
	double amaxmerc = DEG2RAD( 0.5 * fovs.height() );
	double smaxmerc = sin( amaxmerc );
	double tmaxmerc = asinh(tan(amaxmerc));

	fovs = pictypes.maxFov( pictypes.picTypeIndex( "ster" ) );
	double amaxster = DEG2RAD( 0.5 * fovs.width() );
	double tmaxster = tan( 0.5 * amaxster );

	float * ps = verts;
	float * pr = rects,
		  * pf = fishs,
		  * pc = cylis,
		  * pe = equis,
		  * pa = angls,
		  * pm = mercs,
		  * pt = sters;

  // loop over all vertices
	for(int r = vertpnts; r > 0; --r ){
  /*
	x,y,z point on panosphere in direction of vertex  
    xa is angle from +Z in XZ plane [-Pi:Pi],
	ya is angle out of XZ plane [-Pi/2 : Pi/2],
	za is angle away from +Z [0:Pi]
    Viewing 0->+Z, X left, Y up
  */
	// unit direction vector
		double s = 1.0 / sqrt(ps[0]*ps[0] + ps[1]*ps[1] + ps[2]*ps[2]);
		double x = s * ps[0],
			   y = s * ps[1],
			   z = s * ps[2];
	// important angles, their sines and cosines
		double xa = -atan2(x, z), // horiz from +Z
			   ya = acos(y), // vert from -Y
			   za = acos(z);  // radial from +Z
		double  sxa = -x,	// sin(xa)
				cxa = z,	// cos(xa)
				sya = y,	// sin(ya)
				cya = sqrt( x * x + z * z ),
				sza = sqrt( x * x + y * y ),
				cza = z;
	  // direction vector for radial fns (0: invalid)
		double sx = 0, sy = 0;
		if( sza >= 1.0e-4 ){
			sx = sxa / sza;
			sy = -sya / sza;
		}

	  // rectilinear
		if( za > 0.45 * Pi ){
			pr[0] = INVAL( sx );
			pr[1] = INVAL( sy );
		} else {
		  s = 0.5 * (sza/cza) / tmaxrect;
		  double x = CLIP( 0.5 + s * sx ), 
			     y = CLIP( 0.5 + s * sy );
		  pr[0] = float( x );
		  pr[1] = float( y );
		}

	  // fisheye
		if( za > amaxfish ){
			pf[0] = INVAL( xa );
			pf[1] = INVAL( ya - 0.5 * Pi );
		} else {
			s = 0.5 * sqrt(0.5 * ( 1 - cza ));
			pf[0] = float(CLIP(0.5 + s * sx) );
			pf[1] = float(CLIP(0.5 + s * sy) );
		}

	  // equirectangular
		pe[0] = float( CLIP(0.5 + 0.5 * xa / Pi));
		pe[1] = float( CLIP(ya / Pi));

	  // cylindrical
		s = ya - 0.5 * Pi;
		if( fabs(s) > amaxcyli ){
			pc[0] = INVAL( xa );
			pc[1] = INVAL( s );
		} else {
			pc[0] = pe[0];
			pc[1] = float(CLIP(0.5 - 0.5 * (sya/cya) / tmaxcyli));
		}

	  // equiangular sphere
		if( za > amaxsphr ){
			pa[0] = INVAL( xa );
			pa[1] = INVAL( ya - 0.5 * Pi );
		} else {
			s = 0.5 * za / Pi;
			pa[0] = float( CLIP(0.5 + s * sx) );
			pa[1] = float( CLIP(0.5 + s * sy) );
		}

	  // mercator
		pm[0] = pe[0];
		s = ya - 0.5 * Pi;
		if( fabs(sya) > smaxmerc ) pm[1] = INVAL( s );
		else {
			s = asinh(sya / cya);
			pm[1] = float(CLIP( 0.5 -  s / tmaxmerc ));
		}

	  // stereographic
		if( za > amaxster ){
			pt[0] = INVAL( sx );
			pt[1] = INVAL( sy );
		} else {
			s = tan( 0.5 * za ) / tmaxster;
			pt[0] = float(CLIP(0.5 + s * sx));
			pt[1] = float(CLIP(0.5 + s * sy));
		}


	  // next point
		ps += 3; 
		pr += 2; pf += 2; pc += 2; pe += 2; 
		pa += 2; pm += 2; pt += 2;
	}

  /* fix up the +/- 180 wrap */
	pc = cylis;
	pe = equis;
	pm = mercs;
	r = 2 * cols;
	c = r - 2;
	for( row = 0; row < rows; row++ ){
		pc[0] = 1; pc[c] = 0; pc += r;
		pe[0] = 1; pe[c] = 0; pe += r;
		pm[0] = 1; pm[c] = 0; pm += r;
	}
}

panocylinder::~panocylinder(){
	if( words ) delete[] words;
}

const float * panocylinder::texCoords( const char * proj ){
	int i = pictypes.picTypeIndex( proj );
	if( i < 0 || i >= Nprojections ) return 0;
	return TCs + i * 2 * vertpnts;
}

unsigned int panocylinder::texCoordOffset( const char * proj ){
	const float * p = texCoords( proj );
	if( p == 0 ) return 0;
	return (char *)p - (char *)verts;
}

const float * panocylinder::texCoords( pvQtPic::PicType proj ){
	int i = pictypes.picTypeIndex( proj );
	if( i < 0 || i >= Nprojections ) return 0;
	return TCs + i * 2 * vertpnts;	
}

unsigned int panocylinder::texCoordOffset( pvQtPic::PicType proj ){
	const float * p = texCoords( proj );
	if( p == 0 ) return 0;
	return (char *)p - (char *)verts;
}

