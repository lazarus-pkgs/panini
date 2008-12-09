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
	These ideal axial projections are supported:
	0 - rectilinear: d = f * tan(angle)
	1 - equiangular: d = f * angle
	2 - fisheye:	 d = f * sin( angle / 2 )
	3 - stereographic:  d = f * tan( angle / 2 )
	4 - mercator y:	 d = ln( tan( Pi/4 + angle / 2 ))
	(d = distance in image, f = focal length)
	type 2, also known as the "mirror ball" projection, is a decent
	approximation for most fisheye lenses, and type 0 is a good
	approximation for most "normal" lenses.  Type 1 is used for the
	"pure angle" axes of cylindrical, mercator and equirectangular 
	panoramas, type 3 for both axes of stereographic images, type 4
	for the vertical axis of mercator panoramas.

  Notes on FOVs
    The final displayed ("face") image must have FOVs within the
	diplayable range for its picture type.  If an input image FOV
	is larger, the image is cropped to make the picture FOV equal
	to the limit.  But the original input FOVS determine the
	displayed aspect ratio.

	Horizontal and vertical FOVs are treated as independent except 
	in adjustFOV() which couples them via the dimensions, assuming 
	square pixels.  



	
 */
 
#include "pvQtPic.h"
#include <cmath>

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

/**  Exported utility functions  **
	that don't depend on object state
*/

// radius from (full) field of view
double pvQtPic::fov2rad( int proj, double fov ){
	double a = DEG2RAD(0.5 * fov);
	switch( proj ){
	default: return 0;
	case 0: return tan( a );
	case 1: return a;
	case 2: return sin( 0.5 * a );
	case 3: return tan( 0.5 * a );
	case 4: return log( tan( 0.25 * Pi + 0.5 * a ));
	}
}
// (full) field of view from radius
double pvQtPic::rad2fov( int proj, double rad ){
	if( rad <= 0 ) return 0;
	double a;
	switch( proj ){
	default: return 0;
	case 0: 
		a = atan( rad );
		break;
	case 1: 
		a = rad;
		break;
	case 2: 
		a = 2 * asin( rad );
		break;
	case 3: 
		a = 2 * atan( rad );
		break;
	case 4: 
		double t = exp( rad );
		a = 2 * (atan( t ) - 0.25 * Pi );
		break;
	}
	return 2 * RAD2DEG( a );
}

// get axis projection type codes for a PicType
bool pvQtPic::getxyproj( PicType t, int & xproj, int & yproj ){
  	xproj = yproj = 0;	// default rectilinear projection
	switch( t ){
  	default:
		return false;
  	case cub:	
  	case rec:	
  		break;
  	case eqs:	
  		xproj = yproj = 2;
  		break;
  	case eqa:	
  	case eqr:	
  		xproj = yproj = 1;
  		break;
  	case cyl:	
  		xproj = 1;
  		break;
  	case stg:	
  		xproj = yproj = 3;
  		break;
  	case mrc:	
  		xproj = 1; yproj = 4;
  		break;
	}
	return true;
 }

/* limit fovs to the legal maximum for a given projection,
   preserving the linear aspect ratio
*/
QSizeF pvQtPic::legalFov( PicType t, QSizeF fovs ){
	int xp, yp;
	if( !getxyproj( t, xp, yp ) ) return fovs;
	QSizeF lims = picTypes->absMaxFov(picTypes->picTypeIndex( t ));
	double mw = lims.width(), mh = lims.height();
	double w = fovs.width(), h = fovs.height();
	double rw = w / mw, rh = h / mh;
	if( rw > rh ){
		if( rw > 1 ){
			w = mw;
			h = rad2fov(yp, fov2rad(yp, h) / rw );
		}
	} else if( rh > 1 ){
		w = rad2fov(xp, fov2rad(xp, w) / rh );
		h = mh;
	}
	return QSizeF( w, h );
}

/* scale an image dimension to change of FOV 
	or a FOV to change of dimension
  proj is an axis projection type code
  pix is pixels at fov degrees (full width or height)
  return pixels at tofov or degrees at topix
*/
int pvQtPic::scalepix( int proj, int pix, double fov, double tofov ){
	double d = fov2rad( proj, fov );
	if( d == 0 ) return 0;
	double s = fov2rad( proj, tofov ) / d ;
	return int(0.5 + s * pix );
}

double pvQtPic::scalefov( int proj, double fov, 
						  int pix, int topix ){
	if( pix == 0 ) return 0;
	double r = topix / pix;
	return rad2fov( proj, r * fov2rad( proj, fov ) );
}

