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



pvQt::pvQt( QWidget * parent )
{
	theParent = parent;
	theImage = 0;
	connect( &picspec, SIGNAL(wantFiles( int )),
			 this, SLOT(getFiles( int )));
	connect( &picspec, SIGNAL(filePick( int )),
			 this, SLOT(filePick( int )));
	connect( &picspec, SIGNAL(facePick( int, int )),
			 this, SLOT(facePick( int, int )));
	connect( &picspec, SIGNAL(formatPick( int )),
			 this, SLOT(formatPick( int )));
	clear();
}

pvQt::~pvQt()
{
	if( theImage ) delete theImage;
}

// reset internal state
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
		face2file[i] = -1;
		file2face[i] = -1;
	}
	filespecs.clear();
	validDims.clear();
	nValidFiles = 0;
	picspec.clear(FACELIMIT);
//note leave picDir as-is
	setImageFileFilter();
}

// functions that return display faces or info about them

bool pvQt::isValid()
{
	if( type <= 0 ) return false;
	if( numfaces <= 0 ) return false;
	if(ddims.isEmpty() || dfovs.isEmpty() ) return false;
	int nf = 0;
	for(int i = 0; i < FACELIMIT; i++){ //loop over faces
		int k = kinds[i];
		if( k == 0 ) continue;	// this face not valid
		++nf;
		switch( k ){	// where is source image...
			case -1:	// file
				if( !filespecs[face2file[i]].isReadable() ) return false;
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

/* post a likely setup based on current type and valid files.
	If type is nil, guess a type from file specs; then adjust 
	state to be legal for the type.  This may invlove dropping
	some files.
*/

void pvQt::likelySetup()
{
// display dimensions (assumes all files same size)
	QSize & sz = validDims[0];
	int w = sz.width(), h = sz.height();
	ddims = sz;
	dfovs = QSizeF( 1, (double)h / (double)w ); // scaled below

// if no type, guess type based on file count & dimensions
	if( type == nil && nValidFiles > 0 ){
		if(nValidFiles > 2)			type = cub;		
		else if(nValidFiles == 2)	type = hem;
		else {	
			if( w == 2 * h )		type = eqr;
			else if( w == h )		type = sph;
			else if( w > 2 * h )	type = cyl;
			else					type = rec;
		}
	}
// set maxfaces for new type
	switch( type ){
	case ask:
	case nil: 
		clear(); 
		break;
	case rec: case sph: case eqr: case cyl:
		maxfaces = 1;
		break;
	case cub:
		maxfaces = 6;
		break;
	case hem:
		maxfaces = 2;
		break;
	}
// drop any excess files
	while( nValidFiles > maxfaces ){
		filespecs.removeAt( --nValidFiles );
		validDims.removeAt( nValidFiles );
	}

// set likely FOV
	qreal s;
	switch( type ){
	case ask:
	case nil: s = 0; break;
	case cub:
	case rec: s = 90; break;
	case sph: 
	case hem: s = 180; break;
	case eqr: 
	case cyl: s = 360; break;
	}
	dfovs.scale( s, s, (Qt::AspectRatioMode)0 );

  // post to dialog
	picspec.clear( maxfaces );
	picspec.setFormat( type );
	picspec.setDims( ddims );
	picspec.setFovs( dfovs );
	for( int i = 0; i < nValidFiles; i++ ){
		picspec.addFile( filespecs[i].fileName(), validDims[i] );
	}
}


/* Let user set up picture specs
*/
bool pvQt::buildInteractive(){
	clear();
// load initial files and their attributes
	getFiles(6);
	if(nValidFiles < 1) return false;
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

     QFileDialog::Options options;
 //crashes on WinVista...   options |= QFileDialog::DontUseNativeDialog;
     QString selectedFilter;
     QStringList files = QFileDialog::getOpenFileNames(
                                 (QWidget *)0,		// no parent
								 tr("pvQt -- Select Image File(s)"),
                                 picDir.absolutePath(),
                                 imageFileFilter + tr(";;All files (*.*)"),
                                 0,		// don't return filter index
                                 options);

	int nf = files.size();
	if( nf < 1 ) return 0;

  // add to filespecs, no dupes(assume no dupes in files)
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
  // set picDir to the directory of the added files
	k = filespecs.size() - 1;
	picDir.setPath( filespecs[k].absolutePath() );

	return n;
}

/*  get and validate some image files
  slot: also handles request from picspec for more image files
  user picks filenames; we keep those that are unique, readable 
  images in picspec, and their sizes, and a 

  Accepts files that QImageReader says it can read, warns about
  and drops the others.  
*/
void pvQt::getFiles( int nmax ){
	int k = filespecs.size();
	int n = askFiles( nmax );	// this posts all files
  // post the valid image files, drop the others
	for( int i = 0; i < n && nValidFiles < FACELIMIT; i++ ){
		QFileInfo & fi = filespecs[i+k];
		QString fn = fi.fileName();
		imgreader.setFileName( fi.absoluteFilePath() );
		if( imgreader.canRead() ){	// accept...
			QSize siz = imgreader.size();
			validDims.append( siz );
		} else {					// drop..
			qWarning("%s is not a valid image file", fn.toUtf8().data() );
			filespecs.removeAt(i);
			--i; --nValidFiles;
		}
	}
}

void pvQt::setImageFileFilter(){
	QList<QByteArray> fmts = QImageReader::supportedImageFormats();
	QString & ff = imageFileFilter;
	ff = tr("Image files (");
	int n = fmts.size();
	for(int i = 0; i < n; i++ ){
		QString fmt = QString(fmts[i]);
		ff += "*." + fmt.toLower();
		if( 1 + 1 == n ) break;
		ff += " ";
	}
	ff += ")";
}

/* handle associating faces with files
*/
void pvQt::filePick( int filidx ){
	picspec.setFace( file2face[filidx] );
}

void pvQt::facePick( int filidx, int facidx ){
	face2file[facidx] = filidx;
	file2face[filidx] = facidx;
}

void pvQt::formatPick( int idx ){
	if( type < cub || idx < type ){
		type = (PicType)idx;
		picspec.setFormat( type );
	}
}


