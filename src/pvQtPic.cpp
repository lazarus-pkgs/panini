/*
 * pvQtPic.cpp
 * Copyright (C) 2008 Thomas K Sharpless
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
    These ideal projection functions are supported:
    0 - rectilinear: d = tan( a )
    1 - equiangular: d = a
    2 - fisheye:	 d = sin( a / 2 )
    3 - stereographic:  d = tan( a / 2 )
    4 - mercator y:	 d = asinh( tan( a ) )
      a = angle at projection center (radians)
      d = distance in image plane (unit = radius)
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

  How images are scaled for display
    OpenGL needs a "texture image", which I call a "face image".
    OGL may allow arbitrary texture image dimensions, or it may
    require that they are powers of 2.  In either case the largest
    feasible texture image is likely to be smaller than the source
    image.

    The picture must show the image's FOV, up to the limit,
    maxfovs, set by the texture coordinate map for the chosen
    projection.  Where imagefov > maxfov, part of the image must
    be discarded.  Where imagefov < maxfov, only part of the TC
    map applies, and the texture coordinates must be scaled up to
    reach [0:1] at imagefov.

    Qt image reader offers the ability to clip and scale an image
    as it is being read.  I use this to reduce source images to
    displayed size.  Parts corresponding to view angles > maxfov
    are clipped off, and the rest is scaled to the size of the
    texture image.

    The texture coordinate scale factors are set to display the
    face fovs correctly.  They are independent of the pixel aspect
    ratio in the texture image.

    The following class variables are involved in image scaling:
        maxfovs		angular range of current texture coordinate map
        imagedims	the source image dimensions
        imagefovs	its assigned fovs (may be > maxfovs)
        facedims	the chosen texture image size
        facefovs	imagefovs clipped to maxfovs
        texscale	texture coordinate scale factors
        cliprect	fractional image clip matching facefovs
        imageclip	source image clip rectangle (in pixels)
 */

#include "pvQtPic.h"
#include <cmath>

#ifndef Pi
#define Pi 3.141592654
#define DEG2RAD(x) (Pi*(x)/180.0)
#define RAD2DEG(x) (180.0*(x)/Pi)
#endif

// source kind codes used in kinds[]
enum{
    NO_KIND = 0,
    QIMAGE_KIND,
    RASTER_KIND,
    FILE_KIND,
    URL_KIND
};

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

/*
    utility functions that don't depend on object state
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
    case 4: return atanh(sin( a ));
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
        a = asin( tanh( rad ));
        break;
    }
    return 2 * RAD2DEG( a );
}

// get axis projection type codes for a PicType
bool pvQtPic::getxyproj( PicType t, int & xproj, int & yproj ){
    xproj = yproj = 0; // default rectilinear projection
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

/*
 * limit fovs to the legal maximum for a given projection,
 * preserving the linear aspect ratio
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

/*
 * scale an image dimension to change of FOV
 * or a FOV to change of dimension
 * proj is an axis projection type code
 * pix is pixels at fov degrees (full width or height)
 * return pixels at tofov or degrees at topix
*/
int pvQtPic::scalepix( int proj, int pix, double fov, double tofov ){
    double d = fov2rad( proj, fov );

    if( d == 0 ) {
        return 0;
    }
    double s = fov2rad( proj, tofov ) / d ;

    return int(0.5 + s * pix );
}

double pvQtPic::scalefov( int proj, double fov,
                          int pix, int topix ){
    if( pix == 0 ) {
        return 0;
    }
    double r = topix / pix;

    return rad2fov( proj, r * fov2rad( proj, fov ) );
}

/*
 * fit a 2D FOV to image dimensions assuming square pixels
 * and the input projection for a specified pictype.
 * Both dimensions must be valid.  If only one fov is nonzero,
 * it gets assigned to the long dimension;  if both are zero,
 * result is (0,0).  Otherwise the axis with the larger fov
 * sets the scale and the other fov is adjusted.
*/
QSizeF pvQtPic::adjustFov( PicType t, QSizeF fovs, QSize dims ){
    int pr[2];
    if( !getxyproj( t, pr[0], pr[1] ) ) {
        return QSizeF(0,0);
    }

    int d[2];
    d[0] = dims.width(); d[1] = dims.height();
    if( d[0] <= 0 || d[1] <= 0 ) {
        return QSizeF(0,0);
    }

    double f[2];
    f[0] = fovs.width(); f[1] = fovs.height();
    if( f[0] <= 0 ){
        if( f[1] <= 0 ) {
            return QSizeF(0,0);
        } else {
            f[0] = f[1];
        }
    }

    int i = 0, j = 1;
    if( f[1] > f[0] ){
        i = 1;
        j = 0;
    }

    double r = double(d[j])/double(d[i]);
    double s = r * fov2rad( pr[i], f[i] );
    f[j] = rad2fov( pr[j], s );

    return legalFov( t, QSizeF( f[0], f[1] ));
}

