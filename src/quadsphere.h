/* quadsphere.h		for pvQt	04 Nov 2008 TKS

	Arrays of vertices on the unit sphere and of
	their 2D texture coordinates for 5 projections,
	plus arrays of linear indices that map the 
	vertices to line segments and quadrilaterals.

	The sphere is subdivided starting from the corners
	of the inscribed cube whose faces are centered on
	the coordinate axes.  The number of subdivisions
	along each cube edge, divs, is the c'tor argument.
	divs will be rounded up to the next even number if
	odd.  There are 6 * divs * divs quads in the final
	tesselation.  

	The vertices are unit 3-vectors, so are also the
	unit normals OGL needs to generate cubic texture
	coordinates.

	The texture coordinates are 2D (s,t) normalized
	to [0:1] <=> max valid fov for the type.  Points
	outside the valid fov are set to 0 or 1.  Smaller 
	fovs can be mapped by scaling up the coordinates 
	using the texture matrix.

	There are two index arrays, one for quads and one
	for line segments that form a wireframe drawing of
	the face.  

	To avoid an interpolation artefact, vertices and
	texture coordinates that lie on the +/- 180 degree
	line in the YZ plane are duplicated; one copy gets 
	the positive and one the negative coordinate.  So
	there are 2 * divs more vertices than quads.  The
	extra points are not indexed for line drawing.

	All data are stored in one contiguous block, that
	could be copied to an OGL data buffer.  The byte 
	offsets	to and sizes of the various components are 
	available.
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
#ifndef	QUADSPHERE_H
#define	QUADSPHERE_H

class quadsphere {
public:
// Identifiers for the supported projections.   
// This enum matches pvQtPic type except at eqa = 0
	typedef enum { eqa,	rec, sph, cyl, eqr } projection;

	quadsphere( int divs = 30 );
	~quadsphere();
/* c'tor reports errors by posting an error message.  
	errMsg returns 0 if there was no error.
*/
	const char * errMsg(){ return errmsg; }

// 3D sphere points
	const float * vertices(){ return verts; }
	unsigned int vertexOffset(){ return 0; }
	unsigned int vertexBytes(){ return 3 * vertpnts * sizeof(float); }
// corresponding texture coordinates [0:1]
	const float * texCoords( projection proj );
	unsigned int texCoordOffset( projection proj );
	unsigned int texCoordSize(){ return 2 * vertpnts * sizeof(float); }
// index sequence for line drawing
	const unsigned int * lineIndices(){ return lineidx; }
	unsigned int lineIndexCount(){ return linewrds; }
	unsigned int lineIndexOffset(){ return (char *)lineidx - (char *)verts; }
	unsigned int lineIndexSize(){ return linewrds * sizeof(unsigned int); }
// index sequence for quad drawing (CW inside)
	const unsigned int * quadIndices(){ return quadidx; }
	unsigned int quadIndexCount(){ return quadwrds; }
	unsigned int quadIndexOffset(){ return (char *)quadidx - (char *)verts; }
	unsigned int quadIndexSize(){ return quadwrds * sizeof(unsigned int); }
// everything except the indices as a block of bytes
	char * dataBlockAddr(){ return (char *)words; }
	unsigned int dataBlockSize(){ return 15 * vertpnts * sizeof(float); }

private:
	char * errmsg;
  // all memory is allocated in one block
	float * words;
	unsigned int nwords;
  // array sizes
	unsigned int vertpnts;	// in points
	unsigned int linewrds;	// in words
	unsigned int quadwrds;	// in words
  // array addrs
	float * verts,
		  * rects,
		  * fishs,
		  * cylis,
		  * equis,
		  * angls;
	unsigned int   
		  *	lineidx,
		  * quadidx;
};
#endif	//ndef	QUADSPHERE_H
