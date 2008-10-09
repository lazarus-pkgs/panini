/* pictureTypes.h
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
