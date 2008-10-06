/* GLwindow.h for pvQt_1

  A "glue widget", required to make pvQtView work inside QMainWindow.
  Also provides command line and input file handling.
 
*/

#include <QWidget>
#include "pictureTypes.h"

class pvQtView;
class pvQtPic;

class GLwindow : public QWidget {
	Q_OBJECT
public:
	GLwindow(QWidget * parent = 0);
	bool isOK(){ return ok; }
	bool commandLine( int argc, char ** argv );
	bool QTVR_file( QString name );
	bool rect_file( QString name );
	bool fish_file( QString name );
	bool cyli_file( QString name );
	bool equi_file( QString name );
	bool hemi_files( QStringList names );
	bool cube_files( QStringList names );
	bool choosePictureFiles( const char * picTypeName = 0 );
	bool loadPictureFiles( QStringList names );
	const QStringList picTypeDescrs();
	const char * askPicType( QStringList files );
public slots:
	void newPicture();
protected:
	void resizeEvent( QResizeEvent * ev );

private:
	pvQtView * glview;	// display widget
	pvQtPic * pvpic;	// picture maker
	bool ok;	// true if created w/o error
	pictureTypes pictypes;

	bool loadTypedFiles( const char * type, QStringList files );

};
