/*
 * pvQt_QTVR.cpp  for pvQt 02Oct2008 TKS
 *
 *	copyright (c) 2008 T K Sharpless <tksharpless@gmail.com>
 *
 * Adapted from the file
 * QTVRDecoder.cpp, part of the freepv panoramic viewer.
 *
 *  Author: Brian Greenstone <brian@pangeasoft.net>
 *
 *  Modified for FreePV by Pablo d'Angelo <pablo.dangelo@web.de>
 *
    Further modified by TKSharpless to use QImage and other Qt
    data types, and to return cube face images one by one instead
    of all at once.  Also simplified and corrected the tiled image
    building code.

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
 */

#include <math.h>
#include <errno.h>
#include <vector>
#include <zlib.h>

#include <QtCore>
#include <QImage>
#include <QImageReader>
#include "pvQt_QTVR.h"

enum colorChannels{
    NOCOLOR = 0,
    GRAY    =  QImage::Format_Indexed8,
    RGB	    =  QImage::Format_RGB888,
    RGBA    =  QImage::Format_ARGB32
};

typedef unsigned long u_long;
typedef bool Boolean;
typedef int32 DWORD;
typedef short WORD;

/*
 * JPEG image decoders for pvQt_QTVR using Qt components.
 *  If rot90 is true, the returned image is rotated 90 degrees clockwise
 * The QImage in the passed Image may get a new size and/or pixel type
 */
bool decodeJPEG( QIODevice * dev, QImage * img, bool rot90 )
{
    QImageReader ir( dev, "JPEG" );
    bool ok = ir.read( img );	// may re-create

    if( !rot90 || !ok ) {
        return ok;
    }

    // copy off the image data to a temp array
    QSize sz = img->size();
    int w = sz.width(), h = sz.height();
    int jr = img->bytesPerLine();
    int bpp = jr / w;	// bytes/pixel
    char * tmp = new char[ h * jr ];
    char * pi = (char *)(img->bits());

    memcpy( tmp, pi, h * jr );
    // copy back rotated
    char * pll = tmp + (h - 1) * jr;

    for( int i = 0; i < h; i++ ){
        char * ps = pll + i * bpp;
        char * pd = pi + i * jr;
        for( int k = h; k > 0; k-- ){
            memcpy( pd, ps, bpp );
            pd += bpp;
            ps -= jr;
        }
    }

    delete [] tmp;
    return ok;
}

// source is a buffer
bool decodeJPEG( char * buffer, size_t buf_len, QImage & image, bool rot90=false)
{
    QByteArray qb( buffer, int(buf_len) );
    QBuffer buf(&qb);
    return decodeJPEG( &buf, &image, rot90 );
}

// source is an open file
bool decodeJPEG( FILE *f, QImage & image, bool rot90=false){
    QFile qf;
    qf.open( f, QIODevice::ReadOnly );
    return decodeJPEG( &qf, &image, rot90 );
}

struct ChunkOffsetAtom
{
    int32 size;
    int32 type;
    char version;
    char flags[3];
    int32 numEntries;
    int32 chunkOffsetTable[200];
};

struct SampleSizeAtom
{
    int32 size;
    int32 type;
    char version;
    char flags[3];
    int32 sampleSize;
    int32 numEntries;
    int32 sampleSizeTable[200];
};

struct PublicHandlerInfo
{
    int32 componentType;
    int32 componentSubType;
};

struct HandlerAtom
{
    int32 size;
    int32 type;
    char version;
    char flags[3];
    PublicHandlerInfo hInfo;
};

struct QTVRCubicViewAtom
{
    float minPan;
    float maxPan;
    float minTilt;
    float maxTilt;
    float minFieldOfView;
    float maxFieldOfView;
    float defaultPan;
    float defaultTilt;
    float defaultFieldOfView;
};

// 'pdat' atom
struct VRPanoSampleAtom
{
    WORD majorVersion;
    WORD minorVersion;
    DWORD imageRefTrackIndex;
    DWORD hotSpotRefTrackIndex;
    float minPan;
    float maxPan;
    float minTilt;
    float maxTilt;
    float minFieldOfView;
    float maxFieldOfView;
    float defaultPan;
    float defaultTilt;
    float defaultFieldOfView;
    DWORD imageSizeX;
    DWORD imageSizeY;
    WORD imageNumFramesX;
    WORD imageNumFramesY;
    DWORD hotSpotSizeX;
    DWORD hotSpotSizeY;
    WORD hotSpotNumFramesX;
    WORD hotSpotNumFramesY;
    DWORD flags;
    DWORD panoType;
    DWORD reserved2;
};

// 'cube'
#define kQTVRCube 'cube'
//0x65627563

// 'hcyl'
#define kQTVRHorizontalCylinder 'hcyl'
//0x6C796368

// 'vcyl'
#define kQTVRVerticalCylinder 'vcyl'
//0x6C796376

