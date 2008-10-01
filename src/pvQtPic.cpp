/* pvQtPic.cpp
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
 	This is the base implementation; see pvQtPic.h for details.
 	
 */
 
 #include "pvQtPic.h"


#define kcode(bpc,cpp,isfp,pack,align) \
	((bpc&0xFF) + (cpp&0xFF)<<8 + (align&0xFF)<<16 \
	+ (isfp?0x01000000:0) + (pack?0x02000000:0))
#define kdecode(k,bpc,cpp,isfp,pack,align) \
	bpc=k&0xFF, cpp=(k>>8)&0xFF, align=(k>>16)&0xFF, \
	isfp=(k&0x01000000!=0, pack=(k&0x02000000!=0


 pvQtPic::pvQtPic( pvQtPic::PicType t )
{
    maxVideoRAM = 0;
  	pwrOfTwoFaceDims = false;
    maxFaceDims = QSize(0,0);
	if( !setType( t ) ) setType( nil );
}

bool pvQtPic::setType( pvQtPic::PicType t )
{	
 /* set default face format parameters and limits
   Dimensions and in most cases fovs can change when an 
   image is assigned.  These fovs are the upper limits.
   The lockfovs flag marks formats whose fovs and aspect 
   ratios are fixed.
 */  
 	
  	lockfovs = false;
	switch( t ){
  	case nil:	// No picture
  		maxfaces = 0;
  		facedims = QSize(0,0);
  		facefovs = QSizeF(0,0);
  		lockfovs = true;
  		break;
  	case rec:	// A rectilinear image up to 135 x 135 degrees
  		maxfaces = 1;
  		facedims = QSize(256,256);
  		facefovs = QSizeF(135,135);
  		break;
  	case sph:	// A spherical image up to 180 degrees diameter
  		maxfaces = 1;
  		facedims = QSize(256,256);
  		facefovs = QSizeF(180,180);
  		break;
  	case cyl:	// A cylindrical panorama up to 360 x 135 degrees
  		maxfaces = 1;
  		facedims = QSize(512,256);
  		facefovs = QSizeF(360,135);
  		break;
  	case eqr:	// An equirectangular panorama up to 360 x 180 degrees
  		maxfaces = 1;
  		facedims = QSize(512,256);
  		facefovs = QSizeF(360,180);
  		break;
  	case cub:	// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
   		maxfaces = 6;
  		facedims = QSize(256,256);
  		facefovs = QSizeF(90,90);
  		lockfovs = true;
 		break;
 	case hem:	// A panorama with 1 or 2 hemispherical images
  		maxfaces = 2;
  		facedims = QSize(256,256);
  		facefovs = QSizeF(180,180);
  		lockfovs = true;
  		break;
  	default:
		type = nil;		// disable API, but preserve
		return false;	// state for debugging
	}
  // accept the type
	type = t;

  // pixel format for face images
	faceformat = QImage::Format_RGB888; 
  // no face images assigned
	numimgs = numsizes = 0;
  // angular size limits
  	maxfovs = facefovs;
  	if( lockfovs ) minfovs = facefovs;
	else minfovs = QSizeF( 10, 10 );
  // clear source image info; set empty face style
	static QColor defborders[6] = {
		Qt::red, Qt::green, Qt::cyan,
		Qt::magenta, Qt::blue, Qt::yellow
	};
	for( int i = 0; i < 6; i++ ){
		kinds[i] = 0;			// coded source type
		idims[i] = QSize(0,0);	// source dimensions
		addrs[i] = 0	;		// address if in-core
		urls[i]  = QUrl();		// url if external
		labels[i] = FaceName( PicFace(i) );
		borders[i] = defborders[i];
		fills[i] = QColor(Qt::lightGray);
	}
	return true;
}


QString pvQtPic::FaceName( PicFace face )
{
	switch( face ){
 	case any:		return tr("any");
 	case front:		return tr("front");
 	case right:		return tr("right");
 	case back:		return tr("back");
 	case left:		return tr("left");
 	case top:		return tr("top");
 	case bottom:	return tr("bottom");
	}
 	return tr("no such face");
}

bool pvQtPic::setFaceSize( QSize dims )
{
	if( type == nil || dims.isEmpty() ) return false;
	if( lockfovs ){
		double t = dims.width() * facefovs.height();
		t /= dims.height() * facefovs.width();
		if( fabs( t - 1.0 ) > 1.e-8 ) return false;
	}
	facedims = dims;
	return true;
}

bool pvQtPic::setFaceFOV( QSizeF fovs )
{
	if( type == nil || fovs.isEmpty() ) return false;
	facefovs = fovs.boundedTo(maxfovs).expandedTo(minfovs);
	return true;
}

/** fns that set properties of the empty face images **/

bool pvQtPic::setLabel( pvQtPic::PicFace face, QString label )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	labels[int(face)] = label;
	return true;
}
bool pvQtPic::setBorder( pvQtPic::PicFace face, QColor color )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	borders[int(face)] = color;
	return true;
}

