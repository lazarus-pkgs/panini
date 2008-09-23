/* picSpec.cpp  for pvQt  17 Sep2008 TKS
*/
#include "picSpec.h"
#include "pvQt.h"

picSpec::picSpec( QWidget * parent )
	: QDialog(parent)
{
	setupUi( this );
	filesBox->setEditable( false );

	formatBox->setEditable( false );
	formatBox->setMaxCount(7);
	formatBox->setMaxVisibleItems(7);
// indices == pvQt format codes...
	formatBox->addItem(tr(" ??? "));
	formatBox->addItem(tr("rectilinear"));
	formatBox->addItem(tr("fisheye"));
	formatBox->addItem(tr("cylindrical"));
	formatBox->addItem(tr("equirectangular"));
	formatBox->addItem(tr("cubic"));
	formatBox->addItem(tr("hemispheric"));

	faceBox->setEditable( false );
	faceBox->addItem(tr(" ??? "));
	faceBox->addItem(tr("front"));
	faceBox->addItem(tr("right"));
	faceBox->addItem(tr("back"));
	faceBox->addItem(tr("left"));
	faceBox->addItem(tr("top"));
	faceBox->addItem(tr("bottom"));
	
	clear( 1 );
}

void picSpec::clear( int nmax )
{
	filesBox->clear();
	nfiles = maxfiles = 0;
	setMaxFiles(nmax);

	faceBox->setCurrentIndex ( 0 );
	formatBox->setCurrentIndex ( 0 );
}

// add a file if nfiles < maxfiles
bool picSpec::addFile( QString file, QSize dims )
{
	if( nfiles >= maxfiles ) return false;

	QString s = QString("(%2 x %3) %1")
		.arg(file) .arg(dims.width()) .arg(dims.height());
	filesBox->setItemText( nfiles, s );

	if( ++nfiles < maxfiles ){
		filesBox->addItem( tr(" add file..."));
	}
	return true;
}

// add file(s) clicked -- ask for more
void picSpec::on_filesBox_activated ( int idx ){
	if( idx == nfiles && idx < maxfiles ) emit wantFiles( maxfiles - nfiles );
	else emit filePick( idx );
}

void picSpec::on_faceBox_currentIndexChanged( int idx ){
	if( idx > 0 && idx <= nfiles ) emit facePick( filesBox->currentIndex(), idx - 1 );
}

void picSpec::on_formatBox_currentIndexChanged( int idx ){
	if( idx > 0 && idx <= nfiles ) emit formatPick( idx );
}

void picSpec::setFace( int id ){
	faceBox->setCurrentIndex( id + 1 );
}

void picSpec::setFormat( int fmt ){
	formatBox->setCurrentIndex(fmt);
	filesBox->setCurrentIndex( 0 );
	faceBox->setCurrentIndex( fmt == 0 ? 0 : 1 );
	faceBox->setEnabled( fmt == pvQt::cub || fmt == pvQt::hem );
}

int picSpec::setMaxFiles( int mf )
{
	int i = filesBox->count();
	while( nfiles < i ) filesBox->removeItem( --i );

	maxfiles = mf < 0 ? 0 : mf;
	while( i > maxfiles )filesBox->removeItem( --i );

	filesBox->setMaxCount( maxfiles );
	filesBox->setMaxVisibleItems( maxfiles < 20 ? maxfiles : 20 );
	if( i < maxfiles ) filesBox->addItem(tr(" add file..."));

	filesBox->setCurrentIndex( 0 );
	filesBox->setEnabled( maxfiles > 0 );

	return i;
}

void picSpec::setDims( QSize wh ){
	widthPix->setValue( wh.width() );
	heightPix->setValue( wh.height() );
}

void picSpec::setFovs( QSizeF wh ){
	widthDeg->setValue( wh.width() );
	heightDeg->setValue( wh.height() );
}

