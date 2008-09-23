/* picSpec.h for pvQt 17 Sep 2008 TKS
*/
#ifndef _PICSPEC_H_
#define _PICSPEC_H_

#include <QDialog>
#include "ui_PicSpec.h"

class picSpec : 
	public QDialog, 
	public Ui::Dialog
{
	Q_OBJECT
public:
	picSpec( QWidget * parent = 0 );	// create empty
  // make empty and set limit on number of files accepted
	void clear( int maxfiles = 1 );
  // add a file, returns sucess/failure
	bool addFile( QString file, QSize dims );
	void setDims( QSize wh );
	QSize getDims();
	void setFovs( QSizeF hv );
	QSizeF getFovs();
	void setFormat( int fmt );
	int getFormat();
	int setMaxFiles( int mf );	// returns file count
	void setFace( int id );	// note id is list index - 1
signals:
	void wantFiles( int nmax );  // user clicked "add file(s)"
	void filePick( int idx );	// user clicked a real file
	void facePick( int filidx, int facid ); //clicked a real face
	void formatPick( int idx );
private slots:
	void on_filesBox_activated ( int idx );
	void on_faceBox_currentIndexChanged( int idx );
	void on_formatBox_currentIndexChanged( int idx );
	
private:
	int nfiles, maxfiles;
};

#endif	//ndef _PICSPEC_H_