bool pvQtPic::setFill( pvQtPic::PicFace face, QColor color )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	fills[int(face)] = color;
	return true;
}

/**  Assign images to faces

	The source image can be in memory as a QImage or a generic
	raster image, in the local file system, or in some location 
	described by a url (the base implementation accepts only urls
	that describe local files).
	
	It is an error to assign to a face that already has an image,
	however you can remove any assigned image by passing a null
	pointer to the 'QImage *' version of setFaceImage.
	
	When possible, setFaceImage determines the size of the
	image, and adjusts the face dimensions to match, subject 
	to the following controls.
	1) if an estimate of the available video image memory has been 
	posted to maxVideoRAM, it will be respected;
	2) if image size limits have been posted to maxImageDims, they
	will limit the face dimensions (preserving source aspect ratio);
	3) if pwrOfTwoImageDims is true, display image dimensions will
	be powers of 2, and source images will be padded to fit.
	When setFaceImage can't determine the image size it sets a
	flag that makes FaceImage() try to do the same thing.
	
	All source images for a hemispherical or cubic picture must
	be the same size, and square, and will be adjusted to the same 
	display size.  It is not required to assign a full set of these
	images as there is a default "empty" image for every face.

**/
bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QImage * img )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	int i = int(face);
	
	if( img == 0 ){	// remove any assigned image
		if( kinds[i] && numimgs > 0 ) --numimgs;
		kinds[i] = 0;			// coded source type
		idims[i] = QSize(0,0);	// source dimensions
		addrs[i] = 0	;		// address if in-core
		urls[i]  = QUrl();		// url if external
		return true;
	}
	
	if( kinds[i] != 0 ) return false;
	
	kinds[i] = QIMAGE_KIND;
	addrs[i] = img;
	formats[i] = img->format();
	urls[i] = QUrl();

	return addimgsize( i, img->size() );
}

bool pvQtPic::setFaceImage( pvQtPic::PicFace face,
				int width, int height, void * addr,
 				int bitsPerColor, int colorsPerPixel, 
 				bool floatValues, bool packedPixels, int alignBytes )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	int i = int(face);

	if( kinds[i] != 0 ) return false;

	kinds[i] = RASTER_KIND;
	addrs[i] = addr;
	formats[i] = kcode(bitsPerColor,colorsPerPixel,
					floatValues, packedPixels, alignBytes );
	urls[i] = QUrl();
	
	return addimgsize( i, QSize(width,height) );
}
 			    	
bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QString path )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;

	int i = int(face);
	if( kinds[i] != 0 ) return false;
	
	QImageReader ir( path );
	if( !ir.canRead() ) return false;

	kinds[i] = FILE_KIND;
	addrs[i] = 0;
	formats[i] = 0;
	urls[i] = QUrl("file://" + path );
	
	return addimgsize( i, ir.size() );

}

bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QUrl url )
{
	if( url.scheme() == QString("file") ) return setFaceImage( face, url.path() );
  // post the url, even though base class can't fetch it
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	int i = int(face);
	
	kinds[i] = URL_KIND;
	addrs[i] = 0;
	formats[i] = 0;
	urls[i] = url;

  /* call a virtual function to deal with this url.
     if possible it should return the image dimensions
     in dims, however this can be deferred until image 
     load time if necessary
  */
  	QSize dims(0,0);
	if( gotURL( url, dims )) return addimgsize( i, dims );
  // reject
	kinds[i] = 0;
	urls[i] = QUrl();
	return false;
	
}

