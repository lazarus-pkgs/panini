/* picTypeDialog.cpp  for pvQt 06Oct2008 TKS
*/
#include "picTypeDialog.h"

picTypeDialog::picTypeDialog( QWidget * parent)
: QDialog( parent )
{
	setupUi( this );
	setDims( QSize(0,0) );
	setMinFOV( QSizeF( 5, 5 ));
	setMaxFOV( QSizeF( 360, 180 ));
	setFOV( QSizeF( 360, 180 ));
	
	connect( typesBox, SIGNAL(currentIndexChanged( int )),
			this, SIGNAL( picTypeSelected( int )) );
}

void picTypeDialog::setNameLabel( QString name ){
	nameLabel->setText( name );
}

void picTypeDialog::setPicTypes( QStringList types ){
	typesBox->addItems( types );
}

void picTypeDialog::selectPicType( int t, bool lock ){
	typesBox->setCurrentIndex( t );
	typesBox->setEnabled( !lock );
}

int picTypeDialog::chosenType(){
	return typesBox->currentIndex();
}

void picTypeDialog::setMinFOV( QSizeF fovs ){
	minfov = fovs;
	fovBox->setMinimum(  ylong ? minfov.height() : minfov.width() );
	setFOV( thefov );
}

void picTypeDialog::setMaxFOV( QSizeF fovs ){
	maxfov = fovs;
	fovBox->setMaximum(  ylong ? maxfov.height() : maxfov.width() );
	setFOV( thefov );
}

void picTypeDialog::setFOV( QSizeF fovs ){
	thefov = fovs.expandedTo(minfov).boundedTo(maxfov);
	fovBox->setValue( ylong ? thefov.height() : thefov.width() );	
}

/** Slot: record size changes **/
void picTypeDialog::on_fovBox_valueChanged( double w ){
	if( ylong ){
		if( w == thefov.height() ) return;
		thefov.setHeight( w );
	} else {
		if( w == thefov.width() ) return;
		thefov.setWidth( w );
	}
}

void picTypeDialog::setDims( QSize wh ){
	dims = wh;
	widPixels->setText( QString::number( wh.width()));
	hgtPixels->setText( QString::number( wh.height()));
	ylong = wh.height() > wh.width();
}

QSizeF picTypeDialog::getFOV(){
	if( ylong ){
		thefov.setWidth( 0 );
	}  else {
		thefov.setHeight( 0 );
	}
	return thefov;
}