/*
 * new 2D fov from a new value for one axis
 * returns fovs for invalid type or FOV
 * preserves the aspect ratio if possible
 * Result limited to legal fovs for the projection
*/
QSizeF pvQtPic::changeFovAxis( PicType t, QSizeF fovs, double fov, int axis ){
    int xp, yp;

    if( !getxyproj( t, xp, yp ) ) {
        return fovs;
    }

    if( fov <= 0 ) {
        return fovs;
    }
    double w = fovs.width(), h = fovs.height();

    if( w <= 0 || h <= 0 ) {
        return fovs;
    }

    w = fov2rad( xp, w );
    h = fov2rad( yp, h );

    if( axis < 0 ) {
        axis = w >= h ? 0 : 1;
    }

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

/*
 * new 2D fov from change of projection
*/
QSizeF pvQtPic::changeFovType( PicType t, QSizeF fovs, PicType twas ){
    int xp, yp;

    if( !getxyproj( twas, xp, yp ) ) {
        return fovs;
    }

    double xr = fov2rad( xp, fovs.width() ),
            yr = fov2rad( yp, fovs.height() );

    if( !getxyproj( t, xp, yp ) ) {
        return fovs;
    }

    return QSizeF( rad2fov( xp, xr ),
                   rad2fov( yp, yr ) );
}


/* Constructor */
pvQtPic::pvQtPic( pvQtPic::PicType t )
{
    // zero the cached image pointers so
    // we won't try to delete them
    for(int i = 0; i < 6; i++ ) addrs[i] = 0;
    // set up for the given pic type, or none
    picTypes = new pictureTypes();
    if( !setType( t ) ) setType( nil );
    surface = sphere;
}

bool pvQtPic::setType( pvQtPic::PicType t )
{
/*
     * set default face format parameters and limits
     * Dimensions and in most cases fovs can change when an
     * image is assigned.  These fovs are the upper limits.
     * The lockfovs flag marks formats whose fovs and aspect
     * ratios are fixed.
     * Sets xproj, yproj accoding to axis projection type
*/

    type = nil; // disable API

    // pixel format for face images
    faceformat = PVQT_PIC_FACE_FORMAT;
    // no face images assigned
    numimgs = 0;
    // clear face image info; set empty face style
    for( int i = 0; i < 6; i++ ){
        removeImg( i );
        labels[i] = FaceName( PicFace(i) );
        borders[i] = defborders[i];
        fills[i] = deffill;
    }

    ipt = picTypes->picTypeIndex(t);
    if( ipt < 0 ) {
        return false;  // bad type
    }

    // accept the type
    type = t;
    getxyproj( type, xproj, yproj );
    maxfaces = picTypes->picTypeCount( ipt );

    // legalize panosurface
    if( surface != cylinder )  {
        surface = sphere;
    }

    // angular size limits
    maxfovs = picTypes->maxFov( ipt ); ///.boundedTo(SurfaceFOV());

    if( type == cub ) {
        minfovs = maxfovs;
    } else {
        minfovs = QSizeF( 10, 10 );
    }

    // default face size
    facefovs = maxfovs;
    facedims = QSize( 512, 512 );
    double r = facefovs.width() / facefovs.height();

    if( r < 1 ) {
        facedims.setWidth( 256 );
    } else if( r > 1 ) {
        facedims.setHeight( 256 );
    }

    // default image == maxfovs, no pixels
    imagefovs = maxfovs;
    imagedims = QSize(0,0);

    return true;
}

// Clear a face image info entry
// after deleting any associated QImage
// note raster images are not deleted
// note preserves label, border and fill
void pvQtPic::removeImg( int i ){
    if( i < 0 || i >= 6 ) {
        return;
    }

    if(	kinds[i] == QIMAGE_KIND && addrs[i] != 0 ) {
        delete (QImage *)addrs[i];
    }

    accept[i] = false;
    kinds[i] = 0; // coded source type
    idims[i] = QSize(0,0); // source dimensions
    names[i]  = QString(); // path or url
    formats[i] = 0; // pixel format
}

/*
 * select the panosurface.
 * adjust maxfovs and face cropping for new surface
*/
bool pvQtPic::setSurface( int s ){
    if( s != sphere && s != cylinder ) {
        return false;
    }

    surface = s;

    if(  type != nil  ) {
        maxfovs = picTypes->maxFov( ipt );  ///.boundedTo(SurfaceFOV());
        fitFaceToImage( l_maxdims, l_pwr2 );
    }
    return true;
}

QString pvQtPic::FaceName( PicFace face )
{
    switch( face ) {
    case any:
        return tr("any");
    case front:
        return tr("front");
    case right:
        return tr("right");
    case back:
        return tr("back");
    case left:
        return tr("left");
    case top:
        return tr("top");
    case bottom:
        return tr("bottom");
    }

    return tr("no such face");
}

/*
 * set display image parameters for a proposed
   maximum face (texture image) size

  If pwr2 is true, maxdims must both be powers of
  2 and the resulting face dimensions will be, too.

Assumes:
  imagedims, imagefovs and maxfovs are valid.
Sets:
  facefovs = imagefovs clipped to maxfovs
  facedims <= maxdims; either powers of 2 or square-pixel
    congruent with facefovs.  Will not exceed source image
    pixel count by more than a factor of 2.
  texscale = texture coordinate scale factors that map
        facefovs correctly.
  cliprect = fractional image clip to facefovs
  imageclip = same scaled to imagedims
*/
bool pvQtPic::fitFaceToImage( QSize maxdims, bool pwr2 ){
    // no picture
    if( type == nil ) {
        return false;
    }
    // no image
    if(imagedims.isEmpty() ) {
        return false;
    }

    // save args in case surface changes
    l_maxdims = maxdims;
    l_pwr2 = pwr2;

    // displayable angular size
    facefovs = imagefovs.boundedTo( maxfovs );

    // linearized fovs
    double rix = fov2rad(xproj, imagefovs.width()),
            riy = fov2rad(yproj, imagefovs.height()),
            ///rfx = fov2rad(xproj, facefovs.width()),
            ///rfy = fov2rad(yproj, facefovs.height()),
            rmx = fov2rad(xproj, maxfovs.width()),
            rmy = fov2rad(yproj, maxfovs.height());

    // texture coordinate scaling
    double rx = rmx / rix,
            ry = rmy / riy;
    texscale = QSizeF( rx, ry );

    // fractional image clip
    if( rx > 1 ) {
        rx = 1;
    }
    if( ry > 1 ) {
        ry = 1;
    }

    cliprect =  QRectF(
                0.5 * ( 1 - rx ),
                0.5 * ( 1 - ry ),
                rx, ry
                );

    // clipping rect in pixels
    int iw = imagedims.width();
    int ih = imagedims.height();

    imageclip = QRect(
                int( iw * cliprect.x()),
                int( ih * cliprect.y()),
                int( 0.5 + iw * cliprect.width()),
                int( 0.5 + ih * cliprect.height())
                );

    /*
    face size = clip size if possible.
    Otherwise largest feasible size not more
    than twice as big as the clip rectangle.
     */

    iw = imageclip.width();
    ih = imageclip.height();
    int mw = maxdims.width(),
            mh = maxdims.height();
    if( pwr2 ){
        while( mw >= 2 * iw ) mw /= 2;
        while( mh >= 2 * ih ) mh /= 2;
        iw = mw;
        ih = mh;
    } else {
        if( iw > mw ) {
            iw = mw;
        }
        if( ih > mh ) { ih = mh;
        }
    }

    // make sure cube face is square
    if( type == cub ) {
        ih = iw;
    }

    facedims = QSize( iw, ih );

    return true;
}

/*
 * apparent fovs corresponding to arbitrary
 * texture scale factors for current projection
*/
QSizeF pvQtPic::picScale2Fov( QSizeF scl ){
    if( type == cub ) {
        return maxfovs;
    }

    double mx = fov2rad( xproj, maxfovs.width() ),
           my = fov2rad( yproj, maxfovs.height() );

    return QSizeF( rad2fov( xproj, mx / scl.width() ),
                   rad2fov( yproj, my / scl.height() ) );
}

/*
 * Effective displayed image size in megapixels
*/
double pvQtPic::PictureSize(){
    if( type == nil ) {
        return 0;
    }

    double d = facedims.height() * numimgs;
    d *= facedims.width();
    d /= 1000000;

    return d;
}

/*
 * post source image FOVs
 * should only be called once, before setFaceImage()
 * returns false if no type or bad fovs
*/
bool pvQtPic::setImageFOV( QSizeF fovs )
{
    if( type == nil || fovs.isNull() ) {
        return false;
    }

    imagefovs = fovs;

    return true;
}

/** fns that set properties of the empty face images **/

bool pvQtPic::setLabel( pvQtPic::PicFace face, QString label )
{
    if( type == nil ) {
        return false;
    }
    if( face < any || face >= PicFace(maxfaces) ) {
        return false;
    }

    int l = face == any ? 0 : int(face);
    int u = face == any ? 5 : int(face);

    for( int i = l; i <= u; i++ ){
        if( label == "*" ) {
            labels[i] = FaceName( pvQtPic::PicFace(i) );
        } else {
            labels[i] = label;
        }
    }
    return true;
}

bool pvQtPic::setBorder( pvQtPic::PicFace face, QColor color )
{
    if( type == nil ) {
        return false;
    }
    if( face < any || face >= PicFace(maxfaces) ) {
        return false;
    }

    int l = face == any ? 0 : int(face);
    int u = face == any ? 5 : int(face);

    for( int i = l; i <= u; i++ ){
        if( color.isValid() ) {
            borders[i] = color;
        } else {
            borders[i] = defborders[i];
        }
    }

    return true;
}

bool pvQtPic::setFill( pvQtPic::PicFace face, QColor color )
{
    if( type == nil ) {
        return false;
    }
    if( face < any || face >= PicFace(maxfaces) ) {
        return false;
    }
    if( !color.isValid() ) {
        color = deffill;
    }

    int l = face == any ? 0 : int(face);
    int u = face == any ? 5 : int(face);

    for( int i = l; i <= u; i++ ){
        fills[i] = color;
    }

    return true;
}

/*
 * Assign images to faces
 *
 * The source image can be in memory as a QImage or a generic
 * raster image, in the local file system, or in some location
 * described by a url (the base implementation accepts only urls
 * that describe local files).
 *
 * It is an error to assign to a face that already has an image,
 * however you can remove any assigned image by passing a null
 * pointer to the 'QImage *' version of setFaceImage.
 *
 * All source images for a cubic picture must	be the same size,
 * and square, and will be adjusted to the same display size.
 * It is not required to assign a full set of these images as
 * there is a default "empty" image for every face.
*/
bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QImage * img )
{
    if( type == nil ) {
        return false;
    }
    if( face < front || face >= PicFace(maxfaces) ) {
        return false;
    }

    int i = int(face);

    if( img == 0 ){	// remove any assigned image
        if( kinds[i] && numimgs > 0 ) {
            --numimgs;
        }

        removeImg( i );
        return true;
    }

    if( type != cub && kinds[i] != 0 ) {
        return false;
    }

    removeImg( i );
    kinds[i] = QIMAGE_KIND;
    addrs[i] = img;
    formats[i] = img->format();

    return addimgsize( i, img->size() );
}

