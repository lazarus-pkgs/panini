#ifndef MouseModes_H
#define MouseModes_H

#include "ui_ShowText.h"

class pvQtMouseModes
	: public QDialog, public Ui_MouseModes
{
	Q_OBJECT
public:
	pvQtMouseModes( QWidget * parent = 0 ){
		setupUi( this );
		plainTextEdit->setPlainText( QString( "\
  Key	Buttons	Horizontal	Vertical\n\
  none	  left	  Yaw	  Pitch\n\
  none	  right	  Eye	  Zoom\n\
  none	  both		  Zoom\n\
  Shift	  left	  Roll	  Pitch\n\
  Shift	  right	  hFov	  vFov\n\
  Shift	  both		  Zoom\n\
  Shift	  double click: Next Projection\n\
  Scroll wheel: Zoom "));
	}
};

#endif  //ndef MouseModes_H
