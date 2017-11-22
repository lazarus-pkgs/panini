/*
 * panosphere.cpp	for Panini 29 Jan 2009

  A sphere tessellation with texture coordinates,
  line and quad incices, in arrays usable by OpenGl

  Note the OGL coordinate system is left handed:
    X right, Y up, Z toward eye
  PvQt puts the picture center at +Z, and loads
  Y-reversed images, so these texture coordinates
  have X left, Y down (Z toward).
 *
 * Copyright (C) 2008-2009 Thomas K Sharpless
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
*/

#define PANOSURFACE_IMPLEMENTATION
#include	"panosphere.h"

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
    if( divs < 1 ) {
        return;
    }
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

inline void setEdgeTCs( float * base, unsigned int ic, unsigned int id ){
    base[ic] = EVAL( base[ic - 2]);
    base[id] = EVAL( base[ic + 2]);
    base[id + 1] = base[ic + 1];
}

inline void copyTCs( float * base, unsigned int ic, unsigned int id ){
    base[id] = base[ic];
    base[id + 1] = base[ic + 1];
}

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

panosphere::panosphere( int divs ){

    // make sure divs is even, or 1 (cube only)
    divs = 2 * ((divs + 1) / 2);

    if( divs < 1 ) {
        divs = 1;
    }

    /* calculate memory reauirements*/
    int dp1 = divs + 1;	// total points per row
    int qpf = divs * divs;	// quads per face
    int ppf = dp1 * dp1;	// points per face
    int d2 = divs/2;
    int ntb = d2;		// top/bottom seam pnts to copy
    // top/bot centers handled specially, 2 xtra pnts each
    int dups = dp1 + 2 * ntb + 4;	// extra pnts for seam fix

    vertpnts = 6 * ppf + dups;	// total vertices
    linewrds = 24 * qpf;
    quadwrds = linewrds;

    // get memory or die
    if( ! getMemory() ) {
        return;
    }

    // local TC pntrs
    float *	rects = (float *)texCoords("rect");
    float * fishs = (float *)texCoords("fish");
    float * cylis = (float *)texCoords("cyli");
    float * equis = (float *)texCoords("equi");
    float * sters = (float *)texCoords("ster");
    float * mercs = (float *)texCoords("merc");
    float * angls = (float *)texCoords("sphr");

/*
 * Build Vertex arrays
   +Z face is computed from cube corners using slerp;
    rest are rotations of that by multiples of 90 degrees.
*/

    // cube corner coordinates for front face
    const double ccc = sqrt( 1.0 / 3.0 );
    double	vul[3] = { ccc, ccc, ccc },	// upper left
            vur[3] = { -ccc, ccc, ccc },	// upper rigt
            vll[3] = { ccc, -ccc, ccc },	// lower left
            vlr[3] = { -ccc, -ccc, ccc };	// lower right

/*
 * compute front face row-by row with a double slerp.
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

/*
 * remaining faces
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

/*
 * quad indices
    same pattern each face		0	3
                                1	2
    generate one face then copy with offset
*/

    unsigned int * pq = quadidx;
    int r;

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

/*
 * line indices
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

    /* generate texture coordinates for all projections  */
    map_projections();

    /* Fix up the wrap seam
    The wrap seam runs from top center thru bottom center
    down the middle of back, including top and bottom center
    points.  There are 2 *divs + 3 points in all because the
    cube edge points are already doubled.

    These vertices are copied to the dupes area and the indices
    for their right adjacent quads adjusted in-place.  Then the
    corresponding TCs -- both old and new -- are set low or high
    according to the values of the other TC's in their quads.

    Finally the top and bottom centers are split vertically by
    assigning a new split center point to their lower quads.
  */

    // point and quad indices
    int qtb = ntb - 1;	// whole quads top/bottom
    unsigned int ipt = ppf + d2,		// top rear
            iqt = qpf + d2,
            ipk = 3 * ppf + d2,	// back upper
            iqk = 3 * qpf + d2,
            ipb = 4 * ppf + (dp1 - ntb) * dp1 + d2,
            iqb = 4 * qpf + (divs - 1 - qtb) * divs + d2,
            idt = 6 * ppf,		// dupe top
            idk = idt + ntb,	// dupe back
            idb = idk + dp1,	// dupe bottom
            idc = idb + ntb;	// dupe centers
    // copy the vertices in ascending address order
    // and post corresponding correct TCs (old and new)
    unsigned int ic = 2 * ipt, id = 2 * idt;
    ps = verts + 3 * ipt;
    pd = verts + 3 * idt;

    for( r = 0; r < ntb; r++ ){
        pd[0] = ps[0];
        pd[1] = ps[1];
        pd[2] = ps[2];
        ps += 3 * dp1;
        pd += 3;

        copyTCs( angls, ic, id );
        setEdgeTCs( rects, ic, id );
        copyTCs( fishs, ic, id );
        setEdgeTCs( cylis, ic, id );
        setEdgeTCs( equis, ic, id );
        copyTCs( sters, ic, id );
        setEdgeTCs( mercs, ic, id );
        ic += 2 * dp1;
        id += 2;
    }

    ic = 2 * ipk;
    id = 2 * idk;
    ps = verts + 3 * ipk;

    for( r = 0; r < dp1; r++ ){
        pd[0] = ps[0];
        pd[1] = ps[1];
        pd[2] = ps[2];
        ps += 3 * dp1;
        pd += 3;
        copyTCs( angls, ic, id );
        setEdgeTCs( rects, ic, id );
        copyTCs( fishs, ic, id );
        setEdgeTCs( cylis, ic, id );
        setEdgeTCs( equis, ic, id );
        copyTCs( sters, ic, id );
        setEdgeTCs( mercs, ic, id );
        ic += 2 * dp1;
        id += 2;
    }

    ic = 2 * ipb;
    id = 2 * idb;
    ps = verts + 3 * ipb;

    for( r = 0; r < ntb; r++ ){
        pd[0] = ps[0];
        pd[1] = ps[1];
        pd[2] = ps[2];
        ps += 3 * dp1;
        pd += 3;
        copyTCs( angls, ic, id );
        setEdgeTCs( rects, ic, id );
        copyTCs( fishs, ic, id );
        setEdgeTCs( cylis, ic, id );
        setEdgeTCs( equis, ic, id );
        copyTCs( sters, ic, id );
        setEdgeTCs( mercs, ic, id );
        ic += 2 * dp1;
        id += 2;
    }

/*
 * change right quad indices of non-center pnts.
   The first and last points of each seam get 1
   new index, the others get 2.
*/

    pq = quadidx + 4 * iqt;
    for( r = 0; r < qtb; r++ ){
        pq[0] = idt++;
        pq[1] = idt;
        pq += 4 * divs;
    }

    pq[0] = idt;

    pq = quadidx + 4 * iqk;
    pq[0] = idk++;

    for( r = 0; r < divs - 1; r++ ){
        pq[1] = idk;
        pq += 4 * divs;
        pq[0] = idk++;
    }
    pq[1] = idk;

    pq = quadidx + 4 * iqb;
    pq[1] = idb;
    for( r = 0; r < qtb ; r++ ){
        pq += 4 * divs;
        pq[0] = idb++;
        pq[1] = idb;
    }

/*
 * fix up the center points
    Centers get split vertically as well as horizontally,
    so need 2 new vertices and 2 new TC sets.  The upper
    indices of the lower 2 quads point to the new data.
    The vertices are just copies of the center vertex;
    the new TCs have x = x of next row, y = center y.
    NOTE "next" is addr-- for top, addr++ for bottom,
  */

    // point indices
    jf = (dp1 + 1) * d2;	// edge to center
    ic = ppf + jf;		// top center
    id = 4 * ppf + jf;	// bot center

    // copy vertices
    ps = verts + 3 * ic;
    pd = verts + 3 * idc;
    *pd++ = ps[0]; *pd++ = ps[1]; *pd++ = ps[2];
    *pd++ = ps[0]; *pd++ = ps[1]; *pd++ = ps[2];
    ps = verts + 3 * id;
    *pd++ = ps[0]; *pd++ = ps[1]; *pd++ = ps[2];
    *pd++ = ps[0]; *pd++ = ps[1]; *pd++ = ps[2];

    // build new TC points via old quad indices
    // then change the indices

#define makeCTCtop(ps, pd, pq ) \
    *pd++ = ps[2 * pq[-1]];   \
    *pd++ = ps[2 * pq[-2] + 1]; \
    *pd++ = ps[2 * pq[0]]; \
    *pd++ = ps[2 * pq[1] + 1]
#if 1
    unsigned int jq = divs * (d2 - 1) + d2;	// upr rgt quad
    pq = quadidx + 4 * ( qpf + jq );
    jf = 2 * idc;
    ps = angls; pd = ps + jf;
    makeCTCtop(ps, pd, pq );
    ps = rects; pd = ps + jf;
    makeCTCtop(ps, pd, pq );
    ps = fishs; pd = ps + jf;
    makeCTCtop(ps, pd, pq );
    ps = cylis; pd = ps + jf;
    makeCTCtop(ps, pd, pq );
    ps = equis; pd = ps + jf;
    makeCTCtop(ps, pd, pq );
    ps = sters; pd = ps + jf;
    makeCTCtop(ps, pd, pq );
    ps = mercs; pd = ps + jf;
    makeCTCtop(ps, pd, pq );
    // change indices
    pq[-2] = idc; pq[1] = idc + 1;
#endif

#define makeCTCbot(ps, pd, pq ) \
    *pd++ = ps[2 * pq[-2]];   \
    *pd++ = ps[2 * pq[-1] + 1]; \
    *pd++ = ps[2 * pq[1]]; \
    *pd++ = ps[2 * pq[0] + 1]
#if 1
    jq = (divs + 1) * d2;		// lwr rgt quad
    pq = quadidx + 4 * ( 4 * qpf + jq ); // bot...
    jf = 2 * idc + 4;
    ps = angls; pd = ps + jf;
    makeCTCbot(ps, pd, pq );
    ps = rects; pd = ps + jf;
    makeCTCbot(ps, pd, pq );
    ps = fishs; pd = ps + jf;
    makeCTCbot(ps, pd, pq );
    ps = cylis; pd = ps + jf;
    makeCTCbot(ps, pd, pq );
    ps = equis; pd = ps + jf;
    makeCTCbot(ps, pd, pq );
    ps = sters; pd = ps + jf;
    makeCTCbot(ps, pd, pq );
    ps = mercs; pd = ps + jf;
    makeCTCbot(ps, pd, pq );
    // change indices
    pq[-1] = idc + 2; pq[0] = idc + 3;
#endif
}
