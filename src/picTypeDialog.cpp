/* picTypeDialog.cpp  for pvQt 0.5  24 Nov 2008 TKS
 (C) copyright 2008, 209 Thomas K Sharpless

 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
    connect( nonSqOK, SIGNAL(toggled( bool )),
            this, SLOT( freeToggled( bool )) );

    connect ( hfovBox, SIGNAL(valueChanged(double)),
              this, SIGNAL(hFovChanged(double)));
    connect ( vfovBox, SIGNAL(valueChanged(double)),
              this, SIGNAL(vFovChanged(double)));
}

void picTypeDialog::setNameLabel( QString name ){
    nameLabel->setText( name );
}

void picTypeDialog::setPicTypes( QStringList types ){
    typesBox->blockSignals( true );
    typesBox->clear();
    typesBox->addItems( types );
    typesBox->blockSignals( false );
}

void picTypeDialog::setDims( QSize wh ){
    dims = wh;
    widPixels->setText( QString::number( wh.width()));
    hgtPixels->setText( QString::number( wh.height()));
}

void picTypeDialog::setMinFOV( QSizeF fovs ){
    minfov = fovs;
    hfovBox->blockSignals(true);
    vfovBox->blockSignals(true);
    hfovBox->setMinimum( minfov.width() );
    vfovBox->setMinimum( minfov.height() );
    setFOV( thefov );
    hfovBox->blockSignals(false);
    vfovBox->blockSignals(false);
}

void picTypeDialog::setMaxFOV( QSizeF fovs ){
    maxfov = fovs;
    hfovBox->blockSignals(true);
    vfovBox->blockSignals(true);
    hfovBox->setMaximum( maxfov.width() );
    vfovBox->setMaximum( maxfov.height() );
    setFOV( thefov );
    hfovBox->blockSignals(false);
    vfovBox->blockSignals(false);
}

void picTypeDialog::setFOV( QSizeF fovs ){
    thefov = fovs.expandedTo(minfov).boundedTo(maxfov);
    hfovBox->setValue( thefov.width() );
    vfovBox->setValue( thefov.height() );
}

void picTypeDialog::setFreeFovs( bool val ){
    nonSqOK->setChecked( val );
}

bool picTypeDialog::freeFovs(){
    return nonSqOK->isChecked();
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

void picTypeDialog::freeToggled( bool ckd ){
    emit picTypeSelected( typesBox->currentIndex() );
}



