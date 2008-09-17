/* GLwindow.h for freepvQt 09Sep2008 TKS

  Switchboard layer to make GLview work inside QMainWindow

  Incoming signals result in a direct call of a slot on 
  the other side.  This allows for translation without
  needless local signal definitions.

  TODO: propagate main window resizing

*/

#include <QWidget>

class GLview;

class GLwindow : public QWidget {
	Q_OBJECT
public:
	GLwindow(QWidget * parent = 0);
	bool isOK(){ return ok; }
#if 0
public slots:
  // from MainWindow
	void step_pan( int dir );
	void step_tilt( int dir );
	void step_zoom( int dir );
	void step_roll( int dir );
	void set_view( int i );
  // from GLview
#endif
protected:
	void resizeEvent( QResizeEvent * ev );

private:
	GLview * glview;
	bool ok;	// true if created w/o error
};
