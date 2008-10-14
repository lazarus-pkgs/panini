/* picTypeDialog.cpp  for pvQt 06Oct2008 TKS
*/
#include "picTypeDialog.h"

picTypeDialog::picTypeDialog( QWidget * parent)
: QDialog( parent )
{
	setupUi( this );
	setMinSize( QSizeF( 5, 5 ));
	setMaxSize( QSizeF( 360, 180 ));
	setSize( QSizeF( 360, 180 ));
	
	connect( typesBox, SIGNAL(currentIndexChanged( int )),
			this, SIGNAL( picTypeSelected( int )) );
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

void picTypeDialog::setMinSize( QSizeF range ){
	minwh = range;
	widBox->setMinimum( minwh.width() );
	hgtBox->setMinimum( minwh.height());
	setSize( widhgt );
}

void picTypeDialog::setMaxSize(QSizeF range ){
	maxwh = range;
	widBox->setMaximum( maxwh.width() );
	hgtBox->setMaximum( maxwh.height() );
	setSize( widhgt );
}

void picTypeDialog::setSize( QSizeF angles ){
	widhgt = angles.expandedTo( minwh ).boundedTo( maxwh );
	QString txt;
	widBox->setValue( widhgt.width() );	
	hgtBox->setValue( widhgt.height() );	
}

QSizeF picTypeDialog::getSize( ){
	return widhgt;
}

/** record size changes **/
void picTypeDialog::on_widBox_valueChanged( double w ){
	widhgt.setWidth( w );
}

void picTypeDialog::on_hgtBox_valueChanged( double h ){
	widhgt.setHeight( h );
}

