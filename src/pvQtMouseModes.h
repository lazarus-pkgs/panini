#ifndef MouseModes_H
#define MouseModes_H

#include "ui_ShowText.h"

class pvQtMouseModes
	: public QDialog, public Ui_ShowText
{
	Q_OBJECT
public:
	pvQtMouseModes( QWidget * parent = 0 )
	: QDialog(parent) {
		setupUi( this );
		setWindowTitle(" Panini  mouse modes");
		plainTextEdit->setPlainText( QString( "\
  Key	Buttons	Horizontal	Vertical\n\
  none	  left	  Yaw	  Pitch\n\
  none	  right	  EyeZ	  Zoom\n\
  none	  both	  Roll	  Pitch\n\
  Shift	  left	  FrameX	  FrameY\n\
  Shift	  right	  hFov	  vFov\n\
  Shift	  both	  EyeX	  EyeY\n\
  Scroll wheel: Zoom "));
	}
protected:
	void resizeEvent( QResizeEvent * ev ){
		int r = ev->size().width(),
			b = ev->size().height();
	    plainTextEdit->setGeometry(QRect(10, 10, r - 18, b - 18));
	}

};

#endif  //ndef MouseModes_H
