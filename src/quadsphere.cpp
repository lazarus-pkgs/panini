/* quadsphere.cpp	for pvQt 04 Nov 2008 TKS

  A sphere tessellation with texture coordinates,
  in tabular form usable by OpenGl
  Note the OGL coordinate system is left handed:
	X right, Y up, Z toward eye
  PvQt puts the picture center at +Z, and loads
  Y-reversed images, so these texture coordinates 
  have X left, Y down (Z toward).

*/

#include	"quadsphere.h"
#include	<cmath>

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


quadsphere::quadsphere( int divs ){
// allocate memory 
	int n = divs + 1;
	ppf = n * n;	// points per face
	qpf = divs * divs;	// quads per face
	lpf = 4 * ( qpf + 2 * divs ); // line ids per face
	int vwpf  =  3 * ppf,	// vertex words/face
		twpf = 2 * ppf;		// tcoord words/face
	nwords = 6 * ( vwpf + 5 * twpf ) + 4 * qpf + lpf;
	words = new float[ nwords ];
// set pointers to arrays
	vertptrs[0] = words;
	rectptrs[0] = vertptrs[0] + 6 * vwpf;
	fishptrs[0] = rectptrs[0] + 6 * twpf;
	cyliptrs[0] = fishptrs[0] + 6 * twpf;
	equiptrs[0] = cyliptrs[0] + 6 * twpf;
	anglptrs[0] = equiptrs[0] + 6 * twpf;
	quadidx = (int *)(anglptrs[0] + 6 * twpf);
	lineidx = quadidx + 4 * qpf;
	for( int i = 1; i < 6; i++ ){
		vertptrs[i] = vertptrs[0] + i * vwpf;
		rectptrs[i] = rectptrs[0] + i * twpf;
		fishptrs[i] = fishptrs[0] + i * twpf;
		cyliptrs[i] = cyliptrs[0] + i * twpf;
		equiptrs[i] = equiptrs[0] + i * twpf;
		anglptrs[i] = anglptrs[0] + i * twpf;
	}
/*  Index arrays: 4 linear indices per quad.  
		Line indices link ul-ll, ul-ur
		Quad indices link ul-ll-lr-ur
*/
	int il = 0, iq = 0;
	for( int r = 0; r < divs; r++ ){
		int k = r * n;	// 1st index of row
		int c;
		for( c = 0; c < divs; c++ ){
		// top & left quad edges
			lineidx[il++] = k + c;		// ul
			lineidx[il++] = n + k + c;	// ll
			lineidx[il++] = k + c;
			lineidx[il++] = k + c + 1;	// ur
		// CCW quad
			quadidx[iq++] = k + c;
			quadidx[iq++] = n + k + c;
			quadidx[iq++] = n + k + c + 1;
			quadidx[iq++] = k + c + 1;
		}
	  // add right edge line
		lineidx[il++] = k + c;
		lineidx[il++] = n + k + c;
	}
  // add bottom edge lines
	int k = divs * n;
	for( int c = 0; c < divs; c++ ){
		lineidx[il++] = k + c;
		lineidx[il++] = k + c + 1;
	}
		
/* Vertex arrays
	First face is computed from cube corners using slerp;
	rest are rotations of 1st by multiples of 90 degrees.
*/
  // cube corner coordinates for front face
	const double ccc = sqrt( 1.0 / 3.0 );
	double	vul[3] = {-ccc, ccc, ccc },	// upper left
			vur[3] = { ccc, ccc, ccc },	// upper rigt
			vll[3] = {-ccc, -ccc, ccc },	// lower left
			vlr[3] = {ccc, -ccc, ccc };	// lower right
  /* compute first face row-by row with a double slerp.
    The outer one, done here, interpolates row end
	points at double precision, then slerp() computes
	the rows at float precision.
  */
	int i;
	double om  = dotp( vul, vll ); // cos(angle between ends)
	double som = sqrt( 1.0 - om * om );	// sin ditto
	om = asin( som );		// the angle
	float * pf = vertptrs[0];
	double s = 1.0 / divs ;	// the t step
	for( i = 0; i <= divs; i++ ){
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
		slerp( divs, v0, v1, pf, 1 ); // fill row
		pf += 3 * n;
	}
  /* generate the other faces with 90 degree rotations 
	 which are basically just coordinate swaps and negations;
	 but note that "negative" faces are also row-reversed to 
	 preserve index hand.
  */
	float * ps, *pd;
  // right (+X) face
	ps = vertptrs[0];
	pd = vertptrs[1];	
	for( i = 0; i < ppf; i++ ){
		pd[0] = ps[2];	// x = z
		pd[1] = ps[1];	// y = y
		pd[2] = -ps[0];	// z = -x
		ps += 3; pd += 3;
	}
  // top (+Y) face
	ps = vertptrs[0];
	pd = vertptrs[4];	
	for( i = 0; i < ppf; i++ ){
		pd[0] = ps[0];
		pd[1] =	ps[2];
		pd[2] = -ps[1];
		ps += 3; pd += 3;
	}
  // back (-Z) face
	ps = vertptrs[0];
	pd = vertptrs[2];	
	for( i = 0; i < n; i++ ){
		float * p = pd + 3 * (n - 1);
		pd += 3 * n;
		for( int j = 0; j < n; j++ ){
			p[0] = ps[0];
			p[1] = ps[1];
			p[2] = -ps[2];
			ps += 3; p -= 3;
		}
	}
  // left (-X) face
	ps = vertptrs[1];
	pd = vertptrs[3];	
	for( i = 0; i < n; i++ ){
		float * p = pd + 3 * (n - 1);
		pd += 3 * n;
		for( int j = 0; j < n; j++ ){
			p[0] = -ps[0];
			p[1] = ps[1];
			p[2] = ps[2];
			ps += 3; p -= 3;
		}
	}
  // bottom (-Y) face
	ps = vertptrs[4];
	pd = vertptrs[5];	
	for( i = 0; i < n; i++ ){
		float * p = pd + 3 * (n - 1);
		pd += 3 * n;
		for( int j = 0; j < n; j++ ){
			p[0] = ps[0];
			p[1] = -ps[1];
			p[2] = ps[2];
			ps += 3; p -= 3;
		}
	}

  /* Texture coordinates [0:1]
	are generated from the sphere points one by one
	(which is inefficient but simple).

	NOTE max rectilinear FOV is the max vertical
	view angle allowed by pvQtView
  */
// rectlinear fov params
	const double amaxrect = DEG2RAD(137.5 / 2.0);
	const double trect = tan( amaxrect );
 
	for( int face = 0; face < 6; face++ ){
		ps = vertptrs[face];
		float * pr = rectptrs[face],
			  * pf = fishptrs[face],
			  * pc = cyliptrs[face],
			  * pe = equiptrs[face],
			  * pa = anglptrs[face];
		for( i = 0; i < ppf; i++ ){
		  // angles from the axes [0:Pi]
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
			if( za > 0.45 * Pi ) pr[0] = pr[1] = -1; // quickie
			else {
			  s = 0.5 * tan( za ) / trect;
			  double x = 0.5 + s * sx, 
				     y = 0.5 + s * sy;
			  double t = 0.02;
			  if( x < -t || y < -t || x > 1 + t || y > 1 + t ){
				  pr[0] = pr[1] = -1;
			  } else {
				  pr[0] = float( x );
				  pr[1] = float( y );
			  }
			}

		  // fish/mirrorball sphere
			if( sx == 0 && sy == 0 && za > 0.5 * Pi ){
				pf[0] = pf[1] = -1;
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
			if( fabs( s ) > amaxrect ) pc[0] = pc[1] = -1;
			else {
				pc[0] = pe[0];
				pc[1] = float(0.5 + 0.5 * tan(s) / trect );
			}


		  // equiangular sphere
			if( sx == 0 && sy == 0 && za > 0.5 * Pi ){
				pa[0] = pa[1] = -1;
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
}

quadsphere::~quadsphere(){
	if( words ) delete[] words;
}

const int * quadsphere::lineIndices( int face ){
	if( face < 0 || face > 5 ) return 0;
	return lineidx;
}

const int * quadsphere::quadIndices( int face ){
	if( face < 0 || face > 5 ) return 0;
	return quadidx;
}

const float * quadsphere::vertices( int face ){
	if( face < 0 || face > 5 ) return 0;
	return vertptrs[face];
}

const float * quadsphere::texCoords( int face, projection proj ){
	if( face < 0 || face > 5 ) return 0;
	switch( proj ){
	default: return 0;
	case eqa: return anglptrs[face];
	case rec: return rectptrs[face];
	case sph: return fishptrs[face];
	case cyl: return cyliptrs[face];
	case eqr: return equiptrs[face];
	}
}