struct QTVRCubicFaceData
{
    float orientation[4];
    float center[2];
    float aspect;
    float skew;
};

struct QTVRTrackRefEntry
{
    uint32  trackRefType;
    uint16  trackResolution;
    uint32  trackRefIndex;
} ;

#define DECOMP_CHUNK 4096

/*
 * Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.height() and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files.
*/
int decompressZLIBFile(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[DECOMP_CHUNK];
    unsigned char out[DECOMP_CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        return ret;
    }

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, DECOMP_CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = DECOMP_CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);

            ///assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            if(ret == Z_STREAM_ERROR){
                qFatal("pvQt_QTVR::Z_STREAM_ERROR");
            }

            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR; /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }

            have = DECOMP_CHUNK - strm.avail_out;

            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/**************** PARSE QUICKTIME MOVIE ***********************/
// Parses through the QT movie's atoms looking for our 6 cube textures.

QTVRDecoder::QTVRDecoder()
{
    gCurrentTrackMedia = 0;
    //gAlreadyGotVideoMedia = false;
    gFoundJPEGs = false;
    m_imageRefTrackIndex = 0;
    m_panoType = 0;
    m_mainTrack = 0;
    m_currTrackIsImageTrack = false;
    m_type = PANO_UNKNOWN;
    m_cmovZLib = false;

    // determine byteorder
    int testint = 0x01;
    unsigned char * testchar = reinterpret_cast< unsigned char * >(&testint);

    if ( testchar[0] == 0x01 ) {
        m_HostBigEndian = false;
    } else {
        m_HostBigEndian = true;
    }
}

QTVRDecoder::~QTVRDecoder()
{
    if(gFile) {
        fclose(gFile);
    }
}

bool QTVRDecoder::parseHeaders(const char * theDataFilePath)
{
    bool ok = true;
    int32 atomSize;
    gFile = fopen (theDataFilePath, "rb");

    if  (!gFile)
    {
        m_error = "fopen() failed";
        return false;
    }

    m_mainFile = gFile;

    // get file size for EOF test
    size_t filepos = ftell( gFile );
    fseek( gFile, 0, SEEK_END );
    size_t filesize = ftell( gFile );
    fseek( gFile, filepos, SEEK_SET );

    // Recurse through atoms
    m_error = 0;
    do	{
        atomSize = ReadMovieAtom();
    } while(atomSize > 0 && ftell( gFile ) < filesize );

    if (m_error != 0) {
        return false;
    }

    //gMyInstances[instanceNum].drawDecodingBar = false;

    return(ok);
}

long QTVRDecoder::ReadQTMovieAtom(void)
{
    int32   atomSize, atomType,  remainingSize;
    int16   childCount;
    size_t  filePos;
    //char    *c = (char *)&atomType;
    //OSErr iErr;

    filePos = ftell(gFile);

    // read atom size
    size_t sz = fread(&atomSize, 1, 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadQTMovieAtom:  fread() failed!";
        return(-1);
    }

    // read the atom type
    sz = fread(&atomType, 1, 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadQTMovieAtom:  fread() failed!";
        return(-1);
    }

    // skip id and reserved
    fseek(gFile, 6, SEEK_CUR);

    sz = fread(&childCount, 1, 2, gFile);
    if (ferror(gFile) || sz != 2)
    {
        m_error = "ReadQTMovieAtom:  fread() failed!";
        return(-1);
    }

    // skip reserved
    fseek(gFile, 4, SEEK_CUR);

    // convert BigEndian data to LittleEndian
    Swizzle(&atomSize);
    Swizzle(&atomType);
    Swizzle(&childCount);

    // read extended data if needed

    // if atom size == 1 then there's extended data in the header
    if (atomSize == 1)
    {
        m_error = "ReadQTMovieAtom: Extended size isn't supported";
        return(-1);
    }

    /********************/
    /* HANDLE THIS ATOM */
    /********************/
    // Any atom types not in the table below just get skipped.

    switch(atomType)
    {
    case 'sean':
        // recurse into sub atoms

        // there are n bytes left in this atom to parse
        remainingSize = atomSize - 20 ;

/*
        do
        {
            remainingSize -= ReadMovieAtom();           // read atom and dec by its size
        } while(remainingSize > 0);
*/
        for (int i=0; i < childCount; i++) {
            remainingSize -= ReadQTMovieAtom();
        }

        break;
    case 'tref':
        ReadAtom_QTVR_TREF(atomSize-20);
        break;
    case 'pdat':
        ReadAtom_QTVR_PDAT(atomSize-20);
        break;
    }

    //if (iErr != noErr)
    //  return(-1);

    /*************************/
    /* SET FPOS TO NEXT ATOM */
    /*************************/

    // if last atom size was 0 then that was the end
    if (atomSize == 0)
    {
        return(-1);
    } else {
        int r = fseek(gFile, (long)filePos + atomSize, SEEK_SET);
        //if (ferror(gFile) || r != 0)
        //printf("ReadQTMovieAtom: fseek() failed, probably EOF?\n");
    }

    return(atomSize);
}

