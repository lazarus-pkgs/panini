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
 	
  A note on projections
	Three ideal projection types are supported:
	0 - rectilinear: radius = f * tan(angle)
	1 - equiangular: radius = f * angle
	2 - spherical:	 radius = f * sin( angle / 2 )
	(angles in radians, f = focal length)
	type 2, also known as the "mirror ball" projection, is a decent
	approximation for most fisheye lenses, and type 0 is a good
	approximation for most "normal" lenses.  Type 1 is used for the
	"pure angle" axes of cylindrical and equirectangular panoramas.
 	
 */
 
 #include "pvQtPic.h"

#ifndef Pi
#define Pi 3.141592654
#define DEG2RAD(x) (Pi*(x)/180.0)
#define RAD2DEG(x) (180.0*(x)/Pi)
#endif

#define kcode(bpc,cpp,isfp,pack,align) \
	((bpc&0xFF) + (cpp&0xFF)<<8 + (align&0xFF)<<16 \
	+ (isfp?0x01000000:0) + (pack?0x02000000:0))
#define kdecode(k,bpc,cpp,isfp,pack,align) \
	bpc=k&0xFF, cpp=(k>>8)&0xFF, align=(k>>16)&0xFF, \
	isfp=(k&0x01000000!=0, pack=(k&0x02000000!=0

/**  default border and fill colors for empty frames  **/
static QColor defborders[6] = {
	Qt::red, Qt::green, Qt::cyan,
	Qt::magenta, Qt::blue, Qt::yellow
};
static QColor deffill = QColor(Qt::lightGray);

/** utility functions, not exported  **/

static void scaleSize( QSize & s, double f ){
	int & w = s.rwidth();
	int & h = s.rheight();
	w = int( 0.5 + f * w );
	h = int( 0.5 + f * h );
}

// radius from (full) field of view
static double fov2rad( int proj, double fov ){
	double a = DEG2RAD(0.5 * fov);
	switch( proj ){
	default: return 0;
	case 0: return tan( a );
	case 1: return a;
	case 2: return sin( 0.5 * a );
	}
}
// (full) field of view from radius
static double rad2fov( int proj, double rad ){
	double a;
	switch( proj ){
	default: return 0;
	case 0: a = atan( rad );
		break;
	case 1: a = rad;
		break;
	case 2: a = 2 * asin( rad );
		break;
	}
	return 2 * RAD2DEG( a );
}
/* scale image dimensions to changes of FOV and vice versa
  proj is 0 for rectilinear, 1 for equiangular
  pix is pixels at fov degrees (full width or height)
  return pixels at tofov or degrees at topix
*/
int pvQtPic::scalepix( int proj, int pix, double fov, double tofov ){
	double d = fov2rad( proj, fov );
	if( d == 0 ) return 0;
	double s = fov2rad( proj, tofov ) / d ;
	return int(0.5 + s * pix );
}


 pvQtPic::pvQtPic( pvQtPic::PicType t )
{
	if( !setType( t ) ) setType( nil );
}

bool pvQtPic::setType( pvQtPic::PicType t )
{	
 /* set default face format parameters and limits
   Dimensions and in most cases fovs can change when an 
   image is assigned.  These fovs are the upper limits.
   The lockfovs flag marks formats whose fovs and aspect 
   ratios are fixed.
   Sets xproj, yproj accoding to axis projection type
 */  
 	
  	lockfovs = false;
  	xproj = yproj = 0;	// default rectilinear projection
  	
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
  		xproj = yproj = 2;	// ideal fisheye
  		break;
  	case cyl:	// A cylindrical panorama up to 360 x 135 degrees
  		maxfaces = 1;
  		facedims = QSize(512,256);
  		facefovs = QSizeF(360,135);
  		xproj = 1;
  		break;
  	case eqr:	// An equirectangular panorama up to 360 x 180 degrees
  		maxfaces = 1;
  		facedims = QSize(512,256);
  		facefovs = QSizeF(360,180);
  		xproj = yproj = 1;	// ideal equianglular
  		break;
  	case cub:	// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
   		maxfaces = 6;
  		facedims = QSize(256,256);
  		facefovs = QSizeF(90,90);
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
	imagedims = QSize(0,0);
	imagefovs = QSizeF(0,0);
	for( int i = 0; i < 6; i++ ){
		accept[i] = false;
		kinds[i] = 0;			// coded source type
		idims[i] = QSize(0,0);	// source dimensions
		addrs[i] = 0	;		// address if in-core
		names[i]  = QString();	// path or url
		labels[i] = FaceName( PicFace(i) );
		borders[i] = defborders[i];
		fills[i] = deffill;
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

/* change the working face image size
  To be called by View to negotiate a feasible
  texture image size.
  Returns false if there is no picture or the
  factor is absurd;
  Else true with face, image, and crop sizes
  adjusted.
  This function never changes cropping factors,
  as those depend on fovs only.
*/
bool pvQtPic::changeFaceSize( double factor )
{
	if( type == nil || facedims.isEmpty() ) return false;
	if( factor < 0.05 || factor > 20 ) return false;
	facedims.setWidth( int(0.5 + factor * facedims.width()) );
	facedims.setHeight( int(0.5 + factor * facedims.height()) );
	imagedims.setWidth( int(0.5 + factor * imagedims.width()) );
	imagedims.setHeight( int(0.5 + factor * imagedims.height()) );
	return facedims.width() > 0 && facedims.height() > 0;

}

/* set face FOV and size to accomodate image
  If there is no valid image just sets facefovs; otherwise
  also sets facedims by scaling imagedims.
  
  The passed fovs must be within the limits for the type,
  but need not agree with the image FOVs.
  
  Sets face size to image size, scaled by the ratios
  of face to image FOVs.  So image will be cropped to 
  the face size when image fov is larger, and will be 
  smaller than face when image fov is smaller.
  
*/
bool pvQtPic::setFaceFOV( QSizeF fovs ){
	if( type == nil ) return false;	// no picture
	if(fovs != fovs.boundedTo(maxfovs).expandedTo(minfovs))
		return false;	// illegal fovs
	facefovs = fovs;
	if( imagedims.isEmpty() ) return true;
	facedims.setWidth( scalepix( xproj, imagedims.width(),
			 imagefovs.width(), facefovs.width()));
	facedims.setHeight( scalepix( yproj, imagedims.height(),
			 imagefovs.height(), facefovs.height()));
	return true;
}

/* post source image FOVs
  should only be called once, before setFaceImage()
  returns false if no type or bad fovs
*/
bool pvQtPic::setImageFOV( QSizeF fovs )
{
	if( type == nil || fovs.isNull() ) return false;
	imagefovs = fovs;
	return true;
}

/** fns that set properties of the empty face images **/

bool pvQtPic::setLabel( pvQtPic::PicFace face, QString label )
{
	if( type == nil ) return false;
	if( face < any || face >= PicFace(maxfaces) ) return false;
	int l = face == any ? 0 : int(face);
	int u = face == any ? 5 : int(face);
	for( int i = l; i <= u; i++ ){
		if( label == "*" ) labels[i] = FaceName( pvQtPic::PicFace(i) );
		else labels[i] = label;
	}
	return true;
}
bool pvQtPic::setBorder( pvQtPic::PicFace face, QColor color )
{
	if( type == nil ) return false;
	if( face < any || face >= PicFace(maxfaces) ) return false;
	int l = face == any ? 0 : int(face);
	int u = face == any ? 5 : int(face);
	for( int i = l; i <= u; i++ ){
		if( color.isValid() ) borders[i] = color;
		else borders[i] = defborders[i];
	}
	return true;
}

bool pvQtPic::setFill( pvQtPic::PicFace face, QColor color )
{
	if( type == nil ) return false;
	if( face < any || face >= PicFace(maxfaces) ) return false;
	if( !color.isValid() ) color = deffill;
	int l = face == any ? 0 : int(face);
	int u = face == any ? 5 : int(face);
	for( int i = l; i <= u; i++ ){
		fills[i] = color;
	}
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
		names[i]  = QString();		// url if external
		return true;
	}
	
	if( kinds[i] != 0 ) return false;
	
	kinds[i] = QIMAGE_KIND;
	addrs[i] = img;
	formats[i] = img->format();
	names[i] = QString();

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
	names[i] = QString();
	
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
	names[i] = path;
	
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
	names[i] = QString();

  /* call a virtual function to deal with this url.
     if possible it should return the image dimensions
     in dims, however this can be deferred until image 
     load time if necessary
  */
  	QSize dims(0,0);
	if( gotURL( url, dims )) return addimgsize( i, dims );
  // reject
	kinds[i] = 0;
	names[i] = QString();
	return false;
	
}

/* common final stage of assigning an image to a face.
   called by all setFaceImage() overloads to post
   image and display face size data.

  All available info on image has been posted to face i
  except for its size (idims[i]) which should be (0,0) to
  show that this face doesn't have an image.  
  
  imgefovs must contain at least one of the angular 
  dimensions of the image (if the other is 0, it will be 
  set according to the image dimensions).
  
  Argument dims must hold the actual image dimensions.
  
  Argument i must be a valid face index [0:5].
 
  Returns true if the image is accepted.
  
  Counts accepted images in numimgs.
  
  Sets imagedims to the first accepted image size, and
  facedims to the size of a face (of the required shape)
  big enough to hold that image; and may set facefovs too.
 
*/
bool pvQtPic::addimgsize( int i, QSize dims )
{
	bool ok = false;
	if( numimgs < maxfaces && idims[i].isEmpty() ){	
		if( numimgs == 0 ){
			ok = true;	// first size wins
			imagedims = dims;
		  /* make sure image fovs are set:
		    if both 0, use the posted face fovs (strictly a fallback)
		    if one is 0, compute it from the other and image dimensions
		  */ 
		  	if( imagefovs.isNull() ) imagefovs = facefovs;
		  	else if( imagefovs.width() == 0 ){
		  		double r = (double)imagedims.width() / (double)imagedims.height();
		  		imagefovs.setWidth(rad2fov(xproj, r * fov2rad(yproj, imagefovs.height())));
	  		} else if( imagefovs.height() == 0 ){
		  		double r = (double)imagedims.height() / (double)imagedims.width();
	  			imagefovs.setHeight(rad2fov( yproj, r * fov2rad( xproj, imagefovs.width())));
  			}
		  // set face size using legal fovs closest to image fovs
			setFaceFOV( imagefovs.expandedTo(minfovs).boundedTo(maxfovs) );
		} else {
			ok = dims == imagedims;
		}
		idims[i] = dims;	
		if( ok ) ++numimgs;
	} 
	accept[i] = ok;
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

/**  functions that return dispayable images  **
  
  FaceImage() delivers a displayable image, which it gets
  by calling a reader for the appropriate source.  Readers
  return temp images of size imagedims in the native pixel
  format of the source; FaceImage makes the final image and
  may delete the temp one.  
  
  Caller should delete the returned QImage when done with it.
  
**/

QImage * pvQtPic::FaceImage( PicFace face ){
	if( type == nil ) return 0;
	if( face < front || face >= PicFace(maxfaces) ) return 0;

	QImage * pim = 0;
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
		pim =  loadURL( QUrl( names[i] ) );
		break;
	}
// if no image, get the empty face
	if( pim == 0 ) pim = loadEmpty( i ); 
// convert pixel format if necessary		
	if( pim->format() != faceformat ) {
		QImage *oim = new QImage( pim->convertToFormat(faceformat) );
		delete pim;
		pim = oim;
	}
// pack image into face if necessary
	if( !imagedims.isEmpty()  
	    && imagedims != facedims ){
		QSize cropdims = imagedims.boundedTo( facedims );
		QImage * oim = new QImage( facedims, faceformat );
		oim->fill( Qt::black );
		int jit = (pim->height() - cropdims.height() + 1) / 2;
		int jft = (oim->height() - cropdims.height() + 1) / 2;
		int jil = (pim->width() - cropdims.width() + 1) / 2;
		int jfl = (oim->width() - cropdims.width() + 1) / 2;
		int bpli = pim->bytesPerLine();
		int bplf = oim->bytesPerLine();
		int bpp = bpli / pim->width();
		int bcpy =  pim->width(); 
		if( bcpy > oim->width() ) bcpy = oim->width();
		bcpy *= bpp;
		unsigned char * pli, * plf;
		pli = pim->scanLine( jit + i ) + jil * bpp; 
		plf = oim->scanLine( jft + i ) + jfl * bpp; 
		for( int i = cropdims.height(); i > 0; --i ){
			memcpy( plf, pli, bcpy );
			pli += bpli;
			plf += bplf;
		}
		delete pim;
		pim = oim;
	}
	
	return pim;
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
	int pts = (32 * pim->height()) / 512 ;
	qp.setFont(QFont("Arial", pts));
	qp.drawText( box, Qt::AlignCenter, labels[i] );

	return pim;
}

QImage * pvQtPic::loadFile( int face )
{
	QImageReader ir( names[face] );
	if( !ir.canRead() ) return 0;
	ir.setScaledSize( imagedims );
	QImage *pim = new QImage( ir.read() );
	return pim;
}

QImage * pvQtPic::loadQImage( int face )
{
	QImage * iim = (QImage *)(addrs[face]);
	QImage * pim = new QImage(
		iim->scaledToHeight( imagedims.height())
	);
	return pim;
}

QImage * pvQtPic::loadRaster( int face )
{
	return 0;
}


 


