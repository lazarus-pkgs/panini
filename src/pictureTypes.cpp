/* pictureTypes.cpp for pvQt  06Oct2008 TKS
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

  The picture type names visible to the user, and their attributes.

*/
#include "pictureTypes.h"

pictureTypes::pictypnumdesc
pictureTypes::pictypn[NpictureTypes] = {
    { "proj", 1, QString(), 0,0,0,0 },
	{ "qtvr", 1, QString(), 0,0,0,0 },
	{ "rect", 1, QString(), 5,5,135,135 },
	{ "fish", 1, QString(), 50,50,360,360 },
	{ "cyli", 1, QString(), 50,25,360,135 },
	{ "equi", 1, QString(), 50, 25,360,180 }, 
	{ "cube", 6, QString(), 90,90,90,90 }
 };

// need c'tor as tr() does not work outside a QObject
pictureTypes::pictureTypes(){
    pictypn[0].desc = tr("PanoTools script or project");
	pictypn[1].desc = tr("QuickTime VR panorama");
	pictypn[2].desc = tr("Rectilinear image");
	pictypn[3].desc = tr("Fisheye or sphere image");
	pictypn[4].desc = tr("Cylindrical panorama");
	pictypn[5].desc = tr("Equirectangular panorama"); 
	pictypn[6].desc = tr("1 to 6 Cube face images");
}

// return index of a pic type name, -1 if none
int pictureTypes::picTypeIndex( const char * name ){
	if( name == 0 ) return -1;
	int i;
	for( i = 0; i < NpictureTypes; i++ ){
		if(!strcmp( name, pictypn[i].typ )) return i;
	}
	return -1;
}

// given cstring type name, return file count, 0 if no such type
int pictureTypes::picTypeCount( const char * name ){
	int i = picTypeIndex( name );
	if( i < 0 ) return 0;
	return pictypn[i].nfi;	
}

// given type index, return cstring name, 0 if no such type
const char * pictureTypes::picTypeName( int index ){
	if( index < 0 || index >= NpictureTypes ) return 0;
	return pictypn[index].typ;	
}


// given type index, return file count, 0 if no such type
int pictureTypes::picTypeCount( int index ){
	if( index < 0 || index >= NpictureTypes ) return 0;
	return pictypn[index].nfi;	
}

// given picture type name, return description
QString pictureTypes::picTypeDescr( const char * name ){
	int i = picTypeIndex( name );
	if( i < 0 ) return tr("ERROR: nonexistent picture type");
	return pictypn[i].desc;	
}

// given picture type index, return description
QString pictureTypes::picTypeDescr( int index ){
	if( index < 0 || index >= NpictureTypes )
		return tr("ERROR: nonexistent picture type");
	return pictypn[index].desc;	
}

QStringList pictureTypes::picTypeDescrs( ){
	QStringList sl;
	for(int i = 0; i < NpictureTypes; i++ ){
		sl.append(pictypn[i].desc);
	}
	return sl;
}

QSizeF pictureTypes::minFov( int index ){
	if( index < 0 || index >= NpictureTypes ) return QSizeF();
	return QSizeF( pictypn[index].minW, pictypn[index].minH );
}

QSizeF pictureTypes::maxFov( int index ){
	if( index < 0 || index >= NpictureTypes ) return QSizeF();
	return QSizeF( pictypn[index].maxW, pictypn[index].maxH );
}