/********************* READ MOVIE ATOM ************************/
// INPUT:  parentAtomSize = size of the enclosing atom (or -1 if root).
// This is used to determine when we've read all the child atoms from
// the enclosing parent that we sub-recursed.
// OUTPUT:	size of the atom
long QTVRDecoder::ReadMovieAtom(void)
{
    int32	atomSize, atomType,  remainingSize;
    size_t	filePos;
    //char	*c = (char *)&atomType;
    //OSErr	iErr;

    filePos = ftell(gFile);

    // read atom size
    size_t sz = fread(&atomSize, 1, 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadMovieAtom:  fread() failed!";
        return(-1);
    }

    // read atom type
    sz = fread(&atomType, 1, 4, gFile);
    if (ferror(gFile) || sz != 4) {
        m_error = "ReadMovieAtom:  fread() failed!";
        return(-1);
    }

    // convert BigEndian data to LittleEndian
    Swizzle(&atomSize);
    Swizzle(&atomType);

    // read extended data
    // if atom size == 1 then there's extended data in the header
    if (atomSize == 1)
    {
        m_error = "ReadMovieAtom: Extended size isn't supported";
        return(-1);
    }

    /********************/
    /* HANDLE THIS ATOM */
    /********************/
    // Any atom types not in the table below just get skipped.
    switch(atomType)
    {
    // MOOV
    // This contains more sub-atoms, so just recurse
    case 'moov':
        // there are n bytes left in this atom to parse
        remainingSize = atomSize - 8;
        do
        {
            // read atom and dec by its size
            remainingSize -= ReadMovieAtom();
        }
        while(remainingSize > 0);
        break;
    // CMOV
    // This contains more sub-atoms, so just recurse
    case    'cmov':
        //printf("  [Subrecursing 'cmov' atom]\n");

        // there are n bytes left in this atom to parse
        remainingSize = atomSize - 8;
        do
        {
            // read atom and dec by its size
            remainingSize -= ReadMovieAtom();
        }

        while(remainingSize > 0);

        //printf("  [End subrecurse 'cmov' atom]\n");

        break;
    case 'dcom':
        ReadAtom_DCOM(atomSize-8);
        break;
    case 'cmvd':
        ReadAtom_CMVD(atomSize-8);
        break;
    // trak
    case 'trak':
        //printf("  [Subrecursing 'trak' atom]\n");

        // reset per track information
        m_currTrackIsImageTrack = false;
        gCurrentTrackMedia = 0;

        // there are n bytes left in this atom to parse
        remainingSize = atomSize - 8;
        do
        {
            // read atom and dec by its size
            remainingSize -= ReadMovieAtom();
        }
        while(remainingSize > 0);

        //printf("  [End subrecurse 'trak' atom]\n");
        break;
    case 'tkhd':
        ReadAtom_TKHD(atomSize-8);
        break;
    case 'tref':
        ReadAtom_TREF(atomSize-8);
        break;

    case 'mdia':
        //printf("  [Subrecursing 'mdia' atom]\n");

        remainingSize = atomSize - 8;
        do
        {
            remainingSize -= ReadMovieAtom();
        }
        while(remainingSize > 0);
        //printf("  [End subrecurse 'mdia' atom]\n");
        break;
    case 'minf':

        //printf("  [Subrecursing 'minf' atom]\n");

        remainingSize = atomSize - 8;
        do
        {
            remainingSize -= ReadMovieAtom();
        }
        while(remainingSize > 0);

        //printf("  [End subrecurse 'minf' atom]\n");

        break;
    //DataInfoAID
    case 'dinf':
        //printf("  [Subrecursing 'dinf' atom]\n");
        ReadMovieAtom();
        //printf("  [End subrecurse 'dinf' atom]\n");
        break;
    //SampleTableAID
    case 'stbl':
        //printf("  [Subrecursing 'stbl' atom]\n");
        remainingSize = atomSize - 8;					// there are n bytes left in this atom to parse

        do
        {
            remainingSize -= ReadMovieAtom();			// read atom and dec by its size
        }
        while(remainingSize > 0);

        //printf("  [End subrecurse 'stbl' atom]\n");
        break;
    // STChunkOffsetAID
    case 'stco':
        ReadAtom_STCO(atomSize);
        break;
    //STSampleSizeAID
    case 'stsz':
        ReadAtom_STSZ(atomSize);
        break;
    case 'stsc':
        ReadAtom_STSC(atomSize);
        break;
    // HandlerAID
    case 'hdlr':
        ReadAtom_HDLR(atomSize);
        break;
    }

    //if (iErr != noErr)
    //	return(-1);

    // if last atom size was 0 then that was the end
    if (atomSize == 0)
    {
        return(-1);
    }
    else
    {
        // set fpos to next atom
        fseek(gFile, (long)filePos + atomSize, SEEK_SET);
        //if (ferror(gFile))
        //printf("ReadMovieAtom: fseek() failed, probably EOF?\n");
    }

    return(atomSize);
}