/* fit a 2D FOV to image dimensions assuming square pixels
	and the input projection for a specified pictype.
  Both dimensions must be valid.  If one fov is zero, the 
  other gets assigned to the long dimension, which sets the
  scale.  If both are zero, result is (0,0).  
*/
QSizeF pvQtPic::adjustFov( PicType t, QSizeF fovs, QSize dims ){
	int pr[2];
	if( !getxyproj( t, pr[0], pr[1] ) ) return QSizeF(0,0);
	int d[2];
	d[0] = dims.width(); d[1] = dims.height();
	if( d[0] <= 0 || d[1] <= 0 ) return QSizeF(0,0);
	double f[2];
	f[0] = fovs.width(); f[1] = fovs.height();
	if( f[0] <= 0 ) f[0] = f[1];
	if( f[1] <= 0 ) f[1] = f[0];
 
	int i = 0, j = 1;
	if( d[1] > d[0] ){
		i = 1;
		j = 0;
	}
    double r = double(d[j])/double(d[i]);
	double s = r * fov2rad( pr[i], f[i] );
	f[j] = rad2fov( pr[j], s );

	return legalFov( t, QSizeF( f[0], f[1] ));


}

/* new 2D fov from a new value for one axis
	returns fovs for invalid type or FOV
   preserves the aspect ratio (unless fov == 0,
   when return  0, 0 ).
  Result fovs are limited to legal values for the projection,
  as given by picturetypes::absMaxFov
*/
QSizeF pvQtPic::changeFovAxis( PicType t, QSizeF fovs,
							   double fov, int axis ){
	int xp, yp;
	if( !getxyproj( t, xp, yp ) ) return fovs;
	if( fov <= 0 ) return fovs;
	double w = fovs.width(), h = fovs.height();
	if( w <= 0 || h <= 0 ) return fovs;

	w = fov2rad( xp, w );
	h = fov2rad( yp, h );
	if( axis < 0 ) axis = w >= h ? 0 : 1;
	double r;
	QSizeF rf;
	if( axis == 0 ){
		r = fov2rad( xp, fov ) / w;
		rf = QSizeF( fov, rad2fov( yp, h * r ) );
	} else {
		r = fov2rad( yp, fov ) / h;
		rf = QSizeF( rad2fov( xp, w * r ), fov );
	}

	return legalFov( t, rf );
}

/* new 2D FOV after linear rescaling of an image
   of a given projection type
   Returns fovs for invalid type
*/
QSizeF pvQtPic::picScale2Fov( PicType t, QSizeF fovs, 
							  QSizeF scl  // scale factors
							){
	int xp, yp;
	if( !getxyproj( t, xp, yp ) ) return fovs;
	double w = fov2rad( xp, fovs.width());
	double h = fov2rad( yp, fovs.height());
	return QSizeF( rad2fov(xp, w * scl.width()),
				   rad2fov(yp, h * scl.height()) );
}

/* new 2D fov from change of projection
*/
QSizeF pvQtPic::changeFovType( PicType t, QSizeF fovs, 
							   PicType twas ){
	int xp, yp;
	if( !getxyproj( twas, xp, yp ) ) return fovs;
	double xr = fov2rad( xp, fovs.width() ),
		   yr = fov2rad( yp, fovs.height() );
	if( !getxyproj( t, xp, yp ) ) return fovs;
	return QSizeF( rad2fov( xp, xr ),
				   rad2fov( yp, yr ) );
}


/**  Constructor  **/

