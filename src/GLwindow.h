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
#include "pvQtPic.h"
#include "picTypeDialog.h"
#include "About.h"
#include "TurnDialog.h"

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
	void showProj( QString name );
	void showFov( QSizeF fovs );
	void showSurface( int surf );

public slots:
  // from mainwindow
	void newPicture( const char * type );
	void about_pvQt();
	void save_as();
	void set_surface( int surf );
	void turn90( int t );
 // from picType dialog...
	void picTypeChanged( int t );
	void hFovChanged( double h );
	void vFovChanged( double v );
  // from pvQtView...
	void OGLerror( QString msg);
	void reportTurn( int turn, double roll, double pitch );
	
protected:
	void resizeEvent( QResizeEvent * ev );

private:
	bool QTVR_file( QString name );
	bool choosePictureFiles( const char * picTypeName = 0 );
	bool loadPictureFiles( QStringList names );
	const QStringList picTypeDescrs();
	const char * askPicType( QStringList files, 
							 const char * ptyp = 0 );
	bool loadTypedFiles( const char * type, QStringList files );
	void reportPic( bool ok, int c, QStringList files );
	void dragEnterEvent(QDragEnterEvent * event);
	void dropEvent(QDropEvent * event);

	pvQtAbout * aboutbox;
    TurnDialog turndialog;
	picTypeDialog ptd;
	pvQtView * glview;	// display widget
	pvQtPic * pvpic;	// picture maker
	bool ok;	// true if created w/o error
	pictureTypes pictypes;

	int ipt;	// current picture type index, or -1
	pvQtPic::PicType picType;
	QSizeF picFov;	// current FOV
	QSize  picDim;	// current size
	QSizeF lastFOV[NpictureTypes];
	int lastTurn[NpictureTypes];
	double lastRoll[NpictureTypes];
	double lastPitch[NpictureTypes];
	QString errmsg;

	QString loaddir, savedir;

};
