#ifndef _softFace_h_
#define _softFace_h_

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>
#include <Performer/pf/pfNode.h>
#include "tokenStream.h"
#include "softModel.h"
#include "softPrimitive.h"

class softFace : public softPrimitive
	{
	public:
	 softFace(void);
	 void parse(tokenStream *);
	 pfNode *pfGraph(void);
	private:
	 void parseType(tokenStream *);
	 void parseTension(tokenStream *);
	 void parseControlPoints(tokenStream *);
	 void parseNbKeys(tokenStream *);
	 void parseStep(tokenStream *);
	 pfVec2 generateTexCoord(int method,pfVec3 vert);
	 int type_;
	 int numVerts_;
	 pfVec3 *vertex_;
	 pfBox boundBox_;
	 pfVec3 center_;
	};

#endif
