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
	if( !setType( t ) ) setType( nil );
}

bool pvQtPic::setType( pvQtPic::PicType t )
{	
  // set sizes (dims for 'empty' face)
  	bool lockfovs = false;
	switch( t ){
  	case nil:	// No picture
  		maxfaces = 0;
  		facedims = QSize(0,0);
  		facefovs = QSizeF(0,0);
  		lockfovs = true;
  		break;
  	case rec:	// A rectilinear image up to 135 x 135 degrees
  		maxfaces = 1;
  		facedims = QSize(128,128);
  		facefovs = QSizeF(135,135);
  		break;
  	case sph:	// A spherical image up to 180 degrees diameter
  		maxfaces = 1;
  		facedims = QSize(128,128);
  		facefovs = QSizeF(180,180);
  		break;
  	case cyl:	// A cylindrical panorama up to 360 x 135 degrees
  		maxfaces = 1;
  		facedims = QSize(256,128);
  		facefovs = QSizeF(360,135);
  		break;
  	case eqr:	// An equirectangular panorama up to 360 x 180 degrees
  		maxfaces = 1;
  		facedims = QSize(256,128);
  		facefovs = QSizeF(360,180);
  		break;
  	case cub:	// A cubic panorama (1 to 6 rectilinear images 90x90 degrees)
   		maxfaces = 6;
  		facedims = QSize(128,128);
  		facefovs = QSizeF(90,90);
  		lockfovs = true;
 		break;
 	case hem:	// A panorama with 1 or 2 hemispherical images
  		maxfaces = 2;
  		facedims = QSize(128,128);
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
	faceformat = QImage::Format_RGB888; //RGB888
  // angular size limits
  	maxfovs = facefovs;
  	if( lockfovs ) minfovs = facefovs;
	else minfovs = QSizeF( 10, 10 );
  // clear source image info; set empty face style
	static QColor defborders[6] = {
		Qt::red, Qt::green, Qt::blue,
		Qt::cyan, Qt::magenta, Qt::yellow
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
	facedims = dims;
	return true;
}

bool pvQtPic::setFaceFOV( QSizeF fovs )
{
	if( type == nil || fovs.isEmpty() ) return false;
	facefovs = fovs.boundedTo(maxfovs).expandedTo(minfovs);
	return true;
}

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

// single source images, by face
bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QImage * img )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	int i = int(face);
	kinds[i] = QIMAGE_KIND;
	addrs[i] = img;
	formats[i] = img->format();
	idims[i] = img->size();
	urls[i] = QUrl();
	return true;
}

bool pvQtPic::setFaceImage( pvQtPic::PicFace face,
				int width, int height, void * addr,
 				int bitsPerColor, int colorsPerPixel, 
 				bool floatValues, bool packedPixels, int alignBytes )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	int i = int(face);
	kinds[i] = RASTER_KIND;
	addrs[i] = addr;
	formats[i] = kcode(bitsPerColor,colorsPerPixel,
					floatValues, packedPixels, alignBytes );
	idims[i] = QSize(width,height);
	urls[i] = QUrl();
	return true;
}
 			    	
bool pvQtPic::setFaceImage( pvQtPic::PicFace face, QString path )
{
	if( type == nil ) return false;
	if( face < front || face >= PicFace(maxfaces) ) return false;
	QImageReader ir( path );
	if( !ir.canRead() ) return false;
	int i = int(face);
	kinds[i] = FILE_KIND;
	addrs[i] = 0;
	formats[i] = 0;
	idims[i] = ir.size();
	urls[i] = QUrl("file://" + path );
	return true;
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
	idims[i] = QSize();
	urls[i] = url;
	return true;
  // call a virtual function to deal with it
	return gotFaceURL( i );
}


/* Functions that return state info
*/
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

/*  functions that return dispayable images
*/

QImage * pvQtPic::FaceImage( PicFace face ){
	if( type == nil ) return 0;
	if( face < front || face >= PicFace(maxfaces) ) return 0;

	int i = int(face);
	switch( kinds[i] ){
	case QIMAGE_KIND: return loadQImage( i );
	case RASTER_KIND: return loadRaster( i );
	case FILE_KIND:	return loadFile( i );
	case URL_KIND: return loadURL( i );
	}
  // make the empty image for this face	
	QImage * pim = new QImage( facedims + QSize(2,2), faceformat );
  // fill
	QPainter qp( pim );
	QBrush brush( fills[i] );
	QRect box( pim->rect() );
	qp.fillRect( box, brush );
  // draw border
	QPen pen( borders[i], 10 );
	qp.setPen( pen );
	box.adjust(0,0,-10,-10);
	qp.drawRect( box );
  // draw label
	pen.setColor( fills[i].value() < 100 ? Qt::white : Qt::black );
	qp.setPen( pen );
	qp.setFont(QFont("Arial", 18));
	qp.drawText( box, Qt::AlignCenter, labels[i] );

	return pim;
}

QImage * pvQtPic::loadFile( int face )
{
	return 0;
}

QImage * pvQtPic::loadQImage( int face )
{
	return 0;
}

QImage * pvQtPic::loadRaster( int face )
{
	return 0;
}


 


