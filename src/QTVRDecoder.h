/* -*- c-basic-offset: 4 -*- */
/*
 * This file is part of the freepv panoramic viewer.
 *
 *  Author: Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id: QTVRDecoder.h 64 2006-10-08 10:54:23Z dangelo $
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

/* This file contains a platform independent interface for the
 *  freepv player
 */

#ifndef FPV_QTVR_DECODER_H
#define FPV_QTVR_DECODER_H

#include <vector>

#include "Image.h"
#include "Parameters.h"


namespace FPV
{

    // various typedefs required by the structs copied from various places
    typedef unsigned int uint32;
    typedef int int32;
    typedef short int16;
    typedef unsigned short uint16;

#define MAX_TILE_DIMENSIONS 10                                              // this is the maximum # of tiles x/y that we'll support
#define MAX_TILES_PER_FACE  (MAX_TILE_DIMENSIONS * MAX_TILE_DIMENSIONS)

#define MAX_IMAGE_OFFSETS       (MAX_TILES_PER_FACE*6)

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

/****************************/
/*    PROTOTYPES            */
/****************************/


    QTVRDecoder();
    ~QTVRDecoder();

    bool parseHeaders(const char * theDataFilePath);
    bool extractCubeImages(Image * imgs[6]);
    bool extractCylImage(Image * & img);

    Parameters::PanoType getType() 
    {
        return m_type;
    }

    std::string & getErrorDescr()
    { return m_error; }

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
bool SeekAndExtractImages_Tiled(Image * imgs[6]);
bool SeekAndExtractImagesCyl_Tiled(Image * & img);
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
//                                                      // anything after that is the fast-start track, so we ignore it.
bool gFoundJPEGs;
bool gImagesAreTiled;
int     gNumTilesPerImage;

int32       gPanoChunkOffset;
int32       gPanoSampleSize;
int32       gVideoChunkOffset[MAX_IMAGE_OFFSETS];
int         gVideoSampleSize[MAX_IMAGE_OFFSETS];

int32       gTileSize[MAX_TILES_PER_FACE];

    FILE        *gFile;                             // FILE ref used for fopen().  Not used on Mac since mac uses FSSpec
    FILE * m_mainFile;
    FILE * m_cmovFile;

    bool m_HostBigEndian;

    int32   m_imageRefTrackIndex;
    int32 m_panoType;

    int32 m_panoTracks[MAX_REF_TRACKS];
    int32 m_mainTrack;
    bool m_currTrackIsImageTrack;

    std::vector<SampleToChunkEntry> m_sample2ChunkTable;
    std::string m_error;

    bool m_horizontalCyl;
    bool m_cmovZLib;
    
    // information about the panorama track

    Parameters::PanoType m_type;
};

} // namespace

#endif
