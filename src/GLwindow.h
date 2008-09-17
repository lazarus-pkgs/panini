/* GLwindow.h for freepvQt 09Sep2008 TKS

  glue layer to make GLview work inside QMainWindow

*/

#include <QWidget>

class GLview;

class GLwindow : public QWidget {
	Q_OBJECT
public:
	GLwindow(QWidget * parent = 0);
	bool isOK(){ return ok; }
protected:
	void resizeEvent( QResizeEvent * ev );

private:
	GLview * glview;
	bool ok;	// true if created w/o error
};