bool pvQtPic::setFaceImage( pvQtPic::PicFace face,
                            int width, int height, void * addr,
                            int bitsPerColor, int colorsPerPixel,
                            bool floatValues, bool packedPixels, int alignBytes )
{
    if( type == nil ) {
        return false;
    }
    if( face < front || face >= PicFace(maxfaces) ) {
        return false;
    }
    int i = int(face);
    if( type != cub && kinds[i] != 0 ) {
        return false;
    }

    removeImg( i );
    kinds[i] = RASTER_KIND;
    formats[i] = kcode(bitsPerColor,colorsPerPixel,
                       floatValues, packedPixels, alignBytes );

    return addimgsize( i, QSize(width,height) );
}

bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QString path )
{
    if( type == nil ) {
        return false;
    }
    if( face < front || face >= PicFace(maxfaces) ) {
        return false;
    }

    int i = int(face);
    if( type != cub && kinds[i] != 0 ) {
        return false;
    }

    QImageReader ir( path );
    if( !ir.canRead() ) {
        return false;
    }

    removeImg( i );
    kinds[i] = FILE_KIND;
    names[i] = path;

    return addimgsize( i, ir.size() );
}

bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QUrl url )
{
    if( url.scheme() == QString("file") ) {
        return setFaceImage( face, url.path() );
    }
    // post the url, even though base class can't fetch it
    if( type == nil ) {
        return false;
    }
    if( face < front || face >= PicFace(maxfaces) ) {
        return false;
    }
    int i = int(face);
    if( type != cub && kinds[i] != 0 ) {
        return false;
    }

    removeImg( i );
    kinds[i] = URL_KIND;
    names[i] = QString();

    /* call a virtual function to deal with this url.
     if possible it should return the image dimensions
     in dims, however this can be deferred until image
     load time if necessary
    */
    QSize dims(0,0);
    if( gotURL( url, dims )) {
        return addimgsize( i, dims );
    }
    // reject
    kinds[i] = 0;
    names[i] = QString();
    return false;
}

