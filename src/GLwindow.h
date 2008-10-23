/* GLwindow.h for pvQt_1
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

  A "glue widget", required to make pvQtView work inside QMainWindow.
  Also provides command line and input file handling.
 
*/

#include <QWidget>
#include "pictureTypes.h"
#include "picTypeDialog.h"
#include "About.h"

class pvQtView;
class pvQtPic;

class GLwindow : public QWidget {
	Q_OBJECT
public:
	GLwindow(QWidget * parent = 0);
	bool isOK(){ return ok; }
	bool commandLine( int argc, char ** argv );
signals:
	void showTitle( QString msg );

public slots:
	void newPicture( const char * type );
	void picTypeChanged( int t );
	void about_pvQt();
	
protected:
	void resizeEvent( QResizeEvent * ev );

private:
	bool QTVR_file( QString name );
	bool choosePictureFiles( const char * picTypeName = 0 );
	bool loadPictureFiles( QStringList names );
	const QStringList picTypeDescrs();
	const char * askPicType( QStringList files, 
							 QSizeF & size,
							 const char * ptyp = 0 );
	pvQtAbout aboutbox;

	picTypeDialog ptd;
	QSizeF picFov;
	pvQtView * glview;	// display widget
	pvQtPic * pvpic;	// picture maker
	bool ok;	// true if created w/o error
	pictureTypes pictypes;

	bool loadTypedFiles( const char * type, QStringList files );

};
