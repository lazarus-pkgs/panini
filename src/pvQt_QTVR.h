
/*	Qt_QTVR.h  for pvQt  03 Oct 2008 TKS
 *
 *  To make QTVRDecoder work with Qt imaging components
 *
 *  Adapted from QTVRDecoder.h, part of the freepv panoramic
 *  viewer by Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id: QTVRDecoder.h 64 2006-10-08 10:54:23Z dangelo $
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
 */

/*  declares a slightly stripped down version of QTVRDecoder.

 */

#ifndef QT_QTVR_H
#define QT_QTVR_H

#include <vector>
#include <QtCore>

/** possible QTVR panorama types  **/
enum PanoType { PANO_UNKNOWN, PANO_CUBIC, PANO_CYLINDRICAL };

// various typedefs required by the structs copied from various places
typedef unsigned int uint32;
typedef int int32;
typedef short int16;
typedef unsigned short uint16;

#define MAX_TILE_DIMENSIONS 10    // this is the maximum # of tiles x/y that we'll support
#define MAX_TILES_PER_FACE  (MAX_TILE_DIMENSIONS * MAX_TILE_DIMENSIONS)

#define MAX_IMAGE_OFFSETS   (MAX_TILES_PER_FACE*6)

#define MAX_REF_TRACKS 10

struct SampleToChunkEntry
{
    int32 startChunk;
    int32 samplesPerChunk;
    int32 sampleDescriptionID;
};


// dangelo: wrap the parser into a class, this makes the parser
// reentrant. Might be important if multiple plugin instances decode a qtvr
// at the same time.
class QTVRDecoder
{

public:

    QTVRDecoder();
    ~QTVRDecoder();

// scan a file
    bool parseHeaders(const char * theDataFilePath);
// get the type of pano it contains
    PanoType getType() { return m_type; }
// get one image (new QImage)
    QImage * getImage( int face = 0 );
// get error message
    const char * getError(){ return m_error; }

private:

long ReadMovieAtom(void);
long ReadQTMovieAtom(void);
void ReadAtom_DCOM(long size);
void ReadAtom_CMVD(long size);
void ReadAtom_STCO(long size);
void ReadAtom_HDLR(int size);
void ReadAtom_STSZ(long size);
void ReadAtom_STSC(long size);
void ReadAtom_TKHD(long size);
void ReadAtom_TREF(long size);
void ReadAtom_QTVR_PDAT(long size);
void ReadAtom_QTVR_TREF(long size);
void ReadAtom_QTVR_CUFA(long size);
bool extractCubeImage(int i, QImage * &img);
bool extractCylImage(QImage * &img);
bool SeekAndExtractImage_Tiled( int i, QImage * &img );
bool SeekAndExtractImageCyl_Tiled( QImage * &img );
void LoadTilesForFace(int chunkNum);
void Swizzle(int32 *value);
void Swizzle(uint32 *value);
void Swizzle(int16 *value);
void Swizzle(uint16 *value);


/*********************/
/*    VARIABLES      */
/*********************/

uint32  gCurrentTrackMedia;                 // 'pano' or ...
//Boolean   gAlreadyGotVideoMedia;          // we assume that the first video media track is what we want,
//                                          // anything after that is the fast-start track, so we ignore it.
bool gFoundJPEGs;
bool gImagesAreTiled;
int  gNumTilesPerImage;

int32       gPanoChunkOffset;
int32       gPanoSampleSize;
int32       gVideoChunkOffset[MAX_IMAGE_OFFSETS];
int         gVideoSampleSize[MAX_IMAGE_OFFSETS];

int32       gTileSize[MAX_TILES_PER_FACE];

    FILE *gFile;
    FILE * m_mainFile;
    FILE * m_cmovFile;

    bool m_HostBigEndian;

    int32   m_imageRefTrackIndex;
    int32 m_panoType;

    int32 m_panoTracks[MAX_REF_TRACKS];
    int32 m_mainTrack;
    bool m_currTrackIsImageTrack;

    std::vector<SampleToChunkEntry> m_sample2ChunkTable;
    char * m_error;

    bool m_horizontalCyl;
    bool m_cmovZLib;

    PanoType m_type;

};

#endif //ndef QT_QTVR_H
