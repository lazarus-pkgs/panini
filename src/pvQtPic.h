
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
  can be built with data already in memory, or subclassed to get the
  necessary information from other sources.  

  Passing a pvQtPic to pvQtView::showPic() starts a picture display
  session, in which the view object is the active partner.  It
  gets information it needs by calling pvQtPic methods, and may 
  also modify the pvQtPic.  
  
  The images passed to view are always QImages, in the size and 
  projection required by the picture type, so probably resized or 
  otherwise transformed from the source images.  Source images are 
  normally read when called for, but may be cached.  
  
  Source image files may be in any format readable by QImageReader.
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
  
 */

#ifndef __PVQTPIC_H__
#define __PVQTPIC_H__

#include <QtGui>


class pvQtPic : public QObject
{	Q_OBJECT
public:

/* Picture type
   implies the number of faces and the preferred image projection
*/
typedef enum {
  nil = 0,		// No picture
  rec = 1,		// A rectilinear image up to 135 x 135 degrees
  sph = 2,		// A spherical image up to 180 degrees diameter
  cyl = 3,		// A cylindrical panorama up to 360 x 135 degrees
  eqr = 4,		// An equirectangular panorama up to 360 x 180 degrees
  cub = 5,		// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
  hem = 6		// A panorama with 1 or 2 hemispherical images
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
 
/* Functions that return displayable images or info about them
*/
 PicType Type();		// nil => cannot display
 int 	 NumFaces();	// max for type, 0 for nil
 int 	 NumImages();	// number of faces that have source images
 QSize   FaceSize();	// pixels w, h (all faces)
 QSizeF  FaceFOV();		// degrees w, h (all faces)
 QImage * FaceImage( PicFace face = front );  // face image
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
 
    
/* functions for programmed setup return true: success, false: failure.
  
  A pvQtPic with type nil cannot be displayed or accept any setup
  calls but setType().  
  
  setType() sets internal state to a legal default for the new type.
  
  All faces are the same size.  The default dimensions are small
  enough to be feasible in any OpenGL implementation.  There 
  is no explicit upper limit on what you can set, however the
  view will probably reduce infeasibly large face sizes. 
  
  The default angular sizes are also the largest ones allowed for 
  the type; for some types this is the only legal value.
  
  Source images must be individually assigned to specific faces, 
  legal for the type. 
  
*/
 bool setType( PicType pt ); 		// clears, sets all defaults
 bool setFaceSize( QSize dims );
 bool setFaceFOV( QSizeF fovs );
 // single source images, by face
 bool setFaceImage( PicFace face, QImage * img );
 bool setFaceImage( PicFace face, int width, int height, void * addr,
 				int bitsPerColor, int colorsPerPixel, 
 				bool floatValues = false,
 			    bool packedPixels = true,
 			    int alignBytes = 0 );
 bool setFaceImage( PicFace face, QString path );
 bool setFaceImage( PicFace face, QUrl url );
 
 /* Set "empty image" styles
   face = any sets all faces; label = "*" uses face names
   label color is black or white according to fill color 
 */
 bool	setLabel( PicFace face = any, QString label = QString("*") );
 bool	setBorder( PicFace face = any, QColor color = QColor() );
 bool	setFill( PicFace face = any, QColor color = QColor() );


private:
	PicType type;
	int maxfaces;	// 1, 2, or 6
	int numimgs;	// no. of faces with source images
  // display face info
  	QImage::Format faceformat;
	QSize		facedims;	// pixels
	QSizeF		facefovs;	// degrees
	QSizeF		maxfovs, minfovs;	// limits
  // arrays indexed by face id, 0:maxfaces-1
	int		 	kinds[6];	// coded source type
	int			formats[6];	// QImage::Format, or a kcode
	QSize		idims[6];	// source dimensions
	void *		addrs[6];	// address if in core
	QUrl		urls[6];	// url if external
	QString		labels[6];	// for empty images...
	QColor		borders[6];
	QColor		fills[6];
 
// load local images for a face
	QImage * loadFile( int face );
	QImage * loadQImage( int face );
	QImage * loadRaster( int face );

 /*  virtual functions for remote images --
	 reimplement in subclasses that can fetch these.
	 gotFaceURL called when non-local url is put in urls[face]. 
	 loadURL called when it is time to fetch same.

  */
	virtual bool gotFaceURL( int face ){ return false; }
	virtual QImage * loadURL( int face ){ return 0; }
};

#endif // __PVQTPIC_H__
