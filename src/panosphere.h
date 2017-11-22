/*
 * panosphere.h		for Panini 29 Jan 2009 TKS
    Arrays of vertices on the unit sphere and their
    2D texture coordinates for various projections,
    plus arrays of linear indices that map the
    vertices to line segments and quadrilaterals.

    The sphere is subdivided starting from the corners
    of the inscribed cube whose faces are centered on
    the coordinate axes.  The number of subdivisions
    along each cube edge, divs, is the c'tor argument.
    divs will be rounded up to the next even number if
    odd.  There are 6 * divs * divs quads in the final
    tesselation.

    The vertices are unit 3-vectors, and are also the
    normals OGL needs to generate cubic texture
    coordinates.

 *
 * Copyright (C) 2009 Thomas K Sharpless
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
*/

#ifndef	PANOSPHERE_H
#define	PANOSPHERE_H
#include "panosurface.h"

class panosphere : public panosurface {
public:
    panosphere( int divs = 30 );
};
#endif	//ndef	PANOSPHERE_H
