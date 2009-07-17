/* TurnDialog.h		for Panini 0.62 27 Jan 2009
 * Copyright (C) 2009 Thomas K Sharpless
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
  Modal dialog, sets orientation of image on panosurface.
  C'tor sets default all 0 orientation.  This can be
    changed by calling setTurn();
  emits signal newTurn on user change only
  slot setTurn silently clips settings legal

    turn is number of 90 degree CW steps 0:3
    roll and pitch are in degrees

*/

#ifndef TURNDIALOG_H
#define TURNDIALOG_H

#include "ui_TurnDialog.h"

class TurnDialog
    : public QDialog, public Ui_TurnDialog
{
    Q_OBJECT
public:
    TurnDialog( QWidget * parent = 0 );
    void getTurn( int& turn, double& roll, double& pitch, double& yaw );
    void enableYaw( bool enb = true );
    void enablePitch( bool enb = true );
    void enableTurn( bool enb = true );
signals:
    void newTurn( int turn, double roll, double pitch, double yaw );
public slots:
    void setTurn( int turn, double roll, double pitch, double yaw );
private slots:
    void on_TurnList_currentIndexChanged();
    void on_RollBox_valueChanged();
    void on_PitchBox_valueChanged();
    void on_YawBox_valueChanged();
private:
    bool turnEnb;
    bool pitchEnb, yawEnb;
};
#endif	//ndef TURNDIALOG_H
