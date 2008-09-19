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
  
 */

#ifndef __PVQT_H__
#define __PVQT_H__
#include <QtGui>
class picSpec;

class pvQt : public QObject
{	Q_OBJECT
public:
/* Valid c'tor arguments. 
   All but the first are type codes for things pvQt can display
*/
typedef enum {
  ask = -1,	// Ask the user 
  nil,		// No picture
  rec,		// A rectilinear image up to 135 x 135 degrees
  eqr,		// An equirectangular image up to 360 x 180 degrees
  sph,		// A spherical (or fisheye) image up to 180 degrees diameter
  cub		// A cubic panorama (six rectilinear images of specific angular size)
 } PicType;
 
 pvQt( QWidget * parent = 0 );
~pvQt();
 
// functions that return displayable images or info about them
 bool isValid();
 PicType getPicType();
 int getNumImgs();
 QSize getSize();	// pixels
 QSizeF getFOV();	// degrees
 QImage * getImage( int index = 0 );

 QString getDir();

// functions for building programmatically
 bool setPicType( PicType pt ); // call first, clears pic
 bool setSize( QSize dims );	// sets the displayable size
 bool setFOV( QSizeF fovs );
 bool setImage( int index, QString file );
 bool setImage( int index, QImage * img );
 bool setImage( int index, int width, int height, void * addr,
 				int bitsPerColor, int colorsPerPixel, 
 				bool floatValues = false,
 			    bool packedPixels = true,
 			    int alignBytes = 0 );

// the picture file directory
	QDir  picDir;

	QWidget * theParent;

private:
	picSpec * picspec;
	PicType type;
	int numimgs;
	int numexpected;
	QSize		ddims;
	QSizeF		dfovs;
  // arrays indexed by cube face
	QString		names[6];
	void *		addrs[6];
	QSize		idims[6];
	int		 	kinds[6];
	QImage *	theImage;
  // file specs indexed by file index
	QFileInfoList filespecs;
	int cubeidx[6];	// translate spec index to cube index
	int specidx[6];	// translate cube index to spec index

	void clear();
	bool buildInteractive();
	bool loadFile( QString name );
	bool loadQImage( QImage * pimg );
	bool loadOther( int kind, void * addr );
  
 
};

#endif // __PVQT_H__