void QTVRDecoder::ReadAtom_DCOM(long size)
{
    char comp[5];
    comp[4] = 0;
    //int32 count;
    //HandlerAtom *atom;
    //PublicHandlerInfo *info;
    //int32 componentSubType;

    size_t sz = fread(&comp,1 , 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadAtom_DCOM:  fread() failed!\n";
        return;
    }

    if (strcmp(comp, "zlib") != 0 ) {
        m_error = "unsupported header compression scheme ";
        return;
    }
    m_cmovZLib = true;
}


/********************** READ ATOM:  CMVD****************************/

void QTVRDecoder::ReadAtom_CMVD(long size)
{
    int32 uncomp_size;

    size_t sz = fread(&uncomp_size,1 , 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadAtom_CMVD:  fread() failed!\n";
        return;
    }
    size -= (int)sz;

    if (m_cmovZLib) {
        // decompress compressed header
        // create temporary file
        FILE * decomp = tmpfile();
        if (! decomp) {
            // error
            m_error = "Could not open temporary file for header decompression";
            return;
        }

        if (decompressZLIBFile(gFile, decomp) != Z_OK) {
            m_error = "zlib decompression failed";
            fclose(decomp);
            return;
        }

        // restart parser on now decompressed header.
        fseek(decomp,0,SEEK_SET);
        m_mainFile = gFile;
        m_cmovFile = decomp;

        gFile = m_cmovFile;

        // recurse through atoms
        int atomSize;
        do
        {
            atomSize = ReadMovieAtom();
        }while(atomSize > 0);

        // switch back to main file
        gFile = m_mainFile;

        // close header file
        fclose(m_cmovFile);
    }
}

void QTVRDecoder::ReadAtom_STCO(long size)
{
    //int32 count;
    int i;
    ChunkOffsetAtom	*atom;
    int numEntries;

    fseek(gFile, -8, SEEK_CUR);

    // allocate memore for it
    // This is a variable size structure, so we need to allocated based on the size of the atom that's passed in
    atom = (ChunkOffsetAtom *) malloc(size);
    if (atom == NULL)
    {
        m_error = "ReadAtom_STCO:  malloc() failed!";
        return;
    }

    // read the atom
    fread(atom, size, 1, gFile);
    if (ferror(gFile))
    {
        m_error = "ReadAtom_STCO:  fread() failed!";
        free(atom);
        return;
    }


    // see what kind of track we've parsed into and get chunks based on taht
    numEntries = atom->numEntries;

    // convert BigEndian data to LittleEndian (if not Mac)
    Swizzle(&numEntries);

    switch(gCurrentTrackMedia)
    {
    case 'pano':
    {
        gPanoChunkOffset = atom->chunkOffsetTable[0];
        Swizzle(&gPanoChunkOffset);
        //printf("        Chunk offset to 'pano' is : %d\n", gPanoChunkOffset);
        // TODO: parse the pano info atom here!
        long fpos_saved = ftell(gFile);

        bool switchChunk = (m_cmovFile == gFile);
        // the pano chunk is always stored in the main file..
        if (switchChunk) {
            gFile = m_mainFile;
        }

        // seek to panorama description. skip first 12 bytes
        // of new QT atom.
        fseek(gFile, gPanoChunkOffset + 12, SEEK_SET);

        //printf("  [Subrecursing pano 'stco' atom]\n");

        // there are n bytes left in this atom to parse
        size_t remainingSize = gPanoSampleSize-12;
        do
        {
            // read atom and dec by its size
            remainingSize -= ReadQTMovieAtom();
        }
        while(remainingSize > 0);
        //printf("  [End subrecurse pano 'stco' atom]\n");

        // switch back to compressed file, if needed
        if (switchChunk) {
            gFile = m_cmovFile;
        }

        fseek(gFile, fpos_saved, SEEK_SET);
        // reset this now
        gCurrentTrackMedia = 0;
    }
        break;
    case 'vide':
        // if this is the main track, extract the images
        if (m_currTrackIsImageTrack)
        {
        // extract the offsets to the images
#if 0
            for (i = 0; i < numEntries; i++)
            {
                ////		printf("       # Chunk Offset entries: %d\n", numEntries);

                for (i = 0; i < numEntries; i++)
                {
                    gVideoChunkOffset[i] = atom->chunkOffsetTable[i];
                    Swizzle(&gVideoChunkOffset[i]);							// convert BigEndian data to LittleEndian (if not Mac)
                    ////			printf("       Chunk offset #%d = %d\n", i, gVideoChunkOffset[i] );
                }
                gCurrentTrackMedia = 0;											// reset this now!
                break;
            }
#endif
            // consider the sample to chunk atom when writing the offsets
            m_sample2ChunkTable[0];
            int sampleTableId = 0;
            int chunkId = 0;
            int32 sampleOffset = atom->chunkOffsetTable[0];
            Swizzle(&sampleOffset);
            // count sample in current chunk
            int samplesInChunk=0;
            for (i=0; i < gNumTilesPerImage*6; i++) {
                // check if we have reached the end of the current Chunk
                if (m_sample2ChunkTable[sampleTableId].samplesPerChunk == samplesInChunk) {
                    // we have reached a new chunk
                    chunkId++;
                    samplesInChunk = 0;
                    if (sampleTableId < ((int)m_sample2ChunkTable.size())-1) {
                        // check if we have reached a sample to chunk table entry
                        if (chunkId+1 == m_sample2ChunkTable[sampleTableId+1].startChunk) {
                            // advance to next entry in sample table
                            sampleTableId++;
                        }
                    } else {
                        // we are in the last entry of the sample to chunktable, no need to check for a new entry
                    }
                    // update chunk offset
                    sampleOffset = atom->chunkOffsetTable[chunkId];
                    Swizzle(&sampleOffset);
                }
                gVideoChunkOffset[i] = sampleOffset;
                // advance to next sample
                sampleOffset += gVideoSampleSize[i];
                samplesInChunk++;
            }
            gCurrentTrackMedia = 0;
        }
    }

    //bail:
    free(atom);
}

