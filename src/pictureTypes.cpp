/* pictureTypes.cpp for pvQt  06Oct2008 TKS
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

  The picture type names visible to the user, and their attributes,
  one of which is the associated pvQtPic::PicType code.

  The first 7 names are the projection names used  by quadsphere.
  These max FOVs set the ranges of the quadsphere mappings.
 */
#include "pvQtPic.h"

pictureTypes::pictypnumdesc
pictureTypes::pictypn[NpictureTypes] = {
    { "rect", pvQtPic::rec, 1, QString(), 10,10, 150,150, 165,165 },
    { "fish", pvQtPic::eqs, 1, QString(), 10,10, 360,360, 360,360 },
    { "sphr", pvQtPic::eqa, 1, QString(), 10,10, 360,360, 360,360 },
    { "cyli", pvQtPic::cyl, 1, QString(), 10,10, 360,150, 360,165 },
    { "equi", pvQtPic::eqr, 1, QString(), 10,10, 360,180, 360,180 },
    { "ster", pvQtPic::stg, 1, QString(), 10,10, 310,310, 360,360 },
    { "merc", pvQtPic::mrc, 1, QString(), 10,10, 360,150, 360,175 },
    { "cube", pvQtPic::cub, 6, QString(), 90,90, 90,90, 90,90 },
    { "proj", pvQtPic::nil, 1, QString(), 0,0,0,0,0,0 },
    { "qtvr", pvQtPic::nil, 1, QString(), 0,0,0,0,0,0 }
 };

// need c'tor as tr() does not work outside a QObject
pictureTypes::pictureTypes(){
    pictypn[picTypeIndex("proj")].desc =
        tr("PanoTools script or project");
    pictypn[picTypeIndex("qtvr")].desc =
        tr("QuickTime VR panorama");
    pictypn[picTypeIndex("rect")].desc =
        tr("Normal rectilinear image");
    pictypn[picTypeIndex("fish")].desc =
        tr("Fisheye or mirror ball image");
    pictypn[picTypeIndex("sphr")].desc =
        tr("Spherical image");
    pictypn[picTypeIndex("cyli")].desc =
        tr("Cylindrical panorama");
    pictypn[picTypeIndex("equi")].desc =
        tr("Equirectangular panorama");
    pictypn[picTypeIndex("cube")].desc =
        tr("1 to 6 Cube face images");
    pictypn[picTypeIndex("ster")].desc =
        tr("Stereographic image");
    pictypn[picTypeIndex("merc")].desc =
        tr("Mercator panorama");
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

// return index of a PicType code (Note nil => proj)
int pictureTypes::picTypeIndex( pvQtPic::PicType t ){
    int i;
    for( i = 0; i < NpictureTypes; i++ ){
        if( t == pictypn[i].pictype ) return i;
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
// get name for a pvQtPic type code
const char * pictureTypes::picTypeName( pvQtPic::PicType t ){
    int index = picTypeIndex( t );
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

QSizeF pictureTypes::absMaxFov( int index ){
    if( index < 0 || index >= NpictureTypes ) return QSizeF();
    return QSizeF( pictypn[index].AmaxW, pictypn[index].AmaxH );
}


pvQtPic::PicType pictureTypes::PicType( const char * name ){
    return PicType( picTypeIndex( name ));
}

pvQtPic::PicType pictureTypes::PicType( int i ){
    if( i < 0 || i > NpictureTypes ) return pvQtPic::nil;
    return pictypn[i].pictype;
}

