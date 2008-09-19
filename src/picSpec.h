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
	bool addFile( QString file );
	void setDims( QSize wh );
	QSize getDims();
	void setFovs( QSizeF hv );
	QSizeF getFovs();
signals:
	void wantFiles( int nmax );  // user clicked "add file(s)"
private slots:
	void on_filesBox_activated ( int idx );
	
private:
	int nfiles, maxfiles;
};

#endif	//ndef _PICSPEC_H_
