/* picTypeDialog.cpp  for pvQt 0.5  24 Nov 2008 TKS
*/
#include "picTypeDialog.h"

picTypeDialog::picTypeDialog( QWidget * parent)
: QDialog( parent )
{
	setupUi( this );
	setDims( QSize(0,0) );
	thefov = QSizeF( 360, 360 );
	setMinFOV( QSizeF( 5, 5 ));
	setMaxFOV( QSizeF( 360, 360 ));
	
	connect( typesBox, SIGNAL(currentIndexChanged( int )),
			this, SIGNAL( picTypeSelected( int )) );

	connect ( hfovBox, SIGNAL(valueChanged(double)),
			  this, SIGNAL(hFovChanged(double)));
	connect ( vfovBox, SIGNAL(valueChanged(double)),
			  this, SIGNAL(vFovChanged(double)));
}

void picTypeDialog::setNameLabel( QString name ){
	nameLabel->setText( name );
}

void picTypeDialog::setPicTypes( QStringList types ){
	typesBox->clear();
	typesBox->addItems( types );
}

void picTypeDialog::setDims( QSize wh ){
	dims = wh;
	widPixels->setText( QString::number( wh.width()));
	hgtPixels->setText( QString::number( wh.height()));
}

void picTypeDialog::setMinFOV( QSizeF fovs ){
	minfov = fovs;
	hfovBox->setMinimum( minfov.width() );
	vfovBox->setMinimum( minfov.height() );
	setFOV( thefov );
}

void picTypeDialog::setMaxFOV( QSizeF fovs ){
	maxfov = fovs;
	hfovBox->setMaximum( maxfov.width() );
	vfovBox->setMaximum( maxfov.height() );
	setFOV( thefov );
}

void picTypeDialog::setFOV( QSizeF fovs ){
	thefov = fovs.expandedTo(minfov).boundedTo(maxfov);
	hfovBox->setValue( thefov.width() );	
	vfovBox->setValue( thefov.height() );
}

QSizeF picTypeDialog::getFOV(){
	return thefov;
}

void picTypeDialog::selectPicType( int t, bool lock ){
	typesBox->setCurrentIndex( t );
	typesBox->setEnabled( !lock );
}

int picTypeDialog::chosenType(){
	return typesBox->currentIndex();
}



