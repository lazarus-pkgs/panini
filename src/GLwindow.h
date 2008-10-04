/* GLwindow.h for pvQt_1

  A "glue widget", required to make pvQtView work inside QMainWindow.
  Also provides command line and input file handling.
 
*/

#include <QWidget>

class pvQtView;
class pvQtPic;

class GLwindow : public QWidget {
	Q_OBJECT
public:
	GLwindow(QWidget * parent = 0);
	bool isOK(){ return ok; }
	bool commandLine( int argc, char ** argv );
	bool QTVR_file( char * name );
	bool CUBE_files( char ** names );
public slots:
	void newPicture();
protected:
	void resizeEvent( QResizeEvent * ev );

private:
	pvQtView * glview;	// display widget
	pvQtPic * pvpic;	// picture maker
	bool ok;	// true if created w/o error
};