/*
 * common final stage of assigning an image to a face.
 * called by all setFaceImage() overloads.
 *
 * All available info on image has been posted to face i
 * except for its size (idims[i]).
 * Argument dims must hold the actual image dimensions.
 *
 * Argument i must be a valid face index [0:5].
 *
 * Returns true if the image is accepted, and keeps count
 * of accepted images in numimgs.
 *
 * Sets imagedims to the minimum enclosing rectangle
 * of all accepted images
 *
 * All cube face images must be square, but can be different
 * sizes.
*/
bool pvQtPic::addimgsize( int i, QSize dims )
{
    bool ok = false;
    if( type != cub ){
        if( i != 0 ) {
            return false;
        }
    } else {
        if( dims.width() != dims.height()) {
            return false;
        }
    }

    // replace existing
    if( accept[i] ){
        --numimgs;
        accept[i] = false;
    }

    // first image
    if( numimgs == 0 ){
        imagedims = dims;
        // make sure image fovs are set:
        if( imagefovs.isNull() ) {
            imagefovs = facefovs;
        }
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

int pvQtPic::NumFaces()
{
    return maxfaces;
}

int pvQtPic::NumImages()
{
    return numimgs;
}

bool pvQtPic::isEmpty( PicFace face )
{
    if(face == any) {
        return numimgs == 0;
    }
    if(face < front || face >= PicFace(maxfaces) ) {
        return true;
    }

    return kinds[int(face)] == 0;
}

QString	pvQtPic::getLabel( PicFace face )
{
    if(face < front || face >= PicFace(maxfaces) ) {
        return QString();
    }

    return labels[int(face)];
}

QColor	pvQtPic::getBorder( PicFace face )
{
    if(face < front || face >= PicFace(maxfaces) ) {
        return QColor();
    }

    return borders[int(face)];
}

QColor	pvQtPic::getFill( PicFace face )
{
    if(face < front || face >= PicFace(maxfaces) ) {
        return QColor();
    }
    return fills[int(face)];
}

/**  functions that return dispayable images  **

  FaceImage() delivers a displayable image, which it gets
  by calling a reader for the appropriate source.  Readers
  return new images of size facedims in the native pixel
  format of the source; FaceImage converts pixel format if
  necessary.
**/

QImage * pvQtPic::FaceImage( PicFace face ){
    if( type == nil ) {
        return 0;
    }
    if( face < front || face >= PicFace(maxfaces) ) {
        return 0;
    }

    QImage * pim = 0;
    int i = int(face);

    if( !idims[i].isNull() ){
        // set clipping rectangle for this face's image (allows mixed-size cube faces
        int dx = idims[i].width(),
                dy = idims[i].height();
        imageclip = QRect(
                    int( dx * cliprect.x()),
                    int( dy * cliprect.y()),
                    int( dx * cliprect.width()),
                    int( dy * cliprect.height())
                    );

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
    }
    // if no image, return the empty face
    if( pim == 0 ) {
        return loadEmpty( i );
    }

    // convert pixel format if necessary
    if( pim->format() != faceformat ) {
        QImage *oim = new QImage( pim->convertToFormat(faceformat) );
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
    if( !ir.canRead() ) {
        return 0;
    }
    QImage * pim;

    /*
     * TODO:
     * to avoid a bug in Qt4.4 jpeg reader:
     * read at full size, reduce using QImage ops
     */
#if 1
    pim = new QImage(
                ir.read().copy(
                    imageclip
                    ).scaled(
                    facedims,
                    Qt::IgnoreAspectRatio,
                    Qt::SmoothTransformation
                    )
                );

#else	// can use this if bug gets fixed:
    ir.setClipRect( imageclip );
    ir.setScaledSize( facedims );
    pim = new QImage( ir.read() );
#endif

    return pim;
}

QImage * pvQtPic::loadQImage( int face )
{
    QImage * iim = (QImage *)(addrs[face]);
    QImage * pim = new QImage(
                iim->copy(
                    imageclip
                    ).scaled(
                    facedims,
                    Qt::IgnoreAspectRatio,
                    Qt::SmoothTransformation
                    )
                );

    return pim;
}

QImage * pvQtPic::loadRaster( int face )
{
    return 0;
}
