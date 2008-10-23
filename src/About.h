
#ifndef ABOUT_H
#define ABOUT_H

#include "ui_About.h"

class pvQtAbout
	: public QDialog, public Ui_AboutDialog
{
	Q_OBJECT
public:
	pvQtAbout( QWidget * parent = 0 );
	void setInfo( QString info );
};

#endif  //ndef ABOUT_H
