/* pictureTypes.cpp for pvQt  06Oct2008 TKS
*/
#include "pictureTypes.h"

pictureTypes::pictypnumdesc
pictureTypes::pictypn[NpictureTypes] = {
    { "proj", 1, QString()},
	{ "qtvr", 1, QString()},
	{ "rect", 1, QString()},
	{ "fish", 1, QString()},
	{ "cyli", 1, QString()},
	{ "equi", 1, QString()}, 
	{ "hemi", 2, QString()}, 
	{ "cube", 6, QString()}
 };

// need c'tor as tr() does not work outside a QObject
pictureTypes::pictureTypes(){
    pictypn[0].desc = tr("PanoTools script or project");
	pictypn[1].desc = tr("QuickTime VR panorama");
	pictypn[2].desc = tr("Normal lens photo");
	pictypn[3].desc = tr("Fisheye lens photo");
	pictypn[4].desc = tr("Cylindrical panorama");
	pictypn[5].desc = tr("Equirectangular panorama"); 
	pictypn[6].desc = tr("1 or 2 Hemispherical images"); 
	pictypn[7].desc = tr("1 to 6 Cube face images");
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

QStringList pictureTypes::picTypeDescrs(){
	QStringList sl;
	for(int i = 0; i < NpictureTypes; i++ ){
		sl.append(pictypn[i].desc);
	}
	return sl;
}
