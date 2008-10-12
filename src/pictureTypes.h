/* pictureTypes.h for pvQt	10 Oct 2008
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
#ifndef PICTURETYPES_H
#define PICTURETYPES_H

#include <QtCore>

#define NpictureTypes 8

class pictureTypes :
	public QObject
{	Q_OBJECT
public:
	// picture type names, descriptions, and max file counts
	typedef struct { 
		const char * typ; 
		const int nfi; 
		QString desc; 
	} pictypnumdesc;
	pictureTypes();
	int picTypeIndex( const char * name );
	const char * picTypeName( int index );
	int picTypeCount( const char * name );
	int picTypeCount( int index );
	QString picTypeDescr( const char * name );
	QString picTypeDescr( int index );
	QStringList picTypeDescrs();
private:
  static pictypnumdesc pictypn[NpictureTypes]; 

};

#endif	//ndef PICTURETYPES_H
