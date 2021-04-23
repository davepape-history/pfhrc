#ifndef _softMesh_h_
#define _softMesh_h_

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>
#include <Performer/pf/pfNode.h>
#include "tokenStream.h"
#include "softModel.h"
#include "softPrimitive.h"

class softMesh : public softPrimitive
	{
	public:
	 softMesh(void);
	 void parse(tokenStream *);
	 pfNode *pfGraph(void);
	private:
	 void parseFlag(tokenStream *);
	 void parseDiscontinuity(tokenStream *);
	 void parseVertices(tokenStream *);
	 void parsePolygons(tokenStream *);
	 void parseNextPolygon(tokenStream *,struct _polygon *);
	 void parseEdges(tokenStream *);
	 void buildPolygons(struct _polylist *polylist,pfGroup * parent);
	 void buildPolygonsInSeparateGeosets(struct _polylist *polylist,pfGroup *parent);
	 void buildPolygonsInOneGeoset(struct _polylist *polylist,pfGroup *parent);
	 int numVerts_;
	 pfVec3 *vertex_;
	 pfList *polygons_;
	 pfBox boundBox_;
	};

#endif