void QTVRDecoder::ReadAtom_STSZ(long size)
{
    //int32	count;
    SampleSizeAtom *atom;
    int32 numEntries, i;

    fseek(gFile, -8, SEEK_CUR);

    // This is a variable size structure, so we need to allocated based on the size of the atom that's passed in
    atom = (SampleSizeAtom *) malloc(size);
    if (atom == NULL)
    {
        m_error = "ReadAtom_STSZ:  malloc() failed!";
        return;
    }

    fread(atom, size, 1, gFile);
    if (ferror(gFile))
    {
        m_error = "ReadAtom_STSZ:  fread() failed!";
        free(atom);
        return;
    }

    // see what kind of track we've parsed into and get chunks based on that
    numEntries = atom->numEntries;
    Swizzle(&numEntries);

    switch(gCurrentTrackMedia)
    {
    case 'pano':
        gPanoSampleSize = atom->sampleSize;
        Swizzle(&gPanoSampleSize);
        //printf("        'pano' sample size = : %d\n", gPanoSampleSize);
        break;
    case'vide':
        //printf("       # Sample Size entries: %d\n", numEntries);

        if (m_currTrackIsImageTrack)
        {
            if (m_type == PANO_CUBIC) {
                // there MUST be at least 6 jpegs or this isn't a cube
                if (numEntries < 6)
                {
                    m_error="cubic panorama with less than 6 images";
                    free(atom);
                    return;
                }

                gFoundJPEGs = true;


                /* ARE THE IMAGES TILED? */
                gNumTilesPerImage = numEntries / 6;

                if (gNumTilesPerImage > 1)
                {
                    //printf("_____ There are more than 6 entires in the 'vide' track, so this QTVR has tiled images!\n");
                    gImagesAreTiled = true;
                    // are there too many tiles?
                    if (numEntries > MAX_IMAGE_OFFSETS)
                    {
                        m_error = "Too many tiles!";
                        free(atom);
                        return;
                    }
                }
                else
                {
                    gImagesAreTiled = false;
                }
            } else {
                // cylindrical pano
                gFoundJPEGs = true;
                gNumTilesPerImage = numEntries;
                if (gNumTilesPerImage > 1)
                {
                    //printf("_____ There are more than 1 entires in the 'vide' track, so this QTVR has a tiled image!\n");
                    gImagesAreTiled = true;
                    // are there too many tiles?
                    if (numEntries > MAX_IMAGE_OFFSETS)
                    {
                        m_error = "Too many tiles!";
                        free(atom);
                        return;
                    }
                }
                else
                {
                    gImagesAreTiled = false;
                }
            }
            for (i = 0; i < numEntries; i++)
            {
                gVideoSampleSize[i] = atom->sampleSizeTable[i];
                Swizzle(&gVideoSampleSize[i]);
                //printf("       sample size %d = %d\n", i, gVideoSampleSize[i] );
            }
            break;
        }
    }
    free(atom);
}

void QTVRDecoder::ReadAtom_STSC(long size)
{
    int32 numEntries;

    // skip version and flags
    size_t sz = fread(&numEntries,1 , 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadAtom_STSC:  fread() failed!";
        return;
    }

    // read number of entries
    sz = fread(&numEntries,1 , 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadAtom_STSC:  fread() failed!";
        return;
    }
    Swizzle(&numEntries);

    // discart old sample table
    m_sample2ChunkTable.clear();
    for(int i=0; i < numEntries; i++)
    {
        SampleToChunkEntry tmp;
        sz = fread(&tmp,1 , 12, gFile);
        if (ferror(gFile) || sz != 12)
        {
            m_error = "ReadAtom_STSC:  fread() failed!";
            return;
        }
        Swizzle(&tmp.startChunk);
        Swizzle(&tmp.samplesPerChunk);
        Swizzle(&tmp.sampleDescriptionID);
        m_sample2ChunkTable.push_back(tmp);
    }
}

