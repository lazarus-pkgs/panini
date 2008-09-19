/* picSpec.cpp  for pvQt  17 Sep2008 TKS
*/
#include "picSpec.h"

picSpec::picSpec( QWidget * parent )
	: QDialog(parent)
{
	setupUi( this );
	filesBox->setEditable( false );

	formatBox->setEditable( false );
// indices == pvQt format codes...
	formatBox->addItem(tr(" ??? "));
	formatBox->addItem(tr("rectilinear"));
	formatBox->addItem(tr("spherical"));
	formatBox->addItem(tr("cylindrical"));
	formatBox->addItem(tr("equirectangular"));
	formatBox->addItem(tr("cubic"));

	faceBox->setEditable( false );
	faceBox->addItem(tr(" ??? "));
	faceBox->addItem(tr("front"));
	faceBox->addItem(tr("right"));
	faceBox->addItem(tr("back"));
	faceBox->addItem(tr("left"));
	faceBox->addItem(tr("up"));
	faceBox->addItem(tr("down"));
	
	clear( 1 );
}

void picSpec::clear( int nmax )
{
	filesBox->clear();
	nfiles = 0;
	if( nmax > 0 ){
		maxfiles = nmax;
		filesBox->setMaxCount( maxfiles );
		filesBox->setMaxVisibleItems( maxfiles < 10 ? maxfiles : 10 );
		filesBox->addItem(tr(" add file(s)..."));
		filesBox->setEnabled( true );
	} else {
		maxfiles = 0;
		filesBox->setEnabled( false );
	}

	faceBox->setCurrentIndex ( 0 );
	formatBox->setCurrentIndex ( 0 );
}

// add a file if nfiles < maxfiles
bool picSpec::addFile( QString file )
{
	if( nfiles >= maxfiles ) return false;
	filesBox->setItemText( nfiles, file );
	if( ++nfiles < maxfiles ){
		filesBox->addItem( tr(" add file(s)..."));
	}
	return true;
}

// add file(s) clicked -- ask for more
void picSpec::on_filesBox_activated ( int idx ){
	if( idx == nfiles && idx < maxfiles ) emit wantFiles( maxfiles - nfiles );
}


