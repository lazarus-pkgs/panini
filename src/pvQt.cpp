#include <QFileDialog>

#include "pvQt.h"

/* 
  kcode -- a terse image format descriptor packed in one int
	byte 0: bpc = bits per color
	byte 1: cpp = colors per pixel
	byte 2: alg = byte alignment of rows (1,2,4,8,16)
	byte 3: flags
	  bit 0: fp => float values (valid if bpc >= 32 )
	  bit 1: pk => colors packed (else separate arrays)
  All valid kcodes are > 0
  kcode accepts alg = 0 as meaning "assume compiler default for 
    the pixel type, with no padding bytes", which is what a lot
    of code does.
*/
static int kcode( int bpc, int cpp, int alg, bool fp, bool pk ){
	if( bpc <= 0 || cpp <= 0 || alg < 0 ||alg > 16 ) return 0;
	int k = bpc & 0xFF;
		k |= (cpp & 0xFF) << 8;
		k |= (alg & 0xFF) << 16;
		if( fp ) k |= 0x1000000;
		if( pk ) k |= 0x2000000;
	return k;
}
static void kdecode( int k, int &bpc, int &cpp, int &alg, bool &fp, bool &pk ){
	if( k < 0 ) k = 0;
	bpc = k & 0xFF;
	cpp = (k >> 8) & 0xFF;
	alg = (k >> 16) & 0xFF;
	fp = (k & 0x1000000) != 0;
	pk = (k & 0x2000000) != 0;
}

void pvQt::clear()
{
	if(theImage) delete theImage;
	theImage = 0;
	type = nil;
	numfaces = 0;
	maxfaces = 1;
	ddims = QSize(0,0);	
	dfovs = QSizeF(0,0);
	for( int i = 0; i < FACELIMIT; i++){
		idims[i] = QSize(0,0);	
		addrs[i] = 0;	
		kinds[i] = 0;
		faceidx[i] = i;
		specidx[i] = i;
	}
	filespecs.clear();
	picspec.clear();
//note leave picDir as-is
}

pvQt::pvQt( QWidget * parent )
{
	theParent = parent;
	theImage = 0;
	connect( &picspec, SIGNAL(wantFiles( int )),
			 this, SLOT(getFiles( int )));
	clear();
}

pvQt::~pvQt()
{
	if( theImage ) delete theImage;
}

// functions that return display faces or info about them

bool pvQt::isValid()
{
	if( type <= 0 ) return false;
	if( numfaces <= 0 ) return false;
	if(ddims.isEmpty() || dfovs.isEmpty() ) return false;
	int nf = 0;
	for(int i = 0; i < FACELIMIT; i++){
		int k = kinds[i];
		if( k == 0 ) continue;
		++nf;
		switch( k ){
			case -1:	// file
				if( !filespecs[specidx[i]].isReadable() ) return false;
				break;
			default:	// in-core
				if( addrs[i] == 0 ) return false;
				break;
		}
	}
	return nf == numfaces;
}

pvQt::PicType pvQt::getPicType()
{
 	return type;
}

QSize pvQt::getNFaces()
{
 	return QSize(numfaces, maxfaces);
}

QSize pvQt::getFaceSize()
{
	return ddims;
}

QSizeF pvQt::getFOV()
{
	return dfovs;
}

QImage * pvQt::getFace( int index )
{
	if(index < 0 || index >= maxfaces)
		return 0;
  // load the image...
    bool ok = false;
	int kind = kinds[index];
	switch( kind ){
		case -1:	// file...
			ok = loadFile( index );
			break;
		case -2:	// QImage in core...
			ok = loadQImage( (QImage *) addrs[index] );
			break;
		case 0:		// no image
			ok = false;
			break;
		default:	// other image in core...
			ok = loadOther( kind, addrs[index] );
			break;
	}
	if( !ok ) return 0;
	return theImage;
}

// functions for building programmatically

bool pvQt::setPicType( PicType pt )
{
	clear();
	if( pt == ask ) return buildInteractive();
	if( pt < rec || pt > cub ) return false;
	type = pt;
	if( pt == cub )	maxfaces = 6;
	return true;
}

bool pvQt::setFaceSize( QSize dims )
{
	ddims = dims;
	if( ddims.isEmpty() ) {
		clear();
		return false;
	}
	return true;
}

bool pvQt::setFOV( QSizeF fovs )
{
	dfovs = fovs;
	if( dfovs.isEmpty() ) {
		clear();
		return false;
	}
	return true;
}

bool pvQt::setFace( int index, QImage * img )
{
	if( index < 0 || index >= maxfaces ) return false;
	if( kinds[index] == 0 ) ++numfaces;
	addrs[index] = img;
	kinds[index] = -2 ;
	return true;
}

bool pvQt::setFace( int index, int wid, int hgt, void * addr,
		int bpc, int cpp, bool fp, bool pk, int alg )
{
	if( index < 0 || index >= maxfaces ) return false;
	idims[index] = QSize( wid, hgt );
	addrs[index] = addr;
	int k = kcode( bpc, cpp, alg, fp, pk ) ;
	if( kinds[index] == 0 && k != 0) ++numfaces;
	kinds[index] = k;
	return true;
}

bool pvQt::loadFile( int faceindex )
{
	return false;
}

bool pvQt::loadQImage( QImage * pimg )
{
	return false;
}

bool pvQt::loadOther( int kind, void * addr )
{
	return false;
}

bool pvQt::buildInteractive(){
// set picspec to accept up to 6 file names
	picspec.clear( 6 );
// load initial files and their attributes
	getFiles(6);
// run dialog to verify the attributes	
	if( picspec.exec() == 0 ) return false;
// post validated picture specifications

	return true;
}

// pop a fileselector to let user add up to m image files to
// the filespecs list (no duplicates).  Returns number added.
int pvQt::askFiles( int m )
{
	if( m < 1 ) return 0;
	
	QFileDialog fd( 0, tr("pvQt -- Select Image File(s)"));
	QStringList filters;
	filters << tr("Image files (*.jpg *.mov *.png *.tif *.tiff)")
		    << tr("All files (*.*)");
	fd.setNameFilters( filters );
	fd.setFileMode( QFileDialog::ExistingFiles );
	fd.setDirectory( picDir );
	if( fd.exec() == 0 ) return 0;
	
	picDir = fd.directory();
	QStringList files( fd.selectedFiles() );
	int nf = files.size();
	if( nf < 1 ) return 0;

  // add to filespecs, no dupes
  // (assume no dupes in files)
	int n = 0;
	int k = filespecs.size();
	for(int i = 0; m > n && i < nf; i++){
		QString fn = files[i];
		QFileInfo fs( fn );
		int j;
		for( j = 0; j < k; j++ ){
			if( fs.absoluteFilePath() == filespecs[i].absoluteFilePath()) break;
		}
		if( j == k ){
			filespecs.append( fs );
			n++;
		}
	}
	return n;
}

/*  get and validate some image files
  slot: also handles request from picspec for more image files
  user picks filenames; we post those that are unique, readable 
  images to picspec, along with their formats and sizes, and a 
  suggested display tile size.
*/
void pvQt::getFiles( int nmax ){
	int k = filespecs.size();
	int n = askFiles( nmax );
	for( int i = 0; i < n; i++ ){
		QString fn = filespecs[i+k].fileName();
		imgreader.setFileName( fn );
		if( imgreader.canRead() ){
			QByteArray fmt = imgreader.format();
			picspec.addFile( fn );
		}
	}
}
