/*
 * panocylinder.h		for Panini 29 Jan 2009 TKS

    Arrays of vertices on a unit cylinder and their
    2D texture coordinates for various projections,
    plus arrays of linear indices that map the
    vertices to line segments and quadrilaterals.

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
 *
*/

#ifndef	PANOCYLINDER_H
#define	PANOCYLINDER_H
#include "panosurface.h"

class panocylinder : public panosurface {
public:
    panocylinder( int divs = 120 );
};
#endif	//ndef	PANOCYLINDER_H