void QTVRDecoder::ReadAtom_HDLR(int size)
{
    //int32	count;
    HandlerAtom	*atom;
    PublicHandlerInfo *info;
    int32 componentSubType;

    fseek(gFile, -8, SEEK_CUR);

    // This is a variable size structure, so we need to allocated based on the size of the atom that's passed in
    atom = (HandlerAtom *) malloc(size);
    if (atom == NULL)
    {
        m_error = "ReadAtom_HDLR:  malloc() failed!";
        return;
    }

    fread(atom, size, 1, gFile);
    if (ferror(gFile))
    {
        m_error = "ReadAtom_HDLR:  fread() failed!";
        free(atom);
        return;
    }

    // point to handler info
    info = &atom->hInfo;

    // get comp sub type
    componentSubType = info->componentSubType;
    Swizzle(&componentSubType);
    char * t = (char*) & componentSubType;

    if (componentSubType == 'pano')
    {
        //printf("ReadAtom_HDLR:  We found the 'pano' media!\n");
        gCurrentTrackMedia = 'pano';
    }
    else
        if (componentSubType == 'vide')
        {
            gCurrentTrackMedia = 'vide';
            //printf("ReadAtom_HDLR:  We found a 'vide' media!\n");
#if 0
            //		if (!gAlreadyGotVideoMedia)					// if we already got the 'vide' then this one is just the fast-start track, so ignore it!
            {
                gCurrentTrackMedia = 'vide';
                //			gAlreadyGotVideoMedia = true;
            }
            else
            //printf("Found an additional 'vide' media track, but we're going to ignore it since it's probably the fast-start track...\n");
#endif
        }

    free(atom);
}

void QTVRDecoder::ReadAtom_TKHD(long size)
{
    int32 trackid;

    int ret = fseek(gFile, 12, SEEK_CUR);
    if ( ret != 0 )
    {
        m_error = "ReadAtom_TKHD:  fseek() failed!";
        return;
    }

    size_t sz = fread(&trackid,1 , 4, gFile);
    if (ferror(gFile) || sz != 4)
    {
        m_error = "ReadAtom_TKHD:  fread() failed!";
        return;
    }
    Swizzle(&trackid);

    if (trackid == m_mainTrack) {
        // this is the main pano track.
        m_currTrackIsImageTrack = true;
    }
}

void QTVRDecoder::ReadAtom_TREF(long size)
{
    int32 subsize;
    int32 type;
    int32 track;
    //int32 count;
    //HandlerAtom *atom;
    //PublicHandlerInfo *info;
    //int32 componentSubType;

    // loop until everything has been read
    while(size) {
        size_t sz = fread(&subsize,1 , 4, gFile);
        if (ferror(gFile) || sz != 4)
        {
            m_error = "ReadAtom_TREF:  fread() failed!";
            return;
        }
        Swizzle(&subsize);
        subsize -=sz;
        size -= sz;
        sz = fread(&type,1 , 4, gFile);
        if (ferror(gFile) || sz != 4)
        {
            m_error = "ReadAtom_TREF:  fread() failed!";
            return;
        }
        Swizzle(&type);
        subsize -=sz;
        size -= sz;

        //int nRefs = (size)/4;

        // only store tracks of type imgt, since these provide the image tracks.
        int i=0;
        if (type == 'imgt') {
            while(subsize) {
                sz = fread(&track,1 , 4, gFile);
                if (ferror(gFile) || sz != 4)
                {
                    m_error = "ReadAtom_TREF:  fread() failed!";
                    return;
                }
                subsize -= sz;
                size -= sz;
                Swizzle(&track);
                if (i < MAX_REF_TRACKS)
                    m_panoTracks[i++] = track;
                else {
                    m_error = "Too many reference tracks!";
                }
            }
        } else {
            // seek to next atom
            int ret = fseek(gFile, subsize, SEEK_CUR);
            if (ret != 0)
            {
                m_error = "ReadAtom_TREF:  fread() failed!";
                return;
            }
            size -= sz;
            subsize -= sz;
        }
    }
}

