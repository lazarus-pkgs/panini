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
        setWindowTitle(" Panini  Mouse Modes");
        plainTextEdit->setPlainText( QString( "\
  Key	Buttons	Horizontal	Vertical\n\
          left	  Yaw	  Pitch\n\
          right	  EyeZ	  Zoom\n\
          both	  Roll	  Pitch\n\
  Shift	  left	  FrameX	  FrameY\n\
  Shift	  right	  EyeX	  EyeY\n\
  Shift	  both	  hFov	  vFov\n\
  \n\
  Hold Control key for horizontal-only\n\
  Hold Alt key for vertical-only\n\
  \n\
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
