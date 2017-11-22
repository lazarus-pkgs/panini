/*
 * pvQtPic.h
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
  pvQt is a panoramic image viewer built on the Qt4 framework and OpenGL.

  Class pictureTypes defines the supported source formats.

  Class pvQtPic holds the specifications and data for a picture.  It
  can be built with data already in memory, or local files, and can
  be subclassed to get the necessary information from other sources.

  Passing a pvQtPic to pvQtView::showPic() starts a picture display
  session, in which the view object is the active partner.  It
  gets information it needs by calling pvQtPic methods, and may
  also modify the pvQtPic.

  The images passed to view are always QImages, in a size and
  format negotiated by view, which knows the OpenGL capabilities,
  and pic, which knows the picture format. So texture imagea are
  often resized or otherwise transformed from the source images.
  Source images are normally read form an external mediunm when
  called for, but may be cached.

  Source image files can be in any format readable by QImageReader.
  Their locations can be specified by path name or url.  The base
  implementation will read local files specified by url (as passed
  by drag-n-drop, for example) but is not necessarily able to load
  remote files.

  Memory resident source images may be QImages or copies of files
  that QImageReader can decode (this includes most supported file
  types) or raster images whose format can be specified with the
  simple ad hoc interface defined here.

  The base class is a subclass of QObject, but does not define any
  signals or slots.

  The basic cycle of operation is:
  App calls
    setType() -- clear state & post fixed rqt's of new type
    setImageFOV() -- post the source image angular size
    setFaceImage() -- identify & verify the source image(s)
    [setLabel,setBorder(),setFill() -- set a nondefault style
      for empty frames, if desired]
  then passes pic to GLview::showPic().
  PvQtView calls
    setFaceFOV() -- sets image cropping/padding ratios
    setFaceSize() -- sets final face image size
    FaceImage() -- to get the final texture image(s)
    PictureFOV() -- to get pan and tilt limits

  For cubic pictures only, you can call setFaceImage() even
  after the picture is displayed, to add, replace or delete
  face images.   To delete a face, pass a null QImage *.
  To add or replace one, call any of the overloads.  The
  new images will be shown at the existing face dimensions.
*/

#ifndef __PVQTPIC_H__
#define __PVQTPIC_H__

#include <QtGui>

#define PVQT_PIC_FACE_FORMAT  QImage::Format_ARGB32

class pictureTypes;

class pvQtPic : public QObject
{	Q_OBJECT
public:

    /* Panosurface (projection screen) type */
    enum {
        sphere = 0,
        cylinder
    };

    /* Picture type implies the number of faces and the image projection */
    typedef enum {
        nil = 0,		// No picture
        cub,		// cubic (1 to 6 rectilinear cube face images)
        rec,		// rectilinear
        eqs,		// equal solid angle
        eqa,		// equal angle
        cyl,		// cylindrical
        eqr,		// equirectangular
        stg,		// stereographic
        mrc		// mercator
    } PicType;

/*
cube face IDs
Use front for all single images, front and back for hemi pairs.
These enumerated values should not be used as array indices.
*/
    typedef enum {
        any = -1,
        front,
        right,
        back,
        left,
        top,
        bottom
    } PicFace;

    pvQtPic( PicType type = nil );

/*
utility functions to interconvert pixels and angles
proj is an axis projection type code, from getxyproj()
fov in degrees, rad in radians
*/
    static bool getxyproj( PicType t, int & xproj, int & yproj );
    // radius / fl from full fov
    static double fov2rad( int proj, double fov );
    // full fov from radius / fl
    static double rad2fov( int proj, double rad );

    // new dimension from change in fov
    int  scalepix( int proj, int pix,
                   double fov, double tofov );
    // new fov from change in dimension
    double scalefov( int proj, double fov,
                     int pix, int topix );

    // adjust a 2D FOV to match image dimensions
    // The larger axis fov sets the scale.  If the other is
    // zero, the nonzero fov goes with the longer dimension
    QSizeF adjustFov( PicType t, QSizeF fovs, QSize dims );
    // new 2D fov from change in one angle
    QSizeF changeFovAxis( PicType t, QSizeF fovs, double fov, int axis = -1 );

    /* functions that rescale the cuurent picture */

    // new 2D fov from rescaling of dimensions
    QSizeF picScale2Fov( QSizeF scls );
    // new 2D fov from a change of pictype
    QSizeF changeFovType( PicType t, QSizeF fovs, PicType twas );
    // limit fovs to max for type, preserving aspect ratio
    QSizeF legalFov( PicType t, QSizeF fovs );

    /* Functions that return state info */

    PicType Type();		// nil => cannot display
    int 	 NumFaces();	// max for type, 0 for nil
    int 	 NumImages();	// number of faces that have source images

    int Surface(){ return surface; }

    // size of texture image(s)
    QSize   FaceSize(){ return facedims; }
    QSizeF  FaceFOV(){ return facefovs; }
    // size of source image
    QSize   ImageSize(){ return imagedims; }
    QSizeF  ImageFOV(){ return imagefovs; }
    // Effective size of displayed image in megapixels
    double	 PictureSize();

    QString FaceName( PicFace face = front );	// display name
    /* info about the default "empty image" for a face
   isEmpty( any ) is false if any face has a source image.
   Other fns return the empty string or black if face is
   any or has a source image.
*/
    bool isEmpty( PicFace face = any );
    QString	getLabel( PicFace face = front );
    QColor getBorder( PicFace face = front );
    QColor getFill( PicFace face = front );