/* final stage of assigning image to face.

  All available info on image has been posted to face i,
  except for its size (idims[i]) which must be (0,0).
  
  The main test for acceptance is whether the image size is
  compatible with the display face size and/or the various
  constraints on that.  Tries to set face size with the first 
  valid size of a picture, checks the others.
 
  Will accept new images of unkown size provisionally, and 
  can be called again once the size is available.

  Returns true, with size posted, if image is accepted,
  or false, with all face image info cleared if not. 
  Increments numimgs ( if < maxfaces) for an accepted image,
  and numsizes if a valid size was posted.

*/
bool pvQtPic::addimgsize( int i, QSize dims )
{
	bool ok = false;
	if( numimgs < maxfaces ){	// new image?
		if( idims[i].isEmpty() ){	// yes...
			idims[i] = dims;	
			if( numimgs == 0 ){
				ok = true;
				facedims = idims[i]; // note may be 0
			} else {
				ok = idims[i] == facedims;
			}
		}
		if( ok ) {
			++numimgs;
			if( dims.isValid() ) ++numsizes;
		}
	} else {					// retest?
		if( idims[i].isEmpty() && dims.isValid() ){
			idims[i] = dims;	
			if( numsizes == 0 ){
				ok = true;
				facedims = idims[i]; // note may be 0
			} else {
				ok = idims[i] == facedims;
			}
		}
		if( ok ) {
			 ++numsizes;
		}
	}
	
	if( !ok ){
		kinds[i] = 0;			// coded source type
		idims[i] = QSize(0,0);	// source dimensions
		addrs[i] = 0	;		// address if in-core
		urls[i]  = QUrl();		// url if external
	}
	return ok;
}

/**  Functions that return state info  **/

pvQtPic::PicType pvQtPic::Type()
{
	return type;
}

int 	pvQtPic::NumFaces()
{
	return maxfaces;
}

int 	pvQtPic::NumImages()
{
	return numimgs;
}

QSize	pvQtPic::FaceSize()
{
	return facedims;
}

QSizeF	pvQtPic::FaceFOV()
{
	return facefovs;
}
bool	pvQtPic::isEmpty( PicFace face )
{
	if(face == any) return numimgs != 0;
	if(face < front || face >= PicFace(maxfaces) ) return 0;
	return kinds[int(face)] != 0;
}

QString	pvQtPic::getLabel( PicFace face )
{
	if(face < front || face >= PicFace(maxfaces) )
		return QString();
	return labels[int(face)];
}

QColor	pvQtPic::getBorder( PicFace face )
{
	if(face < front || face >= PicFace(maxfaces) )
		return QColor();
	return borders[int(face)];
}

QColor	pvQtPic::getFill( PicFace face )
{
	if(face < front || face >= PicFace(maxfaces) ) 
		return QColor();
	return fills[int(face)];
}

/**  functions that return dispayable images

  FaceImage() delivers a displayable image, which it gets
  by calling a reader for the appropriate source.  Readers
  return temp images sized for display but not necessarily
  in the display pixel format; if necessary, FaceImage makes
  the final image and deletes the temp one.  Caller should
  delete the returned QImage when done with it.
  
  Uses the posted display size and format.
**/

QImage * pvQtPic::FaceImage( PicFace face ){
	if( type == nil ) return 0;
	if( face < front || face >= PicFace(maxfaces) ) return 0;

	QImage * pim;
	int i = int(face);
	switch( kinds[i] ){
	case QIMAGE_KIND: 
		pim = loadQImage( i );
		break;
	case RASTER_KIND: 
		pim = loadRaster( i );
		break;
	case FILE_KIND:	
		pim = loadFile( i );
		break;
	case URL_KIND: 
		pim =  loadURL( urls[i] );
		break;
	default:
		return loadEmpty( i ); // fully formatted
		break;
	}
// convert pixel format if necessary	
	if( pim->format() == faceformat ) return pim;
	QImage *oim = new QImage( pim->convertToFormat(faceformat) );
	delete pim;
	return oim;
}

QImage * pvQtPic::loadEmpty( int i )
{
  // make the empty image for face i
	QImage * pim = new QImage( facedims, faceformat );
  // fill
	QPainter qp( pim );
	QBrush brush( fills[i] );
	QRect box( pim->rect() );
	qp.fillRect( box, brush );
  // draw border
	QPen pen( borders[i], 10 );
	qp.setPen( pen );
	box.adjust(5,5,-5,-5);
	qp.drawRect( box );
  // draw label
	pen.setColor( fills[i].value() < 100 ? Qt::white : Qt::black );
	qp.setPen( pen );
	int pts = (32 * facedims.height()) / 512 ;
	qp.setFont(QFont("Arial", pts));
	qp.drawText( box, Qt::AlignCenter, labels[i] );

	return pim;
}

QImage * pvQtPic::loadFile( int face )
{
	QImageReader ir( urls[face].path() );
	if( !ir.canRead() ) return 0;
	ir.setScaledSize( facedims );
	QImage *pim = new QImage( ir.read() );
	return pim;
}

QImage * pvQtPic::loadQImage( int face )
{
	QImage * iim = (QImage *)(addrs[face]);
	QImage * pim = new QImage(
		iim->scaledToHeight( facedims.height())
	);
	return pim;
}

QImage * pvQtPic::loadRaster( int face )
{
	return 0;
}


 


