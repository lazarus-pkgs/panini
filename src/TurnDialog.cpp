/* TurnDialog.cpp		for Panini 0.62 27 Jan 2009
 * Copyright (C) 2009 Thomas K Sharpless
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
 *
*/

#include "TurnDialog.h"

TurnDialog::TurnDialog( QWidget * parent )
: QDialog( parent )
{
	setupUi( this );
	TurnList->addItem(tr(" Normal"));
	TurnList->addItem(tr(" 90 deg CW"));
	TurnList->addItem(tr(" Invert"));
	TurnList->addItem(tr(" 90 deg CCW"));
	enableTurn();
	enablePitch();
	setTurn( 0, 0, 0 );
}

void TurnDialog::getTurn( int& turn, double& roll, double& pitch ){
	turn = TurnList->currentIndex();
	roll = RollBox->value();
	pitch = PitchBox->value();
}

void TurnDialog::setTurn( int turn, double roll, double pitch ){
	blockSignals( true );
	TurnList->setCurrentIndex( turn & 3 );
	RollBox->setValue( roll );
	PitchBox->setValue( pitch );
	blockSignals( false );
}

void TurnDialog::enableTurn( bool enb ){
	TurnList->setEnabled( enb );
	if( !enb || !turnEnb ) TurnList->setCurrentIndex( 0 );
	turnEnb = enb;
}

void TurnDialog::enablePitch( bool enb ){
	PitchBox->setEnabled( enb );
	if( !enb || !pitchEnb ) PitchBox->setValue( 0 );
	pitchEnb = enb;
}

void TurnDialog::on_TurnList_currentIndexChanged(){
	if( turnEnb ) emit newTurn( TurnList->currentIndex(),
		          RollBox->value(), PitchBox->value() );
}

void TurnDialog::on_RollBox_valueChanged(){
	emit newTurn( TurnList->currentIndex(),
		          RollBox->value(), PitchBox->value() );
}

void TurnDialog::on_PitchBox_valueChanged(){
	if( pitchEnb ) emit newTurn( TurnList->currentIndex(),
		          RollBox->value(), PitchBox->value() );
}