pvQtPic::pvQtPic( pvQtPic::PicType t )
{
	picTypes = new pictureTypes();
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
 	
	type = nil;		// disable API
	ipt = picTypes->picTypeIndex(t);
	if( ipt < 0 )return false;

  // accept the type
	type = t;
	lockfovs = type == cub;
	getxyproj( type, xproj, yproj );
	maxfaces = picTypes->picTypeCount( ipt );

  // default face size 
	facefovs = picTypes->maxFov( ipt );
	facedims = QSize( 512, 512 );
	double r = facefovs.width() / facefovs.height();
	if( r < 1 ) facedims.setWidth( 256 );
	else if( r > 1 ) facedims.setHeight( 256 );
  // default pic = face
	picdims = facedims;
	imagefovs = facefovs;

  // pixel format for face images
	faceformat = PVQT_PIC_FACE_FORMAT; 
  // no face images assigned
	numimgs = 0;
  // angular size limits
  	maxfovs = facefovs;
  	if( lockfovs ) minfovs = facefovs;
	else minfovs = QSizeF( 10, 10 );
  // clear source image info; set empty face style
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

/* set facedims, facefovs and picdims for a proposed 
   maximum face (texture image) size.
  Assumes imagedims and imagefovs are valid.
  Called by pvQtView to set texture image size.  

  If pwr2 is true, maxdims must both be powers of 
  2 and the resulting face dimensions will be, too.
  Otherwise the face dimensions will be just large 
  enough to hold the source image (cropped to max 
  display fov).

  Sets facefovs to min( maxfovs, imagefovs ) and picdims
  so that fov fits inside the returned face dimensions. 

*/
bool pvQtPic::fitFaceToImage( QSize maxdims, bool pwr2 ){
	if( type == nil ) return false;	// no picture
	if(imagedims.isEmpty() ) return false;  // no image

  // scale factors: image dims to max displayable dims
	double rx = 
		fov2rad(xproj, maxfovs.width()) / fov2rad(xproj, imagefovs.width());
	double ry = 
		fov2rad(yproj, maxfovs.height()) / fov2rad(yproj, imagefovs.height());

  // full size of max displayable image
	int iw = imagedims.width(),
		ih = imagedims.height();
	if( rx < 1 ) iw = int( rx * iw );	// crop width
	if( ry < 1 ) ih = int( ry * ih );   // crop height

  // facefovs = displayable part of image
	facefovs = imagefovs.boundedTo(maxfovs);

  // scale down to fit maxdims (maybe adj to pwr of 2)
	int tw = iw, th = ih;
	int mw = maxdims.width(), mh = maxdims.height();
	double rw = double(mw) / double(tw),
		   rh = double(mh) / double(th);
	if( rw < rh ){
		if( rw < 1 ){
			tw = int( tw * rw );
			th = int( th * rw );
		}
	} else if( rh < 1 ){
		tw = int( tw * rh );
		th = int( th * rh );
	}

  /* tw, th are now the dimensions of the biggest
	 cropped image <= maxdims and <= imagedims. 
	 Put the corresponding uncropped size in picdims.
   */
	if( rx < 1 ) iw = int( tw / rx ); else iw = tw;
	if( ry < 1 ) ih = int( th / ry ); else ih = th;
	picdims = QSize( iw, ih );

  /* post the face dimensions 
     padded to powers of 2 if necessary
  */
	if( pwr2 ){
		rw = double( tw ) / double( mw );
		while( rw < 0.5 ){
			rw *= 2;
			mw /= 2;
		}
		tw = mw;
		rh = double( th ) / double( mh );
		while( rh < 0.5 ){
			rh *= 2;
			mh /= 2;
		}
		th = mh;
	}
	facedims = QSize( tw, th );
/*
qCritical("picdims %d, %d\npicfovs %.1f, %.1f\nfacedims %d, %d\nfacefovs %.1f, %.1f",
		  picdims.width(), picdims.height(),
		  imagefovs.width(), imagefovs.height(),
		  facedims.width(), facedims.height(),
		  facefovs.width(), facefovs.height() );
*/
	return !facedims.isEmpty();
}

/* Set working face dimensions (private)
  Sets facedims = the passed dimensions, and picdims =
  facedims scaled by imagefovs/facefovs.  Note if this makes 
  picdims > facedims the pimage will be cropped to the face 
  size when loaded, and will be padded to face size 
  when image fov is smaller.
20 Nov 08:
  When horizontal fov == 360 degrees, force pic width == face width
*/
bool pvQtPic::setFaceSize( QSize dims ){
	if( type == nil ) return false;	// no picture
	if(dims.isEmpty() ) return false;  // bad dimensions
	facedims = dims;
	int picw = scalepix( xproj, facedims.width(),
					facefovs.width(), imagefovs.width());
	int pich = scalepix( yproj, facedims.height(),
					facefovs.height(), imagefovs.height());
  // ensure clean wrap for 360 degree images
	if( imagefovs.width() == 360 ) picw = facedims.width();

	picdims = QSize( picw, pich );
	return !picdims.isEmpty();
}

/* (size at max display FOV) / (size at current face FOV).
   Used to scale texture coordinates for standard display
*/
QSizeF  pvQtPic::getTexScale(){
	QSizeF r(1.0, 1.0);
	if( type == nil ) return r;	// no projection
	if(imagedims.isEmpty() ) return r;  // no image
	if( facefovs == maxfovs ) return r;	// actual == max
	double w = fov2rad( xproj, facefovs.width());
	if( w > 0 ) r.setWidth( fov2rad( xproj, maxfovs.width()) / w );
	double h = fov2rad( yproj, facefovs.height());
	if( h > 0 ) r.setHeight( fov2rad( yproj, maxfovs.height()) / h );
	return r;
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
	
	All source images for a cubic picture must	be the same size, 
	and square, and will be adjusted to the same display size.  
	It is not required to assign a full set of these images as 
	there is a default "empty" image for every face.

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
	
	if( type != cub && kinds[i] != 0 ) return false;
	
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
	if( type != cub && kinds[i] != 0 ) return false;

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
	if( type != cub && kinds[i] != 0 ) return false;
	
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
	if( type != cub && kinds[i] != 0 ) return false;
 	
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
   called by all setFaceImage() overloads.

  All available info on image has been posted to face i
  except for its size (idims[i]).  
  Argument dims must hold the actual image dimensions.
  
  Argument i must be a valid face index [0:5].
 
  Returns true if the image is accepted, and keeps count
  of accepted images in numimgs.
  
  Sets imagedims to the minimum enclosing rectangle
  of all accepted images
  
  All cube face images must be square, but can be different
  sizes.
  
*/
bool pvQtPic::addimgsize( int i, QSize dims )
{
	bool ok = false;
	if( type != cub ){
		if( i != 0 ) return false;
	} else {
		if( dims.width() != dims.height()) return false;
	}
	if( accept[i] ){	// replace existing
		--numimgs; 
		accept[i] = false;
	}
	if( numimgs == 0 ){ // first image..
		imagedims = dims;
	  // make sure image fovs are set:
	  	if( imagefovs.isNull() ) imagefovs = facefovs;
	} else {
		imagedims = dims.expandedTo(imagedims);
	}
	idims[i] = dims;	
	++numimgs;
	accept[i] = true;
	return true; //setFaceSize( imagedims );
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

QSizeF  pvQtPic::PictureFOV(){
  return facefovs.boundedTo( imagefovs ); 
}

bool  pvQtPic::isEmpty( PicFace face )
{
	if(face == any) return numimgs == 0;
	if(face < front || face >= PicFace(maxfaces) ) return true;
	return kinds[int(face)] == 0;
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
  return temp images of size picdims in the native pixel
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
// if no image, return the empty face
	if( pim == 0 ) return loadEmpty( i );

// convert pixel format if necessary		
	if( pim->format() != faceformat ) {
		QImage *oim = new QImage( pim->convertToFormat(faceformat) );
		delete pim;
		pim = oim;
	}
// pack image into face if necessary
	if( !picdims.isEmpty()  
	    && picdims != facedims ){
		QSize cropdims = picdims.boundedTo( facedims );
		QImage * oim = new QImage( facedims, faceformat );
		oim->fill( 0xFF000000 ); // opaque black
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
	QPainter qp( pim );
  // fill
	QRect box( pim->rect() );
	QBrush brush( fills[i] );
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
// get info for temp fix now
// because read() wipes it out!
	QSize fs = ir.size();
	QByteArray fmt = ir.format();

	ir.setScaledSize( picdims );
	QImage *pim = new QImage( ir.read() );

/* Temp fix for a bug in Qt jpeg reader:
  if size was reduced, last column is probably black;
  so if hfov == 360, replace that col with the average 
  of its neighbors.  NOTE assumes 8 bits per color
*/
	if(  imagefovs.width() == 360
	  && picdims.width() < fs.width()
	  && ( fmt == "jpg" || fmt == "jpeg" )
	  )
	{
		int ppl = pim->width(),
			bpl = pim->bytesPerLine(),
			bpp = bpl / ppl;
		int ic = (ppl - 1) * bpp;
		unsigned char * pl = pim->bits();
		for( int l = 0; l < pim->height(); l++ ){
			for(int i = 0; i < bpp; i++ ){
				pl[i + ic] = (pl[i] + pl[i + ic - bpp]) >> 1;
			}
			pl += bpl;
		}
	}

	return pim;
}

QImage * pvQtPic::loadQImage( int face )
{
	QImage * iim = (QImage *)(addrs[face]);
	QImage * pim = new QImage(
		iim->scaledToHeight( picdims.height())
	);
	return pim;
}

QImage * pvQtPic::loadRaster( int face )
{
	return 0;
}


 


