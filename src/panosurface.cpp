/* panosurface.cpp	for Panini  29 Jan 2009 TKS
 *
    Common code for the projection screen classes

 * Copyright (C) 2008-2009 Thomas K Sharpless
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
#define PANOSURFACE_IMPLEMENTATION
#include	"panosurface.h"
#include	<cstdio>


/* c'tor sets null object
*/
panosurface::panosurface(){
    errmsg = 0;
    nwords = vertpnts = linewrds = quadwrds = 0;
    words = 0;
}

/* get memory and set up pointers
  call from subclass c'tor after setting
    vertpnts = total no. of vertices
    linewrds = 2 * no. of line index pairs
    quadwrds = 2 * no. of quad index pairs

  Allocates a pair of TCs for each vertex
  returns false if allocation fails
*/
bool panosurface::getMemory(){
    nwords =  (3 + 2 * Nprojections) * vertpnts + linewrds + quadwrds;
    words = new float[ nwords ];
    if( words == 0 ){
        errmsg = "insufficient memory for panosurface";
        return false;
    }
// set pointers to arrays
    verts = words;
    TCs = verts + 3 * vertpnts;
    lineidx = (unsigned int *)(TCs + Nprojections * 2 * vertpnts);
    quadidx = lineidx + linewrds;

    return true;
}

/* free memory at destruction */
panosurface::~panosurface(){
    if( words ) delete[] words;
}

/*
    API calls to get data pointers and sizes
*/

const float * panosurface::texCoords( const char * proj ){
    int i = pictypes.picTypeIndex( proj );
    if( i < 0 || i >= Nprojections ) return 0;
    return TCs + i * 2 * vertpnts;
}

unsigned int panosurface::texCoordOffset( const char * proj ){
    const float * p = texCoords( proj );
    if( p == 0 ) return 0;
    return (char *)p - (char *)verts;
}

const float * panosurface::texCoords( pvQtPic::PicType proj ){
    int i = pictypes.picTypeIndex( proj );
    if( i < 0 || i >= Nprojections ) return 0;
    return TCs + i * 2 * vertpnts;
}

unsigned int panosurface::texCoordOffset( pvQtPic::PicType proj ){
    const float * p = texCoords( proj );
    if( p == 0 ) return 0;
    return (char *)p - (char *)verts;
}

/*
  Compute texture coordinates from vertex coordinates
  All memory must be allocated and vertices valid

  The supported mappings are listed in pictureTypes

  The valid range of TCs is [0:1]; invalid points
  get TCs just slightly outside that range.

  The TC ranges for a given projection correspond to
  the absolute max Fovs returned by pictureTypes. If
  the panosurface subtends a smaller angle, the TC
  range will be truncated accordingly.

  To correctly map images with angular sizes smaller than
  maxFov, TC's need to be scaled up appopriately.  The
  function getTCScale() can be used for that.

*/