void QTVRDecoder::ReadAtom_QTVR_PDAT(long size)
{
    //int32 count;
    VRPanoSampleAtom *atom;
    //int32 numEntries, i;

    // This is a variable size structure, so we need to allocated based on the size of the atom that's passed in
    atom = (VRPanoSampleAtom *) malloc(size);
    if (atom == NULL)
    {
        m_error = "ReadAtom_QTVR_PDAT:  malloc() failed!";
        return;
    }

    /* READ IT */

    size_t sz = fread(atom, size, 1, gFile);
    if (ferror(gFile) || sz != 1)
    {
        m_error = "ReadAtom_PDAT:  fread() failed!";
        free(atom);
        return;
    }

    // see what kind of track we've parsed into and get chunks based on that

    // check if this is a cubic panorama
    m_panoType = atom->panoType;
    Swizzle(&m_panoType);
    char *t = (char*)&m_panoType;

    if (m_panoType == kQTVRCube) {
        m_type = PANO_CUBIC;
    } else if (m_panoType == 'hcyl' ){
        m_type = PANO_CYLINDRICAL;
        // orientation of panorama.
        m_horizontalCyl = true;
    } else if (m_panoType == 'vcyl' ){
        m_type = PANO_CYLINDRICAL;
        // orientation of panorama.
        m_horizontalCyl = false;
    } else if (m_panoType == 0 ) {
        // old QT format, orientation stored in flags
        m_horizontalCyl = (atom->flags & 1);
        m_type = PANO_CYLINDRICAL;
    }

    // get the track number of the real pano
    m_imageRefTrackIndex = atom->imageRefTrackIndex;
    Swizzle(&m_imageRefTrackIndex);
    m_mainTrack = m_panoTracks[m_imageRefTrackIndex -1];

    free(atom);
}

void QTVRDecoder::ReadAtom_QTVR_TREF(long size)
{
    QTVRTrackRefEntry atom;

    int n=size/10;

    for (int i=0; i < n; i++)
    {
        fread(&(atom.trackRefType), 1,4, gFile);
        fread(&atom.trackResolution, 1,2, gFile);
        fread(&atom.trackRefIndex, 1,4, gFile);
        Swizzle(&(atom.trackRefType));
        Swizzle(&(atom.trackResolution));
        Swizzle(&(atom.trackRefIndex));
        //printf("track %d: refType: %d  Resolution: %d  Index: %d\n", i, atom.trackRefType, atom.trackResolution, atom.trackRefIndex);
    }
}


// seek and extract images
// get the image for cube face iim
bool QTVRDecoder::extractCubeImage(int iim, QImage * &cubeface)
{
    if (m_type != PANO_CUBIC) {
        m_error = "not a cubic panorama";
        return false;
    }

    // see if we need to special-case tiled images
    if (gImagesAreTiled)
    {
        return SeekAndExtractImage_Tiled(iim, cubeface);
    }

    if (!gFoundJPEGs)
    {
        m_error = "Missing cubic images";
        return false;
    }

    int i = iim;
    // seek to the JPEG's data
    fseek(gFile, gVideoChunkOffset[i], SEEK_SET);

    cubeface = new QImage;
    if (!decodeJPEG(gFile, *cubeface)) {
        m_error = "JPEG decoding failed";
        delete cubeface;
        cubeface = 0;
        return false;
    }

    return true;
}

// Seek and extract the image from a cylindrical pano
bool QTVRDecoder::extractCylImage(QImage * &img)
{
    if (m_type != PANO_CYLINDRICAL) {
        m_error = "not a cylindrical panorama";
        return false;
    }

    if (gImagesAreTiled)
    {
        return SeekAndExtractImageCyl_Tiled(img);
    }

    if (!gFoundJPEGs)
    {
        m_error = "No usable JPEG image found";
        return false;
    }

    // seek to the JPEG's data
    fseek(gFile, gVideoChunkOffset[0], SEEK_SET);

    img = new QImage;
    // decode jpeg, and rotate 90 CW, if required
    if (!decodeJPEG( gFile, *img, (!m_horizontalCyl))) {
        m_error = "JPEG decoding failed";
        delete img;
        img = 0;
        return false;
    }

    return true;
}

// seek and extract images: tiled
bool QTVRDecoder::SeekAndExtractImage_Tiled(int i, QImage * &cubeface)
{
    int tileDimensions; //, compSize;
    int faceSize;

    //// TKS: this is really cheesy
    tileDimensions = (int) sqrt((float)gNumTilesPerImage);

    int chunkNum = i * gNumTilesPerImage;

    // init tile assembly
    cubeface = 0;
    int tileSize = 0;

    // load and assemble tiles.
    for (int t = 0; t < gNumTilesPerImage; t++)
    {
        int cChunk = chunkNum + t;
        fseek(gFile, gVideoChunkOffset[cChunk], SEEK_SET);
        if (ferror(gFile))
        {
            m_error = "LoadTilesForFace:  fseek failed!";
            return false;
        }
        QImage img;
        // decode jpg
        if (!decodeJPEG(gFile, img))
        {
            m_error = "JPEG decoding failed";
            return false;
        }

        if (tileSize == 0) {
            // size cube face and create image
            tileSize = img.size().width();
            faceSize = tileSize * tileDimensions;
            cubeface = new QImage(faceSize, faceSize, img.format());
        }

        if (img.size().height() != img.size().width()) {
            m_error = "non square tile";
            return false;
        }

        if (img.size().width() != tileSize) {
            m_error = "tiles of different sizes";
            return false;
        }

        // copy to cube face
        // for QImage must use row strides...
        int tilerowsize = img.bytesPerLine();
        int facerowsize = cubeface->bytesPerLine();
        // (and let's use the actual pixel size too)
        int bpp = facerowsize / faceSize;
        // tile indices
        int h = t % tileDimensions;
        int v = t / tileDimensions;
        // byte addrs (incl alignment)
        unsigned char * srcPtr = img.bits();
        unsigned char * destPtr = cubeface->bits()			// raster
                + facerowsize * tileSize * v	// row
                + bpp * tileSize * h;			// col

        // loop over rows
        for (int y=0; y< tileSize; y++)
        {
            memcpy(destPtr, srcPtr, bpp*tileSize);
            destPtr += facerowsize;
            srcPtr += tilerowsize;
        }
    }

    return true;
}

