/* picTypeDialog.h  for pvQt 06 Oct 2008
 * Copyright (C) 2008 Thomas K Sharpless
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

The user is shown a "source" name (filename or url)
and asked to choose the appropriate picture type and
angular sizes.  Type is just the index of the selected
line in a combobox, which you must load with meaningful
strings.  Sizes are limited to a settable range
that defaults to [10:360] wide x [10:180] high.  The
type index and sizes should be preset but if not, default
to 0 and upper limits.
*/
#ifndef PICTYPEDIALOG_H
#define PICTYPEDIALOG_H

#include "ui_picTypeDialog.h"
//#include <QtCore>

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
	void setMinSize( QSizeF range );
	void setMaxSize(QSizeF range );
	void setSize( QSizeF angles );
	QSizeF getSize( );
signals:
	void picTypeSelected( int t );
	
private:
	QSizeF  minwh, maxwh, widhgt;
	QDoubleValidator * widvd, * hgtvd;
private slots:
	void on_widBox_valueChanged( double w );
	void on_hgtBox_valueChanged( double h );
	
};

#endif	//ndef PICTYPEDIALOG_H
