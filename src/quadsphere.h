/* quadsphere.h		for pvQt	04 Nov 2008 TKS

	Arrays of vertices on the unit sphere and of
	their 2D texture coordinates for 5 projections,
	plus arrays of linear indices that map the 
	vertices to line segments and quadrilaterals.

	The sphere is split into 6 arrays of divs * divs
	quads, corresponding to cube faces.  Faces are
	numbered in order [front, right, back, left, top, 
	bottom] <=> [0:5].  The +Z axis is at the center
	of the front face, +Y at center of top, +X at
	center of right.

	The vertices are unit 3-vectors, so are also the
	unit normals OGL needs to generate cubic texture
	coordinates.

	The texture coordinates are 2D (s,t) normalized
	to [0:1] <=> max valid fov for the type.  Points
	outside the valid fov are set to -1.  Smaller fovs
	can be mapped by scaling up the coordinates using
	the texture matrix.

	There are two index arrays, one for quads and one
	for line segments that form a wireframe drawing of
	the face.  There are line indices for all edges of
	each face, so the number of line indices is greater
	than the number of quad indices.  There is only one
	index array of each kind; half the faces are stored
	row-reversed so all quads have the same orientation.

*/
#ifndef	QUADSPHERE_H
#define	QUADSPHERE_H

class quadsphere {
public:
// this enum matches pvQtPic type except at eqa = 0
	typedef enum { eqa,	rec, sph, cyl, eqr } projection;
	quadsphere( int divs = 30 );
	~quadsphere();
/* sizes
	lineIdxPerFace is the size of the lines index array
	quadIdxPerFace is the size ot the quads index array
*/
	int pointsPerFace(){ return ppf; }
	int quadsPerFace(){ return qpf; }
	int lineIdxPerFace(){ return lpf; }
	int quadIdxPerFace(){ return 4 * qpf; }
// 3D sphere points
	const float * vertices( int face );
// corresponding normalized texture coordinates
	const float * texCoords( int face, projection proj );
// index sequence for line drawing
	const int *   lineIndices( int face );
// index sequence for CW inside quads
	const int *   quadIndices( int face );

private:
	float * words;
	unsigned int nwords;
	int ppf;	// points per face
	int qpf;	// quads per face
	int lpf;	// line ends per face
	float * vertptrs[6],
		  * rectptrs[6],
		  * fishptrs[6],
		  * cyliptrs[6],
		  * equiptrs[6],
		  * anglptrs[6];
	int   *	lineidx,
		  * quadidx;
};
#endif	//ndef	QUADSPHERE_H
