/* picSpec.cpp  for pvQt  17 Sep2008 TKS
*/
#include "picSpec.h"

picSpec::picSpec( QWidget * parent )
	: QDialog(parent)
{
	setupUi( this );
	filesBox->setEditable( false );
	for(int i = 0; i < 7; i++){
		filesBox->addItem(tr(" add a file..."));
	}

	formatBox->setEditable( false );
	formatBox->addItem(tr(" ??? "));
	formatBox->addItem(tr("equirectangular"));
	formatBox->addItem(tr("rectilinear"));
	formatBox->addItem(tr("spherical"));
	formatBox->addItem(tr("cube faces"));
	formatBox->addItem(tr("QTVR"));

	faceBox->setEditable( false );
	faceBox->addItem(tr(" ??? "));
	faceBox->addItem(tr("front"));
	faceBox->addItem(tr("right"));
	faceBox->addItem(tr("back"));
	faceBox->addItem(tr("left"));
	faceBox->addItem(tr("up"));
	faceBox->addItem(tr("down"));
}

void picSpec::setFiles( QStringList files ){
	int n = files.size();
	for(int i = 0; i < n; i++){
		filesBox->setItemText( i, files[i] );
	}
	for(int i = n; i < 7; i++){
		filesBox->setItemText(i, tr(" add a file..."));
	}
}

void picSpec::setFile( int idx, QString name ){
	if( idx >= 0 && idx < filesBox->count()){
		filesBox->setItemText( idx, name );
	}
}

