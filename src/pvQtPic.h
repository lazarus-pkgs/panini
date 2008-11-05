
 /* pvQtPic.h
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
  pvQt is a panoramic image viewer built on the Qt4 framework and OpenGL.
      
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

 */

#ifndef __PVQTPIC_H__
#define __PVQTPIC_H__

#include <QtGui>

#define PVQT_PIC_FACE_FORMAT  QImage::Format_ARGB32

class pictureTypes;	

class pvQtPic : public QObject
{	Q_OBJECT
public:

/* Picture type
   implies the number of faces and the preferred image projection
*/
typedef enum {
  nil = 0,		// No picture
  rec = 1,		// A rectilinear image up to 135 x 135 degrees
  sph = 2,		// A spherical image up to 360 degrees diameter
  cyl = 3,		// A cylindrical panorama up to 360 x 135 degrees
  eqr = 4,		// An equirectangular panorama up to 360 x 180 degrees
  cub = 5,		// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
 } PicType;
 
/* cube face IDs
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
 
// source kind codes used in kinds[]
enum{
	NO_KIND = 0,
	QIMAGE_KIND,
	RASTER_KIND,
	FILE_KIND,
	URL_KIND
};

 pvQtPic( PicType type = nil );
 
/* utility functions to interconvert pixels and angles
	proj indicates the projection function:
		0: rectilinear, 1: equal-angle
	The next 2 arguments calibrate the relationship;
	4th arg is the quantity to be converted.
  fovs are in degrees.  They represent the full width or height 
  of an image, so the calibration actually is fov/2 <=> pix/2.
*/
int 	scalepix( int proj, int pix, double fov, double tofov );

/* Functions that return displayable images or info about them
*/
 PicType Type();		// nil => cannot display
 int 	 NumFaces();	// max for type, 0 for nil
 int 	 NumImages();	// number of faces that have source images
// overall size of texture image(s)
 QSize   FaceSize(){ return facedims; }
// size of source image
 QSize   ImageSize(){ return imagedims; }
 QSizeF  ImageFOV(){ return imagefovs; }
// size of displayed image
 QSize	 PictureSize(){ return picdims; }
 QSizeF  PictureFOV();
 
 QString FaceName( PicFace face = front );	// display name
/* info about the default "empty image" for a face
   isEmpty( any ) is false if any face has a source image.
   Other fns return the empty string or black if face is
   any or has a source image.
*/
 bool		isEmpty( PicFace face = any );
 QString	getLabel( PicFace face = front );
 QColor		getBorder( PicFace face = front );
 QColor		getFill( PicFace face = front );
 
// normally called only from pvQtVew:
 QImage * FaceImage( PicFace face = front );  // get face image

/* programmed setup fns return true: success, false: failure.
  
  A pvQtPic with type nil cannot be displayed or accept any setup
  calls but setType().  
  
  setType() sets internal state to a legal default for the new type.
  The default angular sizes are also the largest ones allowed for 
  the type; for some types this is the only legal value.
  
  Source images must be individually assigned to specific faces, 
  legal for the type. 
  
*/

// to be called only from pvQtView, to set picture size...
 bool setFaceFOV( QSizeF fovs );	// FOV must be set first 
 bool setFaceSize( QSize dims );	// arbitrary dimensions
 bool fitFaceToImage();				// size from FOVs & source dims
 
// to be called only from app:
 bool setType( PicType pt ); 		// clears, sets all defaults
 bool setImageFOV( QSizeF angles );	// before setFaceImage, if needed
 bool setFaceImage( PicFace face, QImage * img );
 bool setFaceImage( PicFace face, int width, int height, void * addr,
 				int bitsPerColor, int colorsPerPixel, 
 				bool floatValues = false,
 			    bool packedPixels = true,
 			    int alignBytes = 0 );
 bool setFaceImage( PicFace face, QString path );
 bool setFaceImage( PicFace face, QUrl url );
 /* Set empty frame styles
   face = any sets all faces; label = "*" uses face names
   label color is black or white according to fill color 
   NOTE setType() restores the default styles
 */
 bool	setLabel( PicFace face = any, QString label = QString("*") );
 bool	setBorder( PicFace face = any, QColor color = QColor() );
 bool	setFill( PicFace face = any, QColor color = QColor() );


private:
	pictureTypes * picTypes;	// for max fovs
	PicType type;
	int maxfaces;	// 1, 2, or 6
	int numimgs;	// no. of faces with source images
	int numsizes;	// no. of faces with valid source sizes
  // display image and face properties
  	QImage::Format faceformat;
	QSize		imagedims;	// source image
	QSizeF		imagefovs;	// degrees
	QSize		picdims;	// image as displayed
	QSize		facedims;	// as displayed
	QSizeF		facefovs;	// degrees
	QSizeF		maxfovs, minfovs;	// face limits
	bool 		lockfovs;	// true => fixed fov & aspect ratio
  // arrays indexed by face id, 0:maxfaces-1
	bool		accept[6];	// usable image
	int		 	kinds[6];	// coded source type
	int			formats[6];	// QImage::Format, or a kcode
	QSize		idims[6];	// source dimensions
	void *		addrs[6];	// address if in core
	QString		names[6];	// path or url
	QString		labels[6];	// for empty images...
	QColor		borders[6];
	QColor		fills[6];
// common logic for assigning an image to a face
	bool	addimgsize( int iface, QSize dims );
// pixels <=> fov angle
	int 	xproj, yproj;	// axis projection types
// load local images for a face
	QImage * loadEmpty( int face );
	QImage * loadFile( int face );
	QImage * loadQImage( int face );
	QImage * loadRaster( int face );

 /*  virtual functions for remote images --
	 reimplement in subclasses that can fetch these.
   gotURL is called when a non-local url is put in urls[face].
     It must return true for a (probably) loadable image, else
     false.  If possible it should put image dimensions in dims,
	 however that can be deferred until the image is loaded. 
   loadURL is called when it is time to fetch the image. 

  */
	virtual bool gotURL( QUrl url, QSize & dims ){
		 return false; 
	}
	virtual QImage * loadURL( QUrl url ){ 
		return 0; 
	}
};

/* Picture types visble to the user
*/
#define NpictureTypes 7

class pictureTypes :
	public QObject
{	Q_OBJECT
public:
  // picture type names, descriptions, and max file counts
	typedef struct { 
		const char * typ; 
		const int nfi; 					// max file count
		QString desc; 
		double minW, minH, maxW, maxH;	// degrees
	} pictypnumdesc;
  // acces funtions
	pictureTypes();
	int picTypeIndex( const char * name );
	const char * picTypeName( int index );
	int picTypeCount( const char * name );
	int picTypeCount( int index );
	QString picTypeDescr( const char * name );
	QString picTypeDescr( int index );
	QStringList picTypeDescrs();
	QSizeF minFov( int index );
	QSizeF maxFov( int index );
  /* convert pvQtPic type id to descriptor index, or -1
    (returns only the primitive file types)
  */
	int picType2Index( pvQtPic::PicType t );
private:
  static pictypnumdesc pictypn[NpictureTypes]; 

};


#endif // __PVQTPIC_H__
