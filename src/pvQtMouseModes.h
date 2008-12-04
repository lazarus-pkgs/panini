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
Modifier	Buttons	Horizontal	Vertical\n\
  none	  left	  Yaw	  Pitch\n\
   Alt	  left	  Roll	  Pitch\n\
  Shift	  left	  hFov	  vFov\n\
   any	  right		  Zoom\n\
   any	  both	  Eye	  Zoom\n\
  Shift - double click: cycle projections\n\
  Scroll wheel: Zoom "));
	}
};

#endif  //ndef MouseModes_H
