/* pvQt.h

  pvQt is a panoramic image viewer built on the Qt 4 framework and OpenGL.
  It displays all images in stereographic projection on a spherical screen
  that can be rotated freely.
    
  Class pvQt converts source images to displayable form.  It can get the 
  necessary information from the user with Qt dialogs, or be built from 
  data already in memory.  A picture is displayed by passing a valid pvQt 
  to GLview::showPic().  
  
  Images are typically read from files when called for, but may also come 
  from memory.  The display images are always QImages, that may have been
  resized or otherwise transformed from the source images.
  
  Source image files may be of type jpeg, tiff, png or QTVR. 

  Note: on Windows (Vista32, at least) you must give the c'tor a valid 
  parent widget pointer, else subdialogs tend to crash intermittently

  
 */

#ifndef __PVQT_H__
#define __PVQT_H__
#include <QtGui>
#include "picSpec.h"

class pvQt : public QObject
{	Q_OBJECT
public:
/* Valid c'tor arguments. 
   All but the first 2 are codes for formats pvQt can display
*/
typedef enum {
  ask = -1,	// Ask the user 
  nil,		// No picture
  rec,		// A rectilinear image up to 135 x 135 degrees
  sph,		// A spherical image up to 180 degrees diameter
  cyl,		// A cylindrical panorama up to 360 x 135 degrees
  eqr,		// An equirectangular panorama up to 360 x 180 degrees
  cub,		// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
  hem		// A panorama with 1 or 2 hemispherical images
 } PicType;
 
 static const int FACELIMIT = 6;
 
 pvQt( QWidget * parent = 0 );
~pvQt();
	QWidget * theParent;
 
// functions that return displayable images or info about them
 bool isValid();
 PicType getPicType();
 QSize getNFaces();	// num, max
 QSize getFaceSize();	// pixels
 QSizeF getFOV();	// degrees
/* get a face image, or null pointer.  
   id is a conventional face index, 0:6 for cubic, 0:2 for hemispheric
*/
 QImage * getFace( int id = 0 );

 QString getDir();

// functions for building programmatically
 bool setPicType( PicType pt ); // call first, clears pic
 bool setFaceSize( QSize dims );
 bool setFOV( QSizeF fovs );
 // setFace index is conventional.  Faces not set are skipped.
 bool setFace( int index, QImage * img );
 bool setFace( int index, int width, int height, void * addr,
 				int bitsPerColor, int colorsPerPixel, 
 				bool floatValues = false,
 			    bool packedPixels = true,
 			    int alignBytes = 0 );

public slots:
	void getFiles( int nmax );
	void filePick( int filidx );	
	void facePick( int filidx, int facidx );
	void formatPick( int idx );

private:
	QDir  picDir;		// current image file directory
	QString imageFileFilter;	// supported file extensions
	QFileInfoList filespecs;	// user-picked files
	int file2face[FACELIMIT];	// face ids (0:5) by valid file, or -1
	int face2file[FACELIMIT];	// index in filespecs by face id, or -1
	QList<QSize> validDims;
	int nValidFiles;			// counts same
	QImageReader imgreader;		// called to verify & load image files
	picSpec picspec;	// image file attributes dialog

	PicType type;
	int numfaces;		// <= maxfaces
	int maxfaces;		// 1, 2, or 6
  /* NOTE 
	The types with numfaces > 1 are cubic (1 to 6 faces)and 
	hemispherical (1 or 2).  All faces must have the same display 
	dimensions and fov's, so there is only one ddims and one dfovs.
	But we do admit the possibility of different filed dimensions.
	Note, too that there can be multiple faces but only one file.
  */
  // display face size
	QSize		ddims;	// pixels
	QSizeF		dfovs;	// degrees
  // arrays indexed by face id
	QSize		idims[FACELIMIT];	// source dimensions
	void *		addrs[FACELIMIT];	// address if in-core
	int		 	kinds[FACELIMIT];	// coded source type
	
	QImage *	theImage;

	void setImageFileFilter();
	void clear();
	bool buildInteractive();
	bool loadFile( int faceidx );
	bool loadQImage( QImage * pimg );
	bool loadOther( int kind, void * addr );
	int askFiles( int nmax );
	void likelySetup();
  
 
};

#endif // __PVQT_H__
