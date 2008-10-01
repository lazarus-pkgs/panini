/* GLwindow.h for pvQt_1

  glue layer to make pvQtView work inside QMainWindow

*/

#include <QWidget>

class pvQtView;
class pvQtPic;

class GLwindow : public QWidget {
	Q_OBJECT
public:
	GLwindow(QWidget * parent = 0);
	bool isOK(){ return ok; }
public slots:
	void newPicture();
protected:
	void resizeEvent( QResizeEvent * ev );

private:
	pvQtView * glview;	// display widget
	pvQtPic * pvpic;	// picture maker
	bool ok;	// true if created w/o error
};