void panosurface::map_projections(){

// set angular limts from fovs in pictureTypes
    QSizeF fovs;
    fovs = pictypes.maxFov( pictypes.picTypeIndex( "rect" ) );
    double amaxrect = DEG2RAD(0.5 * fovs.width());
    double cminrect = cos( amaxrect );
    double tmaxrect = tan( amaxrect );

    fovs = pictypes.maxFov( pictypes.picTypeIndex( "cyli" ) );
    double amaxcyli = DEG2RAD( 0.5 * fovs.height() );
    double tmaxcyli = tan( amaxcyli );

    fovs = pictypes.maxFov( pictypes.picTypeIndex( "sphr" ) );
    double amaxsphr = DEG2RAD( 0.5 * fovs.width() );

    fovs = pictypes.maxFov( pictypes.picTypeIndex( "fish" ) );
    double amaxfish = DEG2RAD( 0.5 * fovs.width() );

    fovs = pictypes.maxFov( pictypes.picTypeIndex( "merc" ) );
    double amaxmerc = DEG2RAD( 0.5 * fovs.height() );
    double smaxmerc = sin( amaxmerc );
    double tmaxmerc = atanh( smaxmerc );

    fovs = pictypes.maxFov( pictypes.picTypeIndex( "ster" ) );
    double amaxster = DEG2RAD( 0.5 * fovs.width() );
    double tmaxster = tan( 0.5 * amaxster );

// working TC array pntrs
    float *	pr = (float *)texCoords("rect");
    float * pf = (float *)texCoords("fish");
    float * pc = (float *)texCoords("cyli");
    float * pe = (float *)texCoords("equi");
    float * pt = (float *)texCoords("ster");
    float * pm = (float *)texCoords("merc");
    float * pa = (float *)texCoords("sphr");
// working vertex array pointer
    float * ps = verts;

  // loop over all vertices
    for(int r = vertpnts; r > 0; --r ){
  /*
    x,y,z is point on panosphere in direction of vertex
    xa is angle from +Z in XZ plane [-Pi:Pi],
    ya is angle out of XZ plane [-Pi/2 : Pi/2],
    za is angle away from +Z [0:Pi]
    Viewing 0->+Z, X left, Y up
  */
    // unit direction vector
        double s = 1.0 / sqrt(ps[0]*ps[0] + ps[1]*ps[1] + ps[2]*ps[2]);
        double x = s * ps[0],
               y = s * ps[1],
               z = s * ps[2];
    // important angles, their sines and cosines
        double xa = -atan2(x, z), // horiz from +Z
               ya = acos(y), // vert from -Y
               za = acos(z);  // radial from +Z
        double  sxa = -x,	// sin(xa)
                cxa = z,	// cos(xa)
                sya = y,	// sin(ya)
                cya = sqrt( x * x + z * z ),
                sza = sqrt( x * x + y * y ),
                cza = z;
      // direction vector for radial fns (0: invalid)
        double sx = 0, sy = 0;
        if( sza >= 1.0e-4 ){
            sx = sxa / sza;
            sy = -sya / sza;
        }

      // rectilinear
        if( za > 0.45 * Pi ){
            pr[0] = INVAL( sx );
            pr[1] = INVAL( sy );
        } else {
          s = 0.5 * (sza/cza) / tmaxrect;
          double x = CLIP( 0.5 + s * sx ),
                 y = CLIP( 0.5 + s * sy );
          pr[0] = float( x );
          pr[1] = float( y );
        }

      // fisheye
        if( za > amaxfish ){
            pf[0] = INVAL( xa );
            pf[1] = INVAL( ya - 0.5 * Pi );
        } else {
            s = 0.5 * sqrt(0.5 * ( 1 - cza ));
            pf[0] = float(CLIP(0.5 + s * sx) );
            pf[1] = float(CLIP(0.5 + s * sy) );
        }

      // equirectangular
        pe[0] = float( CLIP(0.5 + 0.5 * xa / Pi));
        pe[1] = float( CLIP(ya / Pi));

      // cylindrical
        s = ya - 0.5 * Pi;
        if( fabs(s) > amaxcyli ){
            pc[0] = INVAL( xa );
            pc[1] = INVAL( s );
        } else {
            pc[0] = pe[0];
            pc[1] = float(CLIP(0.5 - 0.5 * (sya/cya) / tmaxcyli));
        }

      // equiangular sphere
        if( za > amaxsphr ){
            pa[0] = INVAL( xa );
            pa[1] = INVAL( ya - 0.5 * Pi );
        } else {
            s = 0.5 * za / Pi;
            pa[0] = float( CLIP(0.5 + s * sx) );
            pa[1] = float( CLIP(0.5 + s * sy) );
        }

      // mercator
        pm[0] = pe[0];
        s = ya - 0.5 * Pi;
        if( fabs(sya) > smaxmerc ) pm[1] = INVAL( s );
        else {
            s = atanh(sya);
            pm[1] = float(CLIP( 0.5 -  0.5 * s / tmaxmerc ));
        }

      // stereographic
        if( za > amaxster ){
            pt[0] = INVAL( sx );
            pt[1] = INVAL( sy );
        } else {
            s = 0.5 * tan( 0.5 * za ) / tmaxster;
            pt[0] = float(CLIP(0.5 + s * sx));
            pt[1] = float(CLIP(0.5 + s * sy));
        }


      // next point
        ps += 3;
        pr += 2; pf += 2; pc += 2; pe += 2;
        pa += 2; pm += 2; pt += 2;
    }
}

/*
  texture coordinate scale factors to correctly map
  an image of a given projection and angular size.
    xfov, yfov are full angular sizes in degrees
    xscale, yscale are factors by which the tabulated TC's should
        be multiplied.
*/
bool panosurface::texScale(  int ipt,
                double xfov, double yfov,
                double& xscale, double& yscale){
    xscale = yscale = 1;
    if( ipt < 0 || ipt >= Nprojections ) return false;
    QSizeF fovs = pictypes.maxFov( ipt );
    pvQtPic::PicType ppt = pictypes.PicType( ipt );
    int ptx, pty;
    pvQtPic::getxyproj( ppt, ptx, pty );
    double d;
    d = pvQtPic::fov2rad( ptx, fovs.width() );
    xscale = d / pvQtPic::fov2rad( ptx, xfov );
    d = pvQtPic::fov2rad( pty, fovs.height() );
    yscale = d / pvQtPic::fov2rad( ptx, yfov );
    return true;
}

bool panosurface::texScale( const char * proj,
                double xfov, double yfov,
                double& xscale, double& yscale){
    return texScale( pictypes.picTypeIndex( proj ),
                     xfov, yfov, xscale, yscale );
}

bool panosurface::texScale( pvQtPic::PicType proj,
                double xfov, double yfov,
                double& xscale, double& yscale  ){
    return texScale( pictypes.picTypeIndex( proj ),
                     xfov, yfov, xscale, yscale );
}
