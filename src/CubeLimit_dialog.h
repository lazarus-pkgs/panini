#ifndef CUBELIMIT_DIALOG_H
#define CUBELIMIT_DIALOG_H

#include "ui_CubeLimit_dialog.h"

class CubeLimit_dialog
        : public QDialog,
        public Ui_Dialog {
    Q_OBJECT
public:
    CubeLimit_dialog( int lim, QWidget * parent = 0 )
    : QDialog( parent ) {
        setupUi( this );
        setLimit( lim );
    }
    int limit(){ return spinBox->value(); }
    void setLimit( int lim ){  spinBox->setValue( lim ); }

};

#endif // CUBELIMIT_DIALOG_H
