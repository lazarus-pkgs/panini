/* -*- c-basic-offset: 4 -*- */
/*
 * This file is part of the pvQt panoramic viewer
 *  Author: Tom Sharpless <tksharpless@gmail.com>
 * Purpose: adapt the freepv QTVR reader to Qt's QImage class
 * Cautions: 
 *		pulls in a LOT of Qt definitions, watch for conflicts
 *		some of the orignal Image functions are not implemented
 * 
 * 
 * Based on Image.h: part of the freepv panoramic viewer.
 *
 *  Author: Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id: Image.h 118 2007-07-12 23:01:19Z Leonox $
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; version 2.1 of
 * the License
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */

#ifndef FPV_IMAGE_H
#define FPV_IMAGE_H

#include <QImage>
#define Size2D QSize
#define Point2D QPoint

namespace FPV
{
   enum colorChannels{
	   GRAY =  QImage::FormatIndexed8,
	   RGB	=  QImage::FormatRGB888,
	   RGBA	=  QImage::FormatARGB32
   };


class Image
{

    typedef unsigned char T;

public:

	Image() : pqim(0), bpp(0) {}
	~Image(){
		if( pqim ) delete pqim;
	}

	Image(Size2D sz, colorChannels channels = RGB){
		makeQI( sz, channels );
	}

 	/** Set image size.
     *  
     *  Can be used to resize an existing image as well.
     *  In this case, the previous image data is lost
     */
	bool setSize(Size2D size, colorChannels channels = RGB){
		return makeQI( size, channels );
	}
  /** get pointer to raster data.
    QImage row data are always 32-bit aligned, use getRowStride() too!
  */
    unsigned char * getData(){
		if( pqim == 0 ) return 0;
		return pqim->scanLine(0);
    }
  /** get address increment (bytes) per row.
  */
    size_t getRowStride(){   
		if( pqim == 0 ) return 0;
		return pqim->bytesPerLine();   
	}
  /** get the number of bytes per pixel
  */
    int getColorChannels(){
		return bpp;   
	}
  /** get the format code
  */
    colorChannels getType(){
		if( pqim == 0 ) return 0;
		return pqim->format();   
    }
  /* get the image dimensions.
  */
    Size2D size(){   
		if( pqim == 0 ) return Size2D(0, 0);
		return pqim->size();   
	}

  /** access the red pixel component at coordinates \p x and \p y 
    note this won't work for QImages in general, but should here
  */
    unsigned char & operator()(int x, int y) 
    {
        assert(pqim != 0);
        return *(pqim->scanLine(y) + bpp*x);
    }

    /** write image to a ppm file.
     *
     *  Useful for debugging

    void writePPM(std::string file) = 0;

     */


    /** return a subset of an image
	 With QImages this is hard, so not implemented. 

	Image * Image::getSubImage(Point2D pos, Size2D size);

	*/

private:

	QImage * pqim;
	int bpp;

	bool makeQI( Size2D size, colorchannels channels ){
		if( pqim ) delete pqim; pqim = 0;  
		switch( channels ){
		case GRAY: bpp = 1; break;
		case RGB:  bpp = 3; break;
		case RGBA: bpp = 4; break;
		default:   bpp = 0; return false;
		}
		pqim = new QImage(size, channels);
		if( pqim == 0 ) bpp = 0;
		return( pqim != 0 );
	}


// copying is forbidden.
    Image(const Image & other) {};
    void operator=(const Image & other) {};
};


} // namespace

#endif
