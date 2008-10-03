
/*	QTVRloader.h  for pvQt 03 Oct 2008 TKS
 *
 *  An incremental loader for QTVR panoramas, that signals progress; 
 *  can also be used statically.  If the source is a byte stream, it
 *  must contain only forward pointers.
 *  
 *  Each instance of this class is good for one use only.  It will
 *  accumulate image data internally, and may transiently use large
 *  amounts of buffer space.  You can retrieve either references
 *  to internal images (which will be deleted with the instance)
 *  or new copies (which will not, but use additional memory).
 *
 *  copyright (c) 2008 T K Sharpless
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


#ifndef QTVRLOADER_H
#define QTVRLOADER_H

#include <QtCore>

class QTVRloader : public QObject {
	Q_OBJECT
public:
typedef enum panoType {
	UNKNOWN, CUBIC, HORZ_CYL, VERT_CYL
} panoType;
	QTVRloader( QString path );		// local file
	QTVRloader( QUrl url );			// remote file or stream
	QTVRloader( QBuffer buf );		// custom source
	~QTVRloader();
	bool 	canRead();		// true: source is readable
	bool	isValid();		// false: this loader is dead
	panoType	type();
	bool 	isVertical();
	bool 	isCubic();
	QSize 	size();
	int		faces();	// 1 or 6
	int 	tiles();	// number expected
	int		faceTiles( int face ); // number loaded
// current face image.
	QImage& getFace( int face, QSize size = QSize() ) const;
signals:
	void	haveHeaders( panoType t );  // die if UNKNOWN, 
	void 	faceUpdated( int face, int pctDone );
	void	loadError( QString msg );	// fatal
		
};

#endif //ndef QTVRLOADER_H
