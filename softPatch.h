#ifndef _softPatch_h_
#define _softPatch_h_

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>
#include <Performer/pf/pfNode.h>
#include "tokenStream.h"
#include "softModel.h"
#include "softPrimitive.h"

class softPatch : public softPrimitive
	{
	public:
	 softPatch(void);
	 void parse(tokenStream *);
	 pfNode *pfGraph(void);
	private:
	 void parseControlPoints(tokenStream *);
	 void computeNormals(pfVec3 *norm);
	 void computeTexCoords(softTexture *texture,pfVec2 **texc,ushort **ilist);
	 int utype_, vtype_;
	 int uclose_, vclose_;
	 int upoint_, vpoint_;
	 pfVec3 *controlPoint_;
	 pfBox boundBox_;
	};

#endif
