#include <QFileDialog>

#include "pvQt.h"
#include "picSpec.h"
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
	numimgs = numexpected = 0;
	dfovs = QSizeF(0,0);
	ddims = QSize(0,0);
	for( int i = 0; i < 6; i++){
		idims[i] = QSize(0,0);	
		names[i].clear();	
		addrs[i] = 0;	
		kinds[i] = 0;
		cubeidx[i] = 0;
		specidx[i] = 0;
	}
}

pvQt::pvQt( QWidget * parent )
{
	theParent = parent;
	theImage = 0;
	clear();
}

pvQt::~pvQt()
{
	if( theImage ) delete theImage;
}

// functions that return displayable images or info about them

bool pvQt::isValid()
{
	if( type <= 0 ) return false;
	if( numimgs <= 0 ) return false;
	if( numimgs != numexpected ) return false;
	if(ddims.isEmpty() || dfovs.isEmpty() ) return false;
	for(int i = 0; i < numimgs; i++){
		int k = kinds[i];
		switch( k ){
			case -1:	// file
				if( names[i].isEmpty() ) return false;
				break;
			case 0:		// none
				return false;
				break;
			default:	// in-core
				if( addrs[i] == 0 ) return false;
				break;
		}
	}
	return true;
}

pvQt::PicType pvQt::getPicType()
{
 	return type;
}

int pvQt::getNumImgs()
{
 	return numimgs;
}

QSize pvQt::getSize()
{
	return ddims;
}

QSizeF pvQt::getFOV()
{
	return dfovs;
}

QImage * pvQt::getImage( int index )
{
	if(index < 0 || index >= numimgs)
		return 0;
  // load the image...
    bool ok = false;
	int kind = kinds[index];
	switch( kind ){
		case -1:	// file...
			ok = loadFile( names[index] );
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
	type = pt;
	switch( pt ){
		case nil: 
			break;
		case rec:		// A rectilinear image up to 135 x 135 degrees
			numexpected = 1;
			break;
		case eqr:		// An equirectangular image up to 360 x 180 degrees
			numexpected = 1;
			break;
		case sph:		// A spherical (or fisheye) image up to 200 degrees diameter
			numexpected = 1;
			break;
		case cub:		// A cubic panorama (six rectilinear images of specific angular size)
			numexpected = 6;
			break;
		default:
			type = nil;
			return false;
			break;
	}
	return true;
}

bool pvQt::setSize( QSize dims )
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

bool pvQt::setImage( int index, QString file )
{
	if( index > numimgs ||
	    index >= numexpected ) return false;
	if( index == numimgs ) ++numimgs;
	names[index] = file;
	kinds[index] = -1 ;
	return true;
}

bool pvQt::setImage( int index, QImage * img )
{
	if( index > numimgs ||
	    index >= numexpected ) return false;
	if( index == numimgs ) ++numimgs;
	addrs[index] = img;
	kinds[index] = -2 ;
	return true;
}

bool pvQt::setImage( int index, int wid, int hgt, void * addr,
		int bpc, int cpp, bool fp, bool pk, int alg )
{
	if( index > numimgs ||
	    index >= numexpected ) return false;
	if( index == numimgs ) ++numimgs;
	idims[index] = QSize( wid, hgt );
	addrs[index] = addr;
	kinds[index] = kcode( bpc, cpp, alg, fp, pk ) ;
	return true;
}

bool pvQt::loadFile( QString name )
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
// pop a fileselector to get one or more image files
	QFileDialog fd( 0, tr("pvQt -- Select Image File(s)"));
	QStringList filters;
	filters << tr("Image files (*.jpg *.mov *.png *.tif *.tiff)")
		    << tr("All files (*.*)");
	fd.setNameFilters( filters );
	fd.setFileMode( QFileDialog::ExistingFiles );
	fd.setDirectory( picDir );
	if( fd.exec() == 0 ) return false;
	picDir = fd.directory();
	
	QStringList files( fd.selectedFiles() );

	int n = files.size();
	if( n < 1 || n > 6 ) return false;

return true;
  // get filespecs, leave just the name in files.
	filespecs.clear();
	for(int i = 0; i < n; i++){
		filespecs.append( QFileInfo( files[i] ) );
		files[i] = filespecs[i].baseName();
	}

	picspec->setFiles( files );

	if( picspec->exec() == 0 ) 
		return false;


	return true;
}
