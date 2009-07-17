/* panocylinder.cpp	for pvQt 11 Dec 2008

  A cylinder tessellation with texture coordinates,
  line and quad incices, in arrays usable by OpenGl.

 *
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
*/
#define PANOSURFACE_IMPLEMENTATION
#include	"panocylinder.h"

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
  // get memory or die
    if (!getMemory()) return;

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

/* Texture coordinates */
    map_projections();

  /* fix up the +/- 180 wrap */
    float * pc = cylis;
    float * pe = equis;
    float * pm = mercs;
    r = 2 * cols;
    c = r - 2;
    for( row = 0; row < rows; row++ ){
        pc[0] = 1; pc[c] = 0; pc += r;
        pe[0] = 1; pe[c] = 0; pe += r;
        pm[0] = 1; pm[c] = 0; pm += r;
    }
}

