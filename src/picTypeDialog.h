/* picTypeDialog.h  for pvQt 06 Oct 2008
 * Copyright (C) 2008 Thomas K Sharpless
 *
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
 *

The user is shown a "source" name (filename or url)
and asked to choose the appropriate picture type and
the angular size of the longer image axis.  Type
is just the index of the selected line in a combobox,
which you must load with meaningful strings.

FOVs are passed and returned as QSizeF objects, so there are
lower and upper limits for both axes.  However only the fov
of the longer (or 1st) axis is changed (the other is returned
as zero).  You must set valid image dimensions before running
the dialog, and calculate the "short" fov afterward.

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
    void selectPicType( int t, bool lock = false );
    void setDims( QSize wh );
    void setMinFOV( QSizeF fovs );
    void setMaxFOV( QSizeF fovs );
    void setFOV( QSizeF fovs );
    void setFreeFovs( bool val );

    int chosenType();
    QSizeF getFOV();
    bool freeFovs();	// state of "unlock" checkbox

signals:
    void picTypeSelected( int t ); // also free toggled
    void hFovChanged( double h );
    void vFovChanged( double v );

private slots:
    void freeToggled( bool ckd );
private:
    QSize	dims;
    QSizeF  minfov, maxfov, thefov;

};

#endif	//ndef PICTYPEDIALOG_H