    // normally called only from pvQtView:
    // set face size and display scaling
    bool fitFaceToImage( QSize maxdims, bool pwr2 = false );
    // texcoord scale factors to give correct displayed FOV
    QSizeF  getTexScale(){ return texscale; }
    // get a displayable image
    QImage * FaceImage( PicFace face = front ); // get face image
    // Apparent FOV for arbitrary projection and texcoord scale
    QSizeF  texScale2Fov( QSizeF scl, PicType t );

/*
  programmed setup fns return true: success, false: failure.

  A pvQtPic with type nil cannot be displayed or accept any setup
  calls but setType().

  setType() sets internal state to a legal default for the new type.
  The default angular sizes are also the largest ones allowed for
  the type; for some types this is the only legal value.

  Source images must be individually assigned to specific faces,
  legal for the type.

  call setType before setImageFov before setFaceImage
*/

    // to be called only from app:
    bool setType( PicType pt ); // clears, sets all defaults
    bool setSurface( int s );
    bool setImageFOV( QSizeF angles );
    bool setFaceImage( PicFace face, QImage * img );
    bool setFaceImage( PicFace face, int width, int height, void * addr,
                       int bitsPerColor, int colorsPerPixel,
                       bool floatValues = false,
                       bool packedPixels = true,
                       int alignBytes = 0 );
    bool setFaceImage( PicFace face, QString path );
    bool setFaceImage( PicFace face, QUrl url );

/*
Set empty frame styles
face = any sets all faces; label = "*" uses face names
label color is black or white according to fill color
NOTE setType() restores the default styles
 */
    bool setLabel( PicFace face = any, QString label = QString("*") );
    bool setBorder( PicFace face = any, QColor color = QColor() );
    bool setFill( PicFace face = any, QColor color = QColor() );

private:
    pictureTypes * picTypes; // for max fovs
    PicType type;
    int surface;
    int ipt; // pictureTypes index of type
    int maxfaces; // 0 to 6
    int numimgs; // no. of faces with source images
    // display image and face properties
    QImage::Format faceformat; // pixel format
    /* dimensions and assigned FOVs of source image never modified here */
    QSize imagedims;
    QSizeF imagefovs;
    /* texture image and scaling params set by fitFaceToImage() */
    QSize facedims;
    QSizeF facefovs;
    QSizeF texscale;
    QRectF cliprect; // fractional image clip
    QRect imageclip; // same scaled to image dims
    // display limits for current projection
    QSizeF maxfovs, minfovs;
    // latest arguments to fitFaceToImage
    QSize l_maxdims;
    bool l_pwr2;
    // arrays indexed by face id, 0:maxfaces-1
    bool accept[6];	// usable image
    int kinds[6]; // coded source type
    int formats[6];	// QImage::Format, or a kcode
    QSize idims[6];	// source dimensions
    void * addrs[6]; // address if in core
    QString names[6]; // path or url
    QString labels[6]; // for empty images...
    QColor borders[6];
    QColor fills[6];
    // common logic for assigning an image to a face
    bool addimgsize( int iface, QSize dims );
    // pixels <=> fov angle
    int xproj, yproj; // axis projection types

    // delete and zero a QImage*
    // used to remove cached source images
    void removeImg( int i );
    // load local images for a face
    QImage * loadEmpty( int face );
    QImage * loadFile( int face );
    QImage * loadQImage( int face );
    QImage * loadRaster( int face );

/*
      virtual functions for remote images --
      reimplement in subclasses that can fetch these.
      gotURL is called when a non-local url is put in urls[face].
      It must return true for a (probably) loadable image, else
      false.  If possible it should put image dimensions in dims,
      however that can be deferred until the image is loaded.
      loadURL is called when it is time to fetch the image.
*/
    virtual bool gotURL( QUrl url, QSize & dims ){
        Q_UNUSED(url);
        Q_UNUSED(dims);
        return false;
    }
    virtual QImage * loadURL( QUrl url ){
        Q_UNUSED(url);
        return 0;
    }
};

/*
 Picture types visble to the user
 The first Nprojections of them correspond to projections
 supported by quadsphere (does not include cubic)
*/
#define NpictureTypes 10
#define Nprojections  7

class pictureTypes:
        public QObject
{ Q_OBJECT
public:
    // constructor
    pictureTypes();
/*
 * access functions
  Picture type attributes are accessible by index or by
  the 4 letter picture type names visible to the user.
  For the subset corresponding to primitive projections,
  picture type names and pvQtPic::picType codes can be
  interconverted.
*/
    // PicType code (nil if not valid)
    pvQtPic::PicType PicType( const char * name );
    pvQtPic::PicType PicType( int index );

    // table index from key
    int picTypeIndex( pvQtPic::PicType t );	// subset only
    int picTypeIndex( const char * name );	// all rows

    // attributes
    const char * picTypeName( int index );
    const char * picTypeName( pvQtPic::PicType t );
    int picTypeCount( const char * name );
    int picTypeCount( int index );
    QString picTypeDescr( const char * name );
    QString picTypeDescr( int index );
    QSizeF minFov( int index );
    QSizeF maxFov( int index );
    QSizeF absMaxFov( int index );
    // all descriptors as a list of strings
    QStringList picTypeDescrs();

private:
    // picture type names, descriptions, and max file counts
    typedef struct {
        const char * typ;
        pvQtPic::PicType pictype;
        const int nfi; // max file count
        QString desc;
        double minW, minH, // smallest accepted
        maxW, maxH, // largest displayable
        AmaxW, AmaxH; // largest accepted
    } pictypnumdesc;

    static pictypnumdesc pictypn[NpictureTypes];
};

#endif // __PVQTPIC_H__
