/* picTypeDialog.h  for pvQt 06 Oct 2008

*/
#ifndef PICTYPEDIALOG_H
#define PICTYPEDIALOG_H

#include "ui_picTypeDialog.h"

class picTypeDialog 
	: public QDialog, public Ui_picTypeDialog
{
	Q_OBJECT
public:
	picTypeDialog( QWidget * parent = 0 );
	void setNameLabel( QString name );
	void setPicTypes( QStringList types );
	void selectPicType( int t );
	int chosenType();
};

#endif	//ndef PICTYPEDIALOG_H