bool QTVRDecoder::SeekAndExtractImageCyl_Tiled(QImage * &image)
{

    // init tile assembly
    if (image) {
        delete image;
    }
    image = 0;
    QSize tileSize;

    // load and assemble tiles.
    for (int t = 0; t < gNumTilesPerImage; t++)
    {
        fseek(gFile, gVideoChunkOffset[t], SEEK_SET);
        if (ferror(gFile))
        {
            m_error = "LoadTilesForFace:  fseek failed!";
            return false;
        }

        QImage tile;
        // decode jpg
        if (!decodeJPEG(gFile, tile, (!m_horizontalCyl))) {
            m_error = "JPEG decoding failed";
            return false;
        }

        // creat target image at first tile
        int imgw, imgh, tw, th;
        unsigned int bpp, imgbpl, tilebpl, tilebtc;
        if (tileSize.isEmpty()) {
            tileSize = tile.size();
            tw = tileSize.width();
            imgw = tw * gNumTilesPerImage;
            th = tileSize.height();
            imgh = th;
            image = new QImage(imgw, imgh, tile.format());
            imgbpl = image->bytesPerLine();
            tilebpl = tile.bytesPerLine();
            bpp = tilebpl / tw;	// bytes per pixel
            tilebtc = tw * bpp;	// bytes to copy
        }
        if (tile.size().width() != tw || tile.size().height() != th ) {
            // jpeg image size doesn't correspond to tile size
            m_error = "Tiles with different size found";
            return false;
        }

        //add tile to image
        //left-to right for horizontal fmt,
        //right-to-left for vertical fmt.
        int left=0;
        int right=0;
        if (m_horizontalCyl) {
            left    = t * tw;
            right   = left + tw;
        } else {
            left    = image->size().width() - (t+1) * tw;
            right   = left + tw;
        }
        unsigned char * srcPtr = tile.bits();
        unsigned char * destPtr = image->bits() + bpp * left;
        for (int y = 0; y < th; y++ ){
            memcpy(destPtr, srcPtr, tilebtc );
            destPtr += imgbpl;
            srcPtr += tilebpl;
        }
    }
    return true;
}

// Converts a 4-byte int to reverse the Big-Little Endian order of the bytes.
//
// Quicktime movies are in BigEndian format, so when reading on a PC or other
// Little-Endian machine, we've got to swap the bytes around.
void QTVRDecoder::Swizzle(uint32 *value)
{
    Swizzle((int32*) value);
}

void QTVRDecoder::Swizzle(int32 *value)
{
    if (!m_HostBigEndian) {
        char	*n, b1,b2,b3,b4;
        n = (char *)value;

        // get the 4 bytes
        b1 = n[0];
        b2 = n[1];
        b3 = n[2];
        b4 = n[3];

        // save in reverse order
        n[0] = b4;
        n[1] = b3;
        n[2] = b2;
        n[3] = b1;
    }
}

// convert 16 bit values to native byteorder
void QTVRDecoder::Swizzle(uint16 *value)
{
    Swizzle((int16*) value);
}

void QTVRDecoder::Swizzle(int16 *value)
{
    if (!m_HostBigEndian) {
        char    *n, b1,b2;
        n = (char *)value;

        // get the 4 bytes
        b1 = n[0];
        b2 = n[1];

        // save in reverse order
        n[0] = b2;
        n[1] = b1;
    }
}

/*
 * Get a new QImage or a null pointer
 */
QImage * QTVRDecoder::getImage( int face )
{
    QImage * pim = 0;
    m_error = 0;
    switch( m_type ){
    case PANO_CUBIC:
        if( !extractCubeImage( face, pim ) ){
            if(!m_error) {
                m_error = "extractCubeImage() failed";
            }
            if(pim) {
                delete pim;
            }
            pim = 0;
        }
        break;
    case PANO_CYLINDRICAL:
        if( face != 0 ||
                !extractCylImage( pim ) ){
            if(!m_error) {
                m_error = "extractCylImage() failed";
            }
            if(pim) {
                delete pim;
            }
            pim = 0;
        }
        break;
    default:
        m_error = "No pano loaded";
    }
    return pim;
}
