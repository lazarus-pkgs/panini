/* picTypeDialog.cpp  for pvQt 06Oct2008 TKS
*/
#include "picTypeDialog.h"

picTypeDialog::picTypeDialog( QWidget * parent)
: QDialog( parent )
{
	setupUi( this );
}

void picTypeDialog::setNameLabel( QString name ){
	nameLabel->setText( name );

}

void picTypeDialog::setPicTypes( QStringList types ){
	typesBox->addItems( types );
}

void picTypeDialog::selectPicType( int t ){
	typesBox->setCurrentIndex( t );
}

int picTypeDialog::chosenType(){
	return typesBox->currentIndex();
}
